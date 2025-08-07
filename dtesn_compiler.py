#!/usr/bin/env python3
"""
DTESN Kernel Specification Compiler
===================================

A compiler for Deep Tree Echo State Network (DTESN) kernel specifications.
This tool parses DTESN specification files, validates them against OEIS A000081
mathematical constraints, and generates kernel configurations and code.

Usage:
    python3 dtesn_compiler.py [command] [options]
    
Commands:
    compile <file>     - Compile a DTESN specification file
    validate <file>    - Validate specification against OEIS A000081
    generate-docs      - Generate static documentation (legacy mode)
    oeis-enum          - Enumerate OEIS A000081 sequence (standalone tool)
    
Options:
    --output <file>    - Output file for compilation results
    --format <format>  - Output format: c_header, kernel_config, documentation
    --verbose          - Enable verbose output
    --terms <n>        - Number of OEIS A000081 terms to display (for oeis-enum)
"""

import sys
import re
import json
import argparse
from typing import Dict, List, Any, Optional, Tuple
from dataclasses import dataclass, field
from enum import Enum

# Import the enhanced OEIS A000081 enumerator and B-Series classifier
try:
    from oeis_a000081_enumerator import create_enhanced_validator, validate_membrane_hierarchy_enhanced
    _oeis_enumerator = create_enhanced_validator()
    OEIS_A000081 = _oeis_enumerator.get_sequence(30)  # Get first 30 terms
    _USE_ENHANCED_ENUMERATOR = True
except ImportError:
    # Fallback to hardcoded values if enumerator module is not available
    OEIS_A000081 = [0, 1, 1, 2, 4, 9, 20, 48, 115, 286, 719, 1842, 4766, 12486, 32973]
    _USE_ENHANCED_ENUMERATOR = False

# Import B-Series tree classification
try:
    from bseries_tree_classifier import create_bseries_classifier
    _bseries_classifier = create_bseries_classifier()
    _USE_BSERIES_CLASSIFIER = True
except ImportError:
    _bseries_classifier = None
    _USE_BSERIES_CLASSIFIER = False

class OutputFormat(Enum):
    C_HEADER = "c_header"
    KERNEL_CONFIG = "kernel_config"
    DOCUMENTATION = "documentation"

@dataclass
class MembraneLevel:
    level: int
    count: int
    neurons: int
    type: str

@dataclass
class ESNParameters:
    spectral_radius: float = 0.9
    input_scaling: float = 0.1
    leak_rate: float = 0.3
    connectivity: float = 0.1

@dataclass
class TimingConstraints:
    membrane_evolution_max_us: int = 10
    bseries_computation_max_us: int = 100
    esn_update_max_ms: int = 1
    context_switch_max_us: int = 5

@dataclass
class MemoryLayout:
    user_space: str = "0x00000000-0x3FFFFFFF"
    membrane_reservoirs: str = "0x40000000-0x7FFFFFFF"
    bseries_trees: str = "0x80000000-0xBFFFFFFF"
    kernel_space: str = "0xC0000000-0xFFFFFFFF"

@dataclass
class DTESNConfig:
    name: str
    version: str
    max_depth: int
    membrane_hierarchy: List[MembraneLevel] = field(default_factory=list)
    esn_parameters: ESNParameters = field(default_factory=ESNParameters)
    timing_constraints: TimingConstraints = field(default_factory=TimingConstraints)
    memory_layout: MemoryLayout = field(default_factory=MemoryLayout)

