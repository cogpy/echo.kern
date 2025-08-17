#!/usr/bin/env python3
"""
Echo.Kern DTESN Prototypes Test Suite
Part of Echo9 development area

This script runs all prototype validation tests.
"""

import sys
import os
import subprocess
from pathlib import Path

def run_test_suite():
    """Run comprehensive prototype test suite."""
    test_files = [
        'test_psystem_membranes.py',
        'test_psystem_evolution_engine.py', 
        'test_bseries_tree_classifier.py',
        'test_bseries_differential_calculator.py',
        'test_esn_reservoir.py',
        'test_oeis_a000081.py'
    ]
    
    results = {}
    total_tests = 0
    passed_tests = 0
    
    print("🧪 Running DTESN Prototype Test Suite")
    print("=" * 50)
    
    for test_file in test_files:
        if Path(test_file).exists():
            print(f"\n📋 Running {test_file}...")
            try:
                result = subprocess.run([sys.executable, test_file], 
                                      capture_output=True, text=True, timeout=60)
                if result.returncode == 0:
                    print(f"✅ {test_file} PASSED")
                    results[test_file] = 'PASSED'
                    passed_tests += 1
                else:
                    print(f"❌ {test_file} FAILED")
                    print(f"   Error: {result.stderr.strip()}")
                    results[test_file] = 'FAILED'
                total_tests += 1
            except subprocess.TimeoutExpired:
                print(f"⏰ {test_file} TIMEOUT")
                results[test_file] = 'TIMEOUT'
                total_tests += 1
            except Exception as e:
                print(f"💥 {test_file} ERROR: {e}")
                results[test_file] = 'ERROR'
                total_tests += 1
        else:
            print(f"⚠️  {test_file} not found")
    
    print("\n" + "=" * 50)
    print(f"📊 Test Results: {passed_tests}/{total_tests} passed")
    
    # Run demonstration scripts
    demo_files = ['bseries_calculator_demo.py', 'oeis_validator_demo.py', 'evolution_engine_validation.py']
    for demo_file in demo_files:
        if Path(demo_file).exists():
            print(f"\n🎯 Running demonstration: {demo_file}")
            try:
                result = subprocess.run([sys.executable, demo_file], 
                                      capture_output=True, text=True, timeout=30)
                if result.returncode == 0:
                    print(f"✅ {demo_file} completed successfully")
                else:
                    print(f"⚠️  {demo_file} completed with warnings")
            except Exception as e:
                print(f"❌ {demo_file} failed: {e}")
    
    return passed_tests == total_tests

if __name__ == "__main__":
    success = run_test_suite()
    sys.exit(0 if success else 1)