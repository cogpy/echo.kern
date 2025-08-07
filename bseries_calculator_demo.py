#!/usr/bin/env python3
"""
B-Series Elementary Differential Calculator Demo
===============================================

This demonstration showcases the capabilities of the B-Series elementary
differential calculator for solving differential equations using B-Series methods.
"""

import sys
import math
from bseries_differential_calculator import (
    BSeriesDifferentialCalculator,
    create_differential_function
)


def demo_exponential_growth():
    """Demonstrate B-Series solution for exponential growth: dy/dt = y"""
    print("Demo 1: Exponential Growth (dy/dt = y)")
    print("-" * 40)
    
    calculator = BSeriesDifferentialCalculator()
    
    # Define dy/dt = y (exponential growth)
    def f(y):
        return y
    
    def f_prime(y):
        return 1.0
    
    def f_double(y):
        return 0.0
    
    def f_triple(y):
        return 0.0
    
    def f_quad(y):
        return 0.0
    
    df = create_differential_function(f, f_prime, f_double, f_triple, f_quad, "y")
    
    # Initial conditions
    y0 = 1.0
    h = 0.1
    n_steps = 5
    
    print(f"Differential equation: dy/dt = y")
    print(f"Initial condition: y(0) = {y0}")
    print(f"Step size: h = {h}")
    print(f"Exact solution: y(t) = e^t")
    print()
    
    print(f"{'t':<6} {'B-Series':<12} {'Exact':<12} {'Error':<12}")
    print("-" * 50)
    
    y = y0
    for i in range(n_steps + 1):
        t = i * h
        exact = math.exp(t)
        error = abs(y - exact)
        
        print(f"{t:<6.1f} {y:<12.6f} {exact:<12.6f} {error:<12.6e}")
        
        if i < n_steps:
            y = calculator.evaluate_bseries_step(df, y, h, max_order=5)
    
    print()


def demo_logistic_growth():
    """Demonstrate B-Series solution for logistic growth: dy/dt = y(1-y)"""
    print("Demo 2: Logistic Growth (dy/dt = y(1-y))")
    print("-" * 40)
    
    calculator = BSeriesDifferentialCalculator()
    
    # Define dy/dt = y(1-y)
    def f(y):
        return y * (1 - y)
    
    def f_prime(y):
        return 1 - 2*y
    
    def f_double(y):
        return -2.0
    
    def f_triple(y):
        return 0.0
    
    def f_quad(y):
        return 0.0
    
    df = create_differential_function(f, f_prime, f_double, f_triple, f_quad, "y(1-y)")
    
    # Initial conditions
    y0 = 0.1
    h = 0.2
    n_steps = 10
    
    print(f"Differential equation: dy/dt = y(1-y)")
    print(f"Initial condition: y(0) = {y0}")
    print(f"Step size: h = {h}")
    print(f"Carrying capacity: K = 1")
    print()
    
    print(f"{'t':<6} {'B-Series':<12} {'f(y)':<12}")
    print("-" * 36)
    
    y = y0
    for i in range(n_steps + 1):
        t = i * h
        growth_rate = f(y)
        
        print(f"{t:<6.1f} {y:<12.6f} {growth_rate:<12.6f}")
        
        if i < n_steps:
            y = calculator.evaluate_bseries_step(df, y, h, max_order=4)
    
    print()


def demo_oscillatory_system():
    """Demonstrate B-Series solution for oscillatory motion: dy/dt = -y"""
    print("Demo 3: Oscillatory Motion (dy/dt = -y)")
    print("-" * 40)
    
    calculator = BSeriesDifferentialCalculator()
    
    # Define dy/dt = -y (harmonic oscillator)
    def f(y):
        return -y
    
    def f_prime(y):
        return -1.0
    
    def f_double(y):
        return 0.0
    
    def f_triple(y):
        return 0.0
    
    def f_quad(y):
        return 0.0
    
    df = create_differential_function(f, f_prime, f_double, f_triple, f_quad, "-y")
    
    # Initial conditions
    y0 = 1.0
    h = 0.1
    n_steps = 20
    
    print(f"Differential equation: dy/dt = -y")
    print(f"Initial condition: y(0) = {y0}")
    print(f"Step size: h = {h}")
    print(f"Exact solution: y(t) = e^(-t)")
    print()
    
    print(f"{'t':<6} {'B-Series':<12} {'Exact':<12} {'Error':<12}")
    print("-" * 50)
    
    y = y0
    for i in range(0, n_steps + 1, 4):  # Show every 4th step
        t = i * h
        exact = math.exp(-t)
        error = abs(y - exact)
        
        print(f"{t:<6.1f} {y:<12.6f} {exact:<12.6f} {error:<12.6e}")
        
        # Take 4 steps to get to next display point
        for _ in range(4):
            if i + _ < n_steps:
                y = calculator.evaluate_bseries_step(df, y, h, max_order=5)
    
    print()