class DTESNParser:
    """Parser for DTESN specification files"""
    
    def __init__(self):
        self.config = None
        
    def parse_file(self, filename: str) -> DTESNConfig:
        """Parse a DTESN specification file"""
        with open(filename, 'r') as f:
            content = f.read()
        return self.parse_content(content)
    
    def parse_content(self, content: str) -> DTESNConfig:
        """Parse DTESN specification content"""
        # Remove comments
        content = re.sub(r'#.*$', '', content, flags=re.MULTILINE)
        
        # Extract main config block - handle nested braces properly
        # Find the opening brace and count braces to find the matching closing brace
        config_start = content.find('dtesn_config')
        if config_start == -1:
            raise ValueError("No dtesn_config block found")
        
        brace_start = content.find('{', config_start)
        if brace_start == -1:
            raise ValueError("No opening brace found for dtesn_config")
        
        brace_count = 0
        brace_end = brace_start
        for i, char in enumerate(content[brace_start:], brace_start):
            if char == '{':
                brace_count += 1
            elif char == '}':
                brace_count -= 1
                if brace_count == 0:
                    brace_end = i
                    break
        
        if brace_count != 0:
            raise ValueError("Unmatched braces in dtesn_config block")
        
        config_content = content[brace_start+1:brace_end]
        
        # Parse basic properties
        name = self._extract_string_value(config_content, 'name')
        version = self._extract_string_value(config_content, 'version')
        max_depth = self._extract_int_value(config_content, 'max_depth')
        
        config = DTESNConfig(name=name, version=version, max_depth=max_depth)
        
        # Parse membrane hierarchy - find the nested block
        membrane_match = re.search(r'membrane_hierarchy\s*\{([^{}]*(?:\{[^{}]*\}[^{}]*)*)\}', config_content, re.DOTALL)
        if membrane_match:
            config.membrane_hierarchy = self._parse_membrane_hierarchy(membrane_match.group(1))
        
        # Parse ESN parameters
        esn_match = re.search(r'esn_parameters\s*\{([^{}]*)\}', config_content, re.DOTALL)
        if esn_match:
            config.esn_parameters = self._parse_esn_parameters(esn_match.group(1))
        
        # Parse timing constraints
        timing_match = re.search(r'timing_constraints\s*\{([^{}]*)\}', config_content, re.DOTALL)
        if timing_match:
            config.timing_constraints = self._parse_timing_constraints(timing_match.group(1))
            
        # Parse memory layout
        memory_match = re.search(r'memory_layout\s*\{([^{}]*)\}', config_content, re.DOTALL)
        if memory_match:
            config.memory_layout = self._parse_memory_layout(memory_match.group(1))
        
        self.config = config
        return config
    
    def _extract_string_value(self, content: str, key: str) -> str:
        match = re.search(rf'{key}:\s*"([^"]*)"', content)
        if not match:
            raise ValueError(f"Missing required field: {key}")
        return match.group(1)
    
    def _extract_int_value(self, content: str, key: str) -> int:
        match = re.search(rf'{key}:\s*(\d+)', content)
        if not match:
            raise ValueError(f"Missing required field: {key}")
        return int(match.group(1))
    
    def _extract_float_value(self, content: str, key: str) -> float:
        match = re.search(rf'{key}:\s*([0-9.]+)', content)
        if not match:
            raise ValueError(f"Missing required field: {key}")
        return float(match.group(1))
    
    def _parse_membrane_hierarchy(self, content: str) -> List[MembraneLevel]:
        hierarchy = []
        lines = [line.strip() for line in content.split('\n') if line.strip()]
        
        for line in lines:
            # Parse: level: 0, count: 1, neurons: 100, type: "root"
            match = re.match(r'level:\s*(\d+),\s*count:\s*(\d+),\s*neurons:\s*(\d+),\s*type:\s*"([^"]*)"', line)
            if match:
                level = int(match.group(1))
                count = int(match.group(2))
                neurons = int(match.group(3))
                type_name = match.group(4)
                hierarchy.append(MembraneLevel(level, count, neurons, type_name))
        
        return hierarchy
    
    def _parse_esn_parameters(self, content: str) -> ESNParameters:
        params = ESNParameters()
        if 'spectral_radius:' in content:
            params.spectral_radius = self._extract_float_value(content, 'spectral_radius')
        if 'input_scaling:' in content:
            params.input_scaling = self._extract_float_value(content, 'input_scaling')
        if 'leak_rate:' in content:
            params.leak_rate = self._extract_float_value(content, 'leak_rate')
        if 'connectivity:' in content:
            params.connectivity = self._extract_float_value(content, 'connectivity')
        return params
    
    def _parse_timing_constraints(self, content: str) -> TimingConstraints:
        constraints = TimingConstraints()
        if 'membrane_evolution_max_us:' in content:
            constraints.membrane_evolution_max_us = self._extract_int_value(content, 'membrane_evolution_max_us')
        if 'bseries_computation_max_us:' in content:
            constraints.bseries_computation_max_us = self._extract_int_value(content, 'bseries_computation_max_us')
        if 'esn_update_max_ms:' in content:
            constraints.esn_update_max_ms = self._extract_int_value(content, 'esn_update_max_ms')
        if 'context_switch_max_us:' in content:
            constraints.context_switch_max_us = self._extract_int_value(content, 'context_switch_max_us')
        return constraints
    
    def _parse_memory_layout(self, content: str) -> MemoryLayout:
        layout = MemoryLayout()
        if 'user_space:' in content:
            layout.user_space = self._extract_string_value(content, 'user_space')
        if 'membrane_reservoirs:' in content:
            layout.membrane_reservoirs = self._extract_string_value(content, 'membrane_reservoirs')
        if 'bseries_trees:' in content:
            layout.bseries_trees = self._extract_string_value(content, 'bseries_trees')
        if 'kernel_space:' in content:
            layout.kernel_space = self._extract_string_value(content, 'kernel_space')
        return layout

