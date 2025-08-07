#!/usr/bin/env python3
"""
OEIS A000081 Enumeration Validator Demo
========================================

This script demonstrates the standalone OEIS A000081 enumeration validator
functionality for the Echo.Kern DTESN system.
"""

from oeis_a000081_enumerator import (
    create_enhanced_validator,
    validate_membrane_hierarchy_enhanced,
    KNOWN_A000081_VALUES
)

def demo_basic_enumeration():
    """Demonstrate basic enumeration capabilities"""
    print("🌳 OEIS A000081: Unlabeled Rooted Trees")
    print("=" * 50)
    
    enumerator = create_enhanced_validator()
    
    print("The OEIS A000081 sequence counts the number of unlabeled rooted trees with n nodes:")
    print()
    
    # Display first 15 terms in a nice format
    sequence = enumerator.get_sequence(15)
    
    print("   n  |  Trees  |  Visual Representation")
    print("------|---------|----------------------")
    representations = [
        "∅",           # 0: empty
        "•",           # 1: single node
        "•-•",         # 2: line
        "•-•-• OR •<•>•",  # 3: line or fork
        "4 distinct structures",  # 4
        "9 distinct structures",  # 5
        "20 distinct structures", # 6
        "48 distinct structures", # 7
        "115 distinct structures", # 8
        "286 distinct structures", # 9
        "719 distinct structures", # 10
        "1,842 distinct structures", # 11
        "4,766 distinct structures", # 12
        "12,486 distinct structures", # 13
        "32,973 distinct structures", # 14
    ]
    
    for i, (count, desc) in enumerate(zip(sequence, representations)):
        print(f"  {i:2d}  |  {count:5d}  |  {desc}")
    
    print()
    print(f"Known exact values: 0-{enumerator.get_known_range()}")
    print()

def demo_membrane_validation():
    """Demonstrate membrane hierarchy validation"""
    print("🧬 DTESN Membrane Hierarchy Validation")
    print("=" * 50)
    
    print("DTESN membranes must follow OEIS A000081 tree topology:")
    print()
    
    # Valid examples
    valid_examples = [
        ([1], 0, "Single root (depth 0)"),
        ([1, 1], 1, "Root + 1 child (depth 1)"),
        ([1, 1, 1], 2, "Root + child + grandchild (depth 2)"),
        ([1, 1, 1, 2], 3, "Root + child + grandchild + 2 great-grandchildren (depth 3)"),
        ([1, 1, 1, 2, 4], 4, "Full 4-level hierarchy (depth 4)"),
    ]
    
    print("✅ Valid membrane hierarchies:")
    for hierarchy, depth, description in valid_examples:
        is_valid, errors = validate_membrane_hierarchy_enhanced(hierarchy, depth)
        status = "✅" if is_valid else "❌"
        print(f"  {status} {hierarchy} (depth {depth}): {description}")
    
    print()
    
    # Invalid examples
    invalid_examples = [
        ([2, 1, 1, 2, 4], 4, "Wrong root count (should be 1)"),
        ([1, 2, 1, 2, 4], 4, "Wrong level 1 count (should be 1)"),
        ([1, 1, 2, 2, 4], 4, "Wrong level 2 count (should be 1)"),
        ([1, 1, 1, 3, 4], 4, "Wrong level 3 count (should be 2)"),
        ([1, 1, 1, 2, 5], 4, "Wrong level 4 count (should be 4)"),
    ]
    
    print("❌ Invalid membrane hierarchies:")
    for hierarchy, depth, description in invalid_examples:
        is_valid, errors = validate_membrane_hierarchy_enhanced(hierarchy, depth)
        status = "❌" if not is_valid else "⚠️"
        print(f"  {status} {hierarchy} (depth {depth}): {description}")
        if errors:
            for error in errors[:1]:  # Show first error only
                print(f"      → {error}")
    
    print()

def demo_practical_applications():
    """Demonstrate practical applications"""
    print("🔧 Practical Applications")
    print("=" * 50)
    
    enumerator = create_enhanced_validator()
    
    print("1. Memory allocation planning:")
    for nodes in [5, 10, 15]:
        trees = enumerator.get_term(nodes)
        memory_mb = trees * 0.1  # Assume 100KB per tree structure
        print(f"   {nodes} nodes → {trees:,} trees → ~{memory_mb:.1f} MB memory needed")
    
    print()
    print("2. Performance constraints:")
    for max_trees in [100, 1000, 10000]:
        max_nodes = enumerator.get_max_nodes_for_count(max_trees)
        print(f"   ≤{max_trees:,} trees → max {max_nodes} nodes for real-time processing")
    
    print()
    print("3. Validation checks:")
    test_cases = [(5, 9, "correct"), (5, 10, "incorrect"), (8, 115, "correct"), (8, 100, "incorrect")]
    for nodes, claimed_trees, expectation in test_cases:
        is_valid = enumerator.is_valid_tree_count(nodes, claimed_trees)
        status = "✅" if is_valid else "❌"
        print(f"   {status} {nodes} nodes with {claimed_trees} trees: {expectation}")
    
    print()

def demo_advanced_features():
    """Demonstrate advanced features"""
    print("🚀 Advanced Features")
    print("=" * 50)
    
    enumerator = create_enhanced_validator()
    
    print("1. Large-scale enumeration (using asymptotic approximation):")
    large_indices = [30, 40, 50]
    for n in large_indices:
        trees = enumerator.get_term(n)
        print(f"   A000081({n}) ≈ {trees:,} trees")
    
    print()
    print("2. Mathematical properties:")
    growth_ratios = []
    for n in range(5, 10):
        current = enumerator.get_term(n)
        previous = enumerator.get_term(n-1)
        ratio = current / previous if previous > 0 else 0
        growth_ratios.append(ratio)
        print(f"   A000081({n})/A000081({n-1}) = {ratio:.2f}")
    
    avg_growth = sum(growth_ratios) / len(growth_ratios)
    print(f"   Average growth ratio: {avg_growth:.2f}")
    
    print()
    print("3. Sequence validation:")
    known_subset = KNOWN_A000081_VALUES[:10]
    computed_subset = enumerator.get_sequence(10)
    is_valid, errors = enumerator.validate_sequence(known_subset)
    print(f"   Validation against known values: {'✅ PASSED' if is_valid else '❌ FAILED'}")
    
    print()

def main():
    """Run the complete demo"""
    print("OEIS A000081 Enumeration Validator Demo")
    print("Echo.Kern DTESN System")
    print("=" * 70)
    print()
    
    demo_basic_enumeration()
    demo_membrane_validation()
    demo_practical_applications()
    demo_advanced_features()
    
    print("🎯 Summary")
    print("=" * 50)
    print("The OEIS A000081 enumeration validator provides:")
    print("✅ Dynamic sequence computation with exact values")
    print("✅ Membrane hierarchy validation for DTESN systems")
    print("✅ Practical tools for memory and performance planning")
    print("✅ Mathematical validation and verification")
    print("✅ Integration with the DTESN compiler toolchain")
    print()
    print("This implementation satisfies the immediate (Week 1-2) requirement")
    print("for OEIS A000081 enumeration validation in the Echo.Kern project.")

if __name__ == "__main__":
    main()