def demo_elementary_differentials():
    """Demonstrate evaluation of individual elementary differentials"""
    print("Demo 4: Elementary Differential Evaluation")
    print("-" * 40)
    
    calculator = BSeriesDifferentialCalculator()
    
    # Define function f(y) = y²
    def f(y):
        return y * y
    
    def f_prime(y):
        return 2 * y
    
    def f_double(y):
        return 2.0
    
    def f_triple(y):
        return 0.0
    
    def f_quad(y):
        return 0.0
    
    df = create_differential_function(f, f_prime, f_double, f_triple, f_quad, "y²")
    
    y = 2.0
    print(f"Function: f(y) = y²")
    print(f"Evaluation point: y = {y}")
    print(f"f({y}) = {f(y)}, f'({y}) = {f_prime(y)}, f''({y}) = {f_double(y)}")
    print()
    
    print(f"{'Tree':<6} {'Expression':<20} {'F(τ)(y)':<12} {'α(τ)':<10} {'Contribution':<12}")
    print("-" * 70)
    
    # Show first 8 trees
    for tree_id in range(1, 9):
        try:
            tree_info = calculator.get_tree_evaluation_info(tree_id)
            f_tau = calculator.evaluate_elementary_differential(tree_id, df, y)
            alpha = tree_info["coefficient"]
            contribution = alpha * f_tau
            
            print(f"{tree_id:<6} {tree_info['expression']:<20} {f_tau:<12.6f} {alpha:<10.6f} {contribution:<12.6f}")
        except Exception as e:
            print(f"{tree_id:<6} {'Error':<20} {str(e):<12}")
    
    print()


def demo_convergence_analysis():
    """Demonstrate convergence behavior with different step sizes"""
    print("Demo 5: Convergence Analysis")
    print("-" * 40)
    
    calculator = BSeriesDifferentialCalculator()
    
    # Define dy/dt = y
    def f(y):
        return y
    
    def f_prime(y):
        return 1.0
    
    def f_double(y):
        return 0.0
    
    def f_triple(y):
        return 0.0
    
    df = create_differential_function(f, f_prime, f_double, f_triple, "y")
    
    y0 = 1.0
    t_final = 1.0
    exact_final = math.exp(1.0)
    
    print(f"Differential equation: dy/dt = y")
    print(f"Initial condition: y(0) = {y0}")
    print(f"Target time: t = {t_final}")
    print(f"Exact solution: y({t_final}) = e = {exact_final:.6f}")
    print()
    
    print(f"{'h':<10} {'Steps':<8} {'B-Series':<12} {'Error':<12} {'Order':<8}")
    print("-" * 58)
    
    step_sizes = [0.5, 0.25, 0.125, 0.0625, 0.03125]
    prev_error = None
    
    for h in step_sizes:
        n_steps = int(t_final / h)
        y = y0
        
        for _ in range(n_steps):
            y = calculator.evaluate_bseries_step(df, y, h, max_order=4)
        
        error = abs(y - exact_final)
        
        if prev_error is not None:
            convergence_order = math.log(prev_error / error) / math.log(2.0)
        else:
            convergence_order = 0.0
        
        print(f"{h:<10.5f} {n_steps:<8} {y:<12.6f} {error:<12.6e} {convergence_order:<8.2f}")
        prev_error = error
    
    print()


def main():
    """Run all B-Series differential calculator demonstrations"""
    print("B-Series Elementary Differential Calculator Demonstrations")
    print("=" * 60)
    print()
    
    try:
        demo_exponential_growth()
        demo_logistic_growth()
        demo_oscillatory_system()
        demo_elementary_differentials()
        demo_convergence_analysis()
        
        print("✅ All demonstrations completed successfully!")
        print()
        print("The B-Series elementary differential calculator provides:")
        print("• Numerical evaluation of elementary differentials F(τ)")
        print("• B-Series method step evaluation for ODE solving")
        print("• Support for functions up to 4th order derivatives")
        print("• Integration with existing B-Series tree classification")
        print("• Validation and error handling for differential functions")
        print("• Mathematical accuracy for polynomial and exponential functions")
        
    except Exception as e:
        print(f"❌ Demonstration failed: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()