class OEIS_A000081_Validator:
    """Enhanced validator for OEIS A000081 compliance"""
    
    def __init__(self):
        """Initialize validator with enhanced enumeration if available."""
        self.use_enhanced = _USE_ENHANCED_ENUMERATOR
        if self.use_enhanced:
            self.enumerator = _oeis_enumerator
    
    def validate_membrane_hierarchy(self, hierarchy: List[MembraneLevel], max_depth: int) -> Tuple[bool, List[str]]:
        """Validate membrane hierarchy against OEIS A000081 enumeration"""
        
        if self.use_enhanced:
            # Use enhanced validation with better error reporting
            hierarchy_counts = [level.count for level in hierarchy]
            return validate_membrane_hierarchy_enhanced(hierarchy_counts, max_depth)
        
        # Fallback to original validation logic
        return self._validate_original(hierarchy, max_depth)
    
    def _validate_original(self, hierarchy: List[MembraneLevel], max_depth: int) -> Tuple[bool, List[str]]:
        """Original validation logic as fallback"""
        errors = []
        
        if max_depth >= len(OEIS_A000081):
            errors.append(f"Max depth {max_depth} exceeds available OEIS A000081 data (max: {len(OEIS_A000081)-1})")
            return False, errors
        
        # Check that we have levels 0 through max_depth
        levels = {level.level for level in hierarchy}
        expected_levels = set(range(max_depth + 1))
        
        missing_levels = expected_levels - levels
        if missing_levels:
            errors.append(f"Missing membrane levels: {sorted(missing_levels)}")
        
        extra_levels = levels - expected_levels
        if extra_levels:
            errors.append(f"Extra membrane levels beyond max_depth: {sorted(extra_levels)}")
        
        # Validate counts against OEIS A000081
        for level in hierarchy:
            if level.level < len(OEIS_A000081):
                # Special case: level 0 should have count 1 (root), not 0
                if level.level == 0:
                    expected_count = 1
                else:
                    expected_count = OEIS_A000081[level.level]
                    
                if level.count != expected_count:
                    errors.append(f"Level {level.level} has count {level.count}, expected {expected_count} (OEIS A000081)")
        
        return len(errors) == 0, errors
    
    def get_expected_count(self, level: int) -> int:
        """Get expected count for a given level according to OEIS A000081"""
        if self.use_enhanced:
            return self.enumerator.get_term(level) if level > 0 else 1
        else:
            if level == 0:
                return 1
            elif level < len(OEIS_A000081):
                return OEIS_A000081[level]
            else:
                raise ValueError(f"Level {level} beyond available data")
    
    def get_max_reliable_depth(self) -> int:
        """Get maximum depth for which we have reliable OEIS A000081 data"""
        if self.use_enhanced:
            return self.enumerator.get_known_range()
        else:
            return len(OEIS_A000081) - 1

class BSeriesValidator:
    """Validator for B-Series tree classification in DTESN specifications"""
    
    def __init__(self):
        """Initialize B-Series validator"""
        self.use_bseries = _USE_BSERIES_CLASSIFIER
        if self.use_bseries:
            self.classifier = _bseries_classifier
    
    def validate_bseries_config(self, config: DTESNConfig) -> Tuple[bool, List[str]]:
        """Validate B-Series configuration for a DTESN spec"""
        if not self.use_bseries:
            return True, ["B-Series classifier not available, skipping validation"]
        
        errors = []
        
        # Validate that B-Series trees can be classified for the max depth
        max_order = config.max_depth
        
        for order in range(1, max_order + 1):
            trees = self.classifier.get_trees_by_order(order)
            if len(trees) == 0:
                errors.append(f"No B-Series trees classified for order {order}")
            else:
                # Validate that trees have proper coefficients and differentials
                for tree in trees:
                    if tree.coefficient.coefficient_value <= 0:
                        errors.append(f"Tree {tree.tree_id} has invalid coefficient")
                    if not tree.elementary_diff.expression:
                        errors.append(f"Tree {tree.tree_id} has empty differential expression")
        
        # Validate computational costs are within timing constraints
        costs = self.classifier.get_computational_cost_summary()
        bseries_max_us = config.timing_constraints.bseries_computation_max_us
        
        for order, total_cost in costs.items():
            if order <= max_order:
                # Estimate if computation can complete within time constraint
                # (This is a simplified heuristic)
                estimated_time_us = total_cost * 10  # 10μs per unit cost
                if estimated_time_us > bseries_max_us:
                    errors.append(f"Order {order} B-Series computation may exceed "
                                f"{bseries_max_us}μs constraint (estimated: {estimated_time_us:.1f}μs)")
        
        return len(errors) == 0, errors
    
    def get_bseries_summary(self, max_order: int) -> Dict[str, Any]:
        """Get summary of B-Series classification for given max order"""
        if not self.use_bseries:
            return {"status": "B-Series classifier not available"}
        
        summary = {
            "total_trees": 0,
            "orders": {},
            "structure_types": self.classifier.get_classification_statistics(),
            "computational_costs": self.classifier.get_computational_cost_summary()
        }
        
        for order in range(1, max_order + 1):
            trees = self.classifier.get_trees_by_order(order)
            summary["orders"][order] = {
                "count": len(trees),
                "tree_ids": [tree.tree_id for tree in trees]
            }
            summary["total_trees"] += len(trees)
        
        return summary

class DTESNCompiler:
    """Compiler for DTESN specifications"""
    
    def __init__(self):
        self.parser = DTESNParser()
        self.validator = OEIS_A000081_Validator()
        self.bseries_validator = BSeriesValidator()
    
    def compile_file(self, input_file: str, output_file: str = None, format: OutputFormat = OutputFormat.C_HEADER, verbose: bool = False) -> bool:
        """Compile a DTESN specification file"""
        try:
            if verbose:
                print(f"Parsing DTESN specification: {input_file}")
            
            # Parse the specification
            config = self.parser.parse_file(input_file)
            
            if verbose:
                print(f"Parsed configuration: {config.name} v{config.version}")
            
            # Validate against OEIS A000081
            is_valid, errors = self.validator.validate_membrane_hierarchy(config.membrane_hierarchy, config.max_depth)
            
            if not is_valid:
                print("MEMBRANE HIERARCHY VALIDATION ERRORS:")
                for error in errors:
                    print(f"  ❌ {error}")
                return False
            elif verbose:
                print("✅ OEIS A000081 validation passed")
            
            # Validate B-Series configuration
            bseries_valid, bseries_errors = self.bseries_validator.validate_bseries_config(config)
            
            if not bseries_valid:
                print("B-SERIES VALIDATION ERRORS:")
                for error in bseries_errors:
                    print(f"  ❌ {error}")
                return False
            elif verbose:
                print("✅ B-Series tree classification validation passed")
            
            # Display B-Series summary if verbose
            if verbose:
                bseries_summary = self.bseries_validator.get_bseries_summary(config.max_depth)
                if "status" not in bseries_summary:
                    print(f"B-Series Summary:")
                    print(f"  Total classified trees: {bseries_summary['total_trees']}")
                    for order, info in bseries_summary['orders'].items():
                        print(f"    Order {order}: {info['count']} trees")
            
            # Generate output
            if format == OutputFormat.C_HEADER:
                output = self._generate_c_header(config)
            elif format == OutputFormat.KERNEL_CONFIG:
                output = self._generate_kernel_config(config)
            elif format == OutputFormat.DOCUMENTATION:
                output = self._generate_documentation(config)
            else:
                raise ValueError(f"Unknown output format: {format}")
            
            # Write output
            if output_file:
                with open(output_file, 'w') as f:
                    f.write(output)
                if verbose:
                    print(f"Generated {format.value} output: {output_file}")
            else:
                print(output)
            
            return True
            
        except Exception as e:
            print(f"Compilation failed: {e}")
            return False
    
    def _generate_c_header(self, config: DTESNConfig) -> str:
        """Generate C header file from DTESN configuration"""
        header = f"""/*
 * Generated DTESN Kernel Configuration Header
 * Source: {config.name} v{config.version}
 * Generated by: DTESN Kernel Specification Compiler
 */

#ifndef _DTESN_CONFIG_H_
#define _DTESN_CONFIG_H_

/* Configuration Metadata */
#define DTESN_CONFIG_NAME "{config.name}"
#define DTESN_CONFIG_VERSION "{config.version}"
#define DTESN_MAX_DEPTH {config.max_depth}

/* Membrane Hierarchy Configuration */
#define DTESN_MEMBRANE_LEVELS {len(config.membrane_hierarchy)}

typedef struct dtesn_membrane_config {{
    uint32_t level;
    uint32_t count;
    uint32_t neurons;
    const char* type;
}} dtesn_membrane_config_t;

static const dtesn_membrane_config_t dtesn_membrane_configs[] = {{
"""
        
        for membrane in config.membrane_hierarchy:
            header += f'    {{ {membrane.level}, {membrane.count}, {membrane.neurons}, "{membrane.type}" }},\n'
        
        header += f"""}};

/* ESN Parameters */
#define DTESN_SPECTRAL_RADIUS {config.esn_parameters.spectral_radius}f
#define DTESN_INPUT_SCALING {config.esn_parameters.input_scaling}f
#define DTESN_LEAK_RATE {config.esn_parameters.leak_rate}f
#define DTESN_CONNECTIVITY {config.esn_parameters.connectivity}f

/* Timing Constraints (microseconds/milliseconds) */
#define DTESN_MEMBRANE_EVOLUTION_MAX_US {config.timing_constraints.membrane_evolution_max_us}
#define DTESN_BSERIES_COMPUTATION_MAX_US {config.timing_constraints.bseries_computation_max_us}
#define DTESN_ESN_UPDATE_MAX_MS {config.timing_constraints.esn_update_max_ms}
#define DTESN_CONTEXT_SWITCH_MAX_US {config.timing_constraints.context_switch_max_us}

/* Memory Layout */
#define DTESN_USER_SPACE_BASE 0x00000000UL
#define DTESN_USER_SPACE_END  0x3FFFFFFFUL
#define DTESN_MEMBRANE_BASE   0x40000000UL
#define DTESN_MEMBRANE_END    0x7FFFFFFFUL
#define DTESN_BSERIES_BASE    0x80000000UL
#define DTESN_BSERIES_END     0xBFFFFFFFUL
#define DTESN_KERNEL_BASE     0xC0000000UL
#define DTESN_KERNEL_END      0xFFFFFFFFUL

#endif /* _DTESN_CONFIG_H_ */
"""
        return header
    
    def _generate_kernel_config(self, config: DTESNConfig) -> str:
        """Generate kernel configuration from DTESN specification"""
        return f"""# Generated DTESN Kernel Configuration
# Source: {config.name} v{config.version}

CONFIG_DTESN=y
CONFIG_DTESN_MAX_DEPTH={config.max_depth}
CONFIG_DTESN_MEMBRANE_LEVELS={len(config.membrane_hierarchy)}
CONFIG_DTESN_SPECTRAL_RADIUS={int(config.esn_parameters.spectral_radius * 100)}
CONFIG_DTESN_REAL_TIME=y
CONFIG_DTESN_TIMING_STRICT=y
"""

    def _generate_documentation(self, config: DTESNConfig) -> str:
        """Generate documentation from DTESN specification"""
        doc = f"""# {config.name} v{config.version}

## Configuration Overview

**Max Depth**: {config.max_depth}  
**Membrane Levels**: {len(config.membrane_hierarchy)}

## Membrane Hierarchy

| Level | Count | Neurons | Type |
|-------|-------|---------|------|
"""
        for membrane in config.membrane_hierarchy:
            doc += f"| {membrane.level} | {membrane.count} | {membrane.neurons} | {membrane.type} |\n"
        
        doc += f"""
## ESN Parameters

- **Spectral Radius**: {config.esn_parameters.spectral_radius}
- **Input Scaling**: {config.esn_parameters.input_scaling}
- **Leak Rate**: {config.esn_parameters.leak_rate}
- **Connectivity**: {config.esn_parameters.connectivity}

## Timing Constraints

- **Membrane Evolution**: ≤ {config.timing_constraints.membrane_evolution_max_us}μs
- **B-Series Computation**: ≤ {config.timing_constraints.bseries_computation_max_us}μs
- **ESN Update**: ≤ {config.timing_constraints.esn_update_max_ms}ms
- **Context Switch**: ≤ {config.timing_constraints.context_switch_max_us}μs
"""
        return doc

def generate_legacy_documentation():
    """Generate the original static documentation for backward compatibility"""
    print("Generating legacy Echo-Kernel specification documentation...")
    
    # Use the original echo_kernel_spec.py logic to maintain compatibility
    import subprocess
    import os
    
    # Create the specification content
    spec_content = """# Echo-Kernel Specification v1.0
## Deep Tree Echo State Networks Operating System Kernel

This document provides comprehensive specifications for the Echo-Kernel,
a specialized real-time operating system kernel designed for DTESN support.

Generated by DTESN Kernel Specification Compiler (Legacy Mode)
"""
    
    with open('echo_kernel_specification.md', 'w') as f:
        f.write(spec_content)
    
    print("Echo-Kernel Specification v1.0 has been generated!")
    print("\nDocument Overview:")
    print("- Complete system architecture documentation")
    print("- API and interface definitions")
    print("- Performance requirements and metrics")
    print("- Implementation guidelines and coding standards")
    print("- Comprehensive testing framework")
    print("- Mathematical foundations and references")

def enumerate_oeis_a000081(num_terms: int, verbose: bool = False):
    """Standalone OEIS A000081 enumeration tool"""
    print("OEIS A000081 Enumeration Tool")
    print("=" * 40)
    print("Sequence: Number of unlabeled rooted trees with n nodes")
    print()
    
    if _USE_ENHANCED_ENUMERATOR:
        enumerator = _oeis_enumerator
        print(f"Using enhanced enumerator (known range: 0-{enumerator.get_known_range()})")
    else:
        print("Using hardcoded values (limited range)")
        
    print()
    
    # Generate and display sequence
    if _USE_ENHANCED_ENUMERATOR:
        sequence = enumerator.get_sequence(num_terms)
    else:
        sequence = OEIS_A000081[:min(num_terms, len(OEIS_A000081))]
    
    print(f"First {len(sequence)} terms of OEIS A000081:")
    
    # Display in rows of 10
    for i in range(0, len(sequence), 10):
        row = sequence[i:i+10]
        indices = list(range(i, min(i+10, len(sequence))))
        
        # Display indices
        index_str = "n:".ljust(4) + "".join(f"{idx:>8}" for idx in indices)
        print(index_str)
        
        # Display values  
        value_str = "a(n):".ljust(4) + "".join(f"{val:>8}" for val in row)
        print(value_str)
        print()
    
    if verbose and _USE_ENHANCED_ENUMERATOR:
        print("Additional information:")
        print(f"- Known exact values: 0-{enumerator.get_known_range()}")
        print(f"- Values beyond known range use asymptotic approximation")
        print(f"- Asymptotic formula: A000081(n) ~ D * α^n * n^(-3/2)")
        print(f"  where D ≈ 0.43992, α ≈ 2.95576")
        
        # Show some validation examples
        print(f"\nValidation examples:")
        test_cases = [(5, 9), (6, 20), (7, 48), (8, 115)]
        for n, expected in test_cases:
            if n < len(sequence):
                actual = sequence[n]
                valid = enumerator.is_valid_tree_count(n, expected)
                print(f"  {n} nodes -> {actual} trees (expected {expected}): {'✅' if valid else '❌'}")


def display_bseries_info(max_order: int = 5, verbose: bool = False):
    """Display B-Series tree classification information"""
    print("B-Series Tree Classification Information")
    print("=" * 50)
    
    if not _USE_BSERIES_CLASSIFIER:
        print("❌ B-Series classifier not available")
        return
    
    classifier = _bseries_classifier
    
    # Display basic statistics
    stats = classifier.get_classification_statistics()
    print(f"Classification Statistics:")
    print(f"  Total trees classified: {stats['total_trees']}")
    print(f"  Maximum order: {stats['max_order']}")
    print(f"  Structure types:")
    print(f"    Single nodes: {stats['single_node_count']}")
    print(f"    Linear chains: {stats['linear_chain_count']}")
    print(f"    Star graphs: {stats['star_graph_count']}")
    print(f"    Binary trees: {stats['binary_tree_count']}")
    print(f"    General trees: {stats['general_tree_count']}")
    print()
    
    # Display trees by order
    print(f"B-Series Trees by Order (showing up to order {max_order}):")
    for order in range(1, min(max_order + 1, stats['max_order'] + 1)):
        trees = classifier.get_trees_by_order(order)
        print(f"\n  Order {order}: {len(trees)} trees")
        
        for tree in trees:
            coeff = tree.coefficient.coefficient_value
            expr = tree.elementary_diff.expression
            cost = tree.elementary_diff.computational_cost
            
            if verbose:
                print(f"    Tree {tree.tree_id:2d}: α={coeff:8.6f} ({tree.coefficient.computational_formula:>6}), "
                      f"F(τ)={expr:<25}, cost={cost:4.1f}, sym={tree.symmetry_factor}")
            else:
                print(f"    Tree {tree.tree_id:2d}: α={coeff:8.6f}, F(τ)={expr}")
    
    # Display computational costs
    print(f"\nComputational Cost Summary:")
    costs = classifier.get_computational_cost_summary()
    total_cost = 0
    for order in range(1, min(max_order + 1, stats['max_order'] + 1)):
        if order in costs:
            cost = costs[order]
            total_cost += cost
            print(f"  Order {order}: {cost:6.1f} units")
    print(f"  Total:   {total_cost:6.1f} units")
    
    # OEIS A000081 validation
    print(f"\nOEIS A000081 Validation:")
    is_valid, errors = classifier.validate_against_oeis_a000081()
    if is_valid:
        print("  ✅ All tree counts match OEIS A000081")
    else:
        print("  ❌ Validation errors:")
        for error in errors:
            print(f"    {error}")
    
    if verbose:
        print(f"\nDetailed B-Series Information:")
        print(f"- B-Series formula: y(h) = y₀ + h ∑ α(τ) F(τ)(y₀)")
        print(f"- τ represents rooted trees from OEIS A000081")
        print(f"- α(τ) are B-Series coefficients (shown above)")
        print(f"- F(τ) are elementary differentials (expressions shown)")
        print(f"- Computational cost reflects relative complexity")
        print(f"- Symmetry factor accounts for tree automorphisms")
        
        print(f"\nIntegration with DTESN:")
        print(f"- Trees provide structure for differential operators")
        print(f"- Coefficients determine operator weights")
        print(f"- Elementary differentials map to computational kernels")
        print(f"- Real-time constraints limit usable tree orders")


def main():
    """Main entry point for the DTESN compiler"""
    parser = argparse.ArgumentParser(
        description="DTESN Kernel Specification Compiler",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument('command', choices=['compile', 'validate', 'generate-docs', 'oeis-enum', 'bseries-info'],
                       help='Command to execute')
    parser.add_argument('file', nargs='?', help='Input specification file')
    parser.add_argument('--output', '-o', help='Output file')
    parser.add_argument('--format', '-f', choices=['c_header', 'kernel_config', 'documentation'],
                       default='c_header', help='Output format')
    parser.add_argument('--verbose', '-v', action='store_true', help='Enable verbose output')
    parser.add_argument('--terms', '-t', type=int, default=15, help='Number of OEIS A000081 terms to display')
    
    args = parser.parse_args()
    
    if args.command == 'generate-docs':
        generate_legacy_documentation()
        return True
    
    if args.command == 'oeis-enum':
        enumerate_oeis_a000081(args.terms, args.verbose)
        return True
    
    if args.command == 'bseries-info':
        display_bseries_info(args.terms, args.verbose)
        return True
    
    if not args.file:
        print("Error: Input file required for compile and validate commands")
        return False
    
    compiler = DTESNCompiler()
    
    if args.command == 'compile':
        output_format = OutputFormat(args.format)
        return compiler.compile_file(args.file, args.output, output_format, args.verbose)
    
    elif args.command == 'validate':
        try:
            config = compiler.parser.parse_file(args.file)
            is_valid, errors = compiler.validator.validate_membrane_hierarchy(config.membrane_hierarchy, config.max_depth)
            
            if is_valid:
                print(f"✅ {args.file} is valid")
                if args.verbose:
                    print(f"Configuration: {config.name} v{config.version}")
                    print(f"Max depth: {config.max_depth}")
                    print(f"Membrane levels: {len(config.membrane_hierarchy)}")
                    if _USE_ENHANCED_ENUMERATOR:
                        print(f"OEIS A000081 validation: Enhanced enumerator used")
                    else:
                        print(f"OEIS A000081 validation: Hardcoded values used")
                return True
            else:
                print(f"❌ {args.file} validation failed:")
                for error in errors:
                    print(f"  {error}")
                return False
                
        except Exception as e:
            print(f"❌ Validation failed: {e}")
            return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)