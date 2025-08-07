# Deep Tree Echo Kernel Development Instructions

**ALWAYS follow these instructions first. Only search for additional information or use bash commands if the information here is incomplete or found to be in error.**

## Working Effectively

### Initial Setup & Dependencies
- Ensure Python 3 is available: `python3 --version` (minimum 3.6, tested with Python 3.12.3)
- Ensure Node.js is available: `node --version` (minimum 14.0.0, tested with Node.js v20.19.4)
- Repository is standalone - **NO BUILD STEP REQUIRED**
- **NO PACKAGE INSTALLATION NEEDED** - all dependencies are standard library

### **Optional Dependencies for Advanced Testing**
```bash
# Optional: For ESN reservoir tests (not required for basic functionality)
pip install numpy scipy  # Only install if you need test_esn_reservoir.py

# Optional: For diagram generation (mentioned in Makefile)
npm install -g @mermaid-js/mermaid-cli
npm install -g @plantuml/plantuml
```
**NOTE**: The core application works completely without these optional dependencies.

### Core Application Components
1. **Interactive Web Application**: Deep Tree Echo visualization system
   - `index.html` - Main application page (13,221 bytes)
   - `app.js` - JavaScript application logic (25,987 bytes, 703 lines)
   - `style.css` - Comprehensive styling (34,091 bytes, 1314+ lines)

2. **Documentation Generator**: Python-based specification generator
   - `echo_kernel_spec.py` - Generates technical specifications (16,820 bytes)

### Running the Application

#### Web Application
```bash
# Preferred: Python HTTP server (instant startup)
python3 -m http.server 8000

# Alternative: Node.js with npx (if needed)
npx http-server -p 8000
```
- **Startup time**: Instant (< 1 second)
- **Access**: Open `http://localhost:8000` in web browser
- **NEVER CANCEL**: Let server run as long as needed

#### Documentation Generator
```bash
# Generate technical specifications (fast execution)
python3 echo_kernel_spec.py
```
- **Execution time**: ~0.08 seconds (validated)
- **Output**: Creates/updates `echo_kernel_specification.md`
- **NEVER CANCEL**: Fast command, but let it complete

### Validation & Testing

#### Essential Validation Steps
1. **Server Response Test**:
   ```bash
   curl -I localhost:8000
   # Expected: HTTP/1.0 200 OK, Content-type: text/html
   # **Timeout**: 30 seconds (network dependent)
   ```

2. **JavaScript Syntax Validation**:
   ```bash
   node -c app.js
   # Expected: No output (success), exit code 0
   # **Timeout**: 10 seconds
   ```

3. **Python Script Execution**:
   ```bash
   time python3 echo_kernel_spec.py
   # Expected: Success message, real time ~0.08s
   # **Timeout**: 30 seconds
   ```

4. **File Integrity Check**:
   ```bash
   # Verify all core files are accessible
   curl -s localhost:8000/app.js | wc -l  # Should return 703
   curl -s localhost:8000/style.css | wc -l  # Should return 1314+
   curl -s localhost:8000 | grep -q "Deep Tree Echo"  # Should find title
   # **Timeout**: 60 seconds per command
   ```

#### **Testing Framework Validation**
Run the comprehensive test suite to validate functionality:
```bash
# OEIS enumeration tests - NEVER CANCEL
python3 test_oeis_a000081.py
# Expected: 7/7 tests passed, ~1-3 seconds

# P-System membrane tests - NEVER CANCEL  
python3 test_psystem_membranes.py
# Expected: 35/35 tests passed, ~5-10 seconds

# Memory layout validation - NEVER CANCEL
python3 test_memory_layout_validator.py  
# Expected: 15/15 tests passed, ~2-5 seconds

# Quick validation suite - NEVER CANCEL
make test-quick
# Expected: All validations pass, ~3-5 seconds

# Full project validation - NEVER CANCEL, timeout 10 minutes
make validate
# Expected: Project validation complete, ~30-60 seconds

# Performance benchmarks - NEVER CANCEL, timeout 10 minutes
make test-performance  
# Expected: Detailed performance report, ~1-2 seconds
# Note: Some performance targets may not be met in development phase

# Code linting - NEVER CANCEL
make lint
# Expected: All syntax valid, ~2-5 seconds
```

#### **CRITICAL: Manual User Scenario Testing**
After making any changes, **ALWAYS test the following user scenarios**:

1. **Basic Application Load**:
   - Open `http://localhost:8000` in browser
   - Verify page loads completely (no 404s or missing resources)
   - Check browser console for JavaScript errors (F12 → Console)
   - **Expected**: Title "Deep Tree Echo - Living Memory & Distributed Cognition" visible
   - **Expected**: Interactive tree visualization with colored nodes visible
   - **Expected**: Console shows "Page fully loaded, nodes available: 8"

2. **Interactive Echo Node Testing**:
   - Hover over different colored nodes in the tree visualization
   - Click on nodes to trigger concept displays (should show concept name and description)
   - Verify hover effects and animations work
   - Test keyboard navigation (Tab key to move between nodes)
   - **Expected**: Nodes respond with visual feedback and information panels

3. **Reflection Panel Testing**:
   - Click the chat icon to open reflection panel (right side of screen)
   - Type test text in the echo input area: "Test echo - validation successful"
   - Click "Add Echo" button
   - **Expected**: New echo appears at top of "Recent Echoes" list
   - **Expected**: Console shows "Adding user echo:" and "Creating new node at:"
   - **Expected**: Input field clears after adding echo

4. **End-to-End Echo Creation Validation**:
   - Add a test echo using the reflection panel
   - Verify the new echo appears in the visualization as a new interactive node
   - Click on the newly created node to verify it shows your custom text
   - **Expected**: Complete round-trip from input → display → interaction works

5. **Responsive Design Testing**:
   - Test on different browser window sizes
   - Verify mobile-friendly behavior (width < 768px)
   - Check that all interactive elements remain functional

#### **Performance Expectations**
- **Server startup**: Instant (<1 second)
- **Page load time**: <2 seconds (dependent on browser)
- **JavaScript execution**: Immediate (modern browsers) 
- **Interactive response**: <100ms for user actions
- **Python script**: ~0.08 seconds execution time (validated)
- **Test suite execution**: 1-10 seconds per test file
- **Performance benchmarks**: ~1-2 seconds for full suite
- **Full validation**: 30-60 seconds for make validate

#### **External Dependencies**
- **ONE EXTERNAL FONT**: `https://r2cdn.perplexity.ai/fonts/FKGroteskNeue.woff2`
  - **Non-critical**: Application works without it (fallback fonts available)
  - **Network dependent**: May fail in restricted environments
  - **Expected behavior**: Browser console shows "Failed to load resource" for font - this is NORMAL
  - **No action required**: Application functionality unaffected

#### **Console Output Expectations**
**Normal/Expected Console Messages**:
```
[LOG] Initializing Deep Tree Echo...
[LOG] Page fully loaded, nodes available: 8
[ERROR] Failed to load resource: net::ERR_BLOCKED_BY_CLIENT.Inspector (for external font - IGNORE)
```

**After Adding Echo**:
```
[LOG] Adding user echo: [your text]
[LOG] Creating new node at: [coordinates]
```

### Code Structure & Navigation

#### **Key JavaScript Components** (app.js)
- `DeepTreeEcho` class (main application controller)
- Event handling for node interactions
- Animation systems for visual effects
- User echo management and persistence
- Real-time interaction feedback

#### **Key Areas for Changes**
- **Concepts Configuration**: Lines 6-46 in app.js (`this.concepts` object)
- **Node Interactions**: Lines 150-180 (click/hover handlers)
- **User Echo System**: Lines 394-440 (add/display functionality)
- **Styling Variables**: Lines 1-50 in style.css (CSS custom properties)

#### **Important Files to Monitor**
- When changing `app.js`: Always check browser console for errors
- When changing `style.css`: Verify visual integrity across viewport sizes
- When changing `index.html`: Validate HTML structure and SVG elements
- When updating documentation: Re-run `python3 echo_kernel_spec.py`

### Troubleshooting

#### **Common Issues**
1. **Server won't start on port 8000**:
   ```bash
   # Try alternative port
   python3 -m http.server 8080
   # Or check what's using port 8000 (choose the command for your OS)
   lsof -i :8000                # macOS/Linux (if lsof is available)
   ss -tuln | grep 8000         # Linux (if ss is available)
   netstat -an | grep 8000      # macOS/Linux (if netstat is available)
   netstat -ano | find "8000"   # Windows (Command Prompt)
   ```

2. **JavaScript errors in browser**:
   - Check browser console (F12 → Console)
   - Verify all files are served correctly (Network tab)
   - Common cause: SVG namespace issues or DOM element access

3. **Python script fails**:
   ```bash
   # Check Python version and basic functionality
   python3 --version
   python3 -c "print('Python working')"
   ```

4. **Interactive features not working**:
   - Verify JavaScript loaded completely (check Network tab)
   - Check for console errors
   - Ensure DOM content loaded event fired

5. **Test failures (some expected)**:
   ```bash
   # Some tests require numpy (not installed by default)
   python3 test_esn_reservoir.py  # May fail with "ModuleNotFoundError: No module named 'numpy'"
   
   # Performance tests may show failures for development targets
   make test-performance  # Some thresholds intentionally strict for future implementation
   ```

6. **Alternative server ports**:
   ```bash
   # If port 8000 is busy, try alternative ports
   python3 -m http.server 8080
   python3 -m http.server 3000
   npx http-server -p 8000  # Alternative using Node.js
   ```

#### **Debugging Commands**
```bash
# Check server is responding
curl -v localhost:8000

# Verify file sizes are correct
ls -la *.html *.js *.css

# Test JavaScript syntax across all files
for f in *.js; do echo "Checking $f"; node -c "$f" && echo "OK"; done

# Monitor server logs
python3 -m http.server 8000 2>&1 | tee server.log
```

### Development Workflow

#### **Before Making Changes**
1. Start the web server: `python3 -m http.server 8000`
2. Open application in browser: `http://localhost:8000`
3. Verify current functionality works as expected
4. Test all interactive scenarios (see Manual Testing section above)

#### **After Making Changes**
1. **ALWAYS** validate syntax: `node -c app.js` (timeout: 10 seconds)
2. **ALWAYS** refresh browser and test user scenarios (timeout: 5 minutes manual testing)
3. **ALWAYS** check browser console for new errors (F12 → Console)
4. **ALWAYS** test interactive features work correctly (see Manual Testing section)
5. If changing Python code: `python3 echo_kernel_spec.py` (timeout: 30 seconds)
6. **RECOMMENDED**: Run test suite: `make test-quick` (timeout: 5 minutes - NEVER CANCEL)
7. **RECOMMENDED**: Validate project: `make validate` (timeout: 10 minutes - NEVER CANCEL)

#### **Testing Checklist**
- [ ] Server starts successfully (`python3 -m http.server 8000` - timeout: 30 seconds)
- [ ] All files load without 404 errors (`curl` tests - timeout: 60 seconds each)
- [ ] No JavaScript console errors (F12 → Console in browser)
- [ ] Interactive nodes respond to hover/click (manual testing)
- [ ] Reflection panel opens and accepts input (manual testing)
- [ ] New echoes can be added and appear correctly (manual testing)
- [ ] Animations and visual effects work (manual testing)
- [ ] Responsive design functions properly (manual testing)
- [ ] Python script executes successfully: `python3 echo_kernel_spec.py` (timeout: 30 seconds)
- [ ] **COMPREHENSIVE**: Test suite passes: `make test-quick` (timeout: 5 minutes - NEVER CANCEL)
- [ ] **COMPREHENSIVE**: Project validates: `make validate` (timeout: 10 minutes - NEVER CANCEL)
- [ ] **OPTIONAL**: Performance benchmarks: `make test-performance` (timeout: 10 minutes - NEVER CANCEL)

### **Critical Reminders**
- **NEVER CANCEL server processes** - let them run until testing complete  
- **NEVER CANCEL test commands** - all tests complete within 10 minutes maximum
- **ALWAYS test manual scenarios** - automated checks don't cover user interaction
- **No build system exists** - changes are immediately reflected on refresh
- **All dependencies are built-in** - no npm install or pip install needed  
- **Application is fully self-contained** - works offline except for external font
- **Timing is fast** - if operations take more than expected times, investigate issues
- **Set appropriate timeouts**: Use 60+ minutes for any potential build commands, 30+ minutes for comprehensive tests
- **External dependency tolerance**: Some tests may require numpy/scipy (optional for basic functionality)

This is a **real-time interactive application**. Always validate that user interactions work correctly before considering changes complete.

---

## 🔧 Comprehensive Development Standards

### Code Styles and Standards

#### **C/C++ (Kernel Implementation)**
Follow Linux kernel coding style with DTESN-specific extensions:

**Naming Conventions:**
- Functions: `dtesn_component_action()` format
- Variables: `lower_case_with_underscores`
- Constants: `UPPER_CASE_WITH_UNDERSCORES`
- Structures: `struct dtesn_component_name`

**Function Documentation:**
```c
/**
 * dtesn_membrane_evolve - Evolve P-system membrane state
 * @membrane: Target membrane structure
 * @input: Input vector for evolution
 * @timestamp_ns: Current timestamp in nanoseconds
 *
 * Evolves the membrane state according to P-lingua rules and
 * OEIS A000081 topological constraints.
 *
 * Return: 0 on success, negative error code on failure
 */
int dtesn_membrane_evolve(struct dtesn_membrane *membrane,
                         const float *input,
                         uint64_t timestamp_ns);
```

**Real-time Requirements:**
- All functions must meet strict timing constraints
- Document timing requirements in function headers
- Use `__attribute__((always_inline))` for critical paths
- Avoid dynamic memory allocation in real-time contexts

#### **Python (Specifications and Tools)**
Follow PEP 8 with DTESN-specific conventions:

**Naming Conventions:**
- Classes: `CamelCase` (e.g., `DTESNSpecification`)
- Functions/variables: `snake_case`
- Constants: `UPPER_CASE`
- Private members: `_underscore_prefix`

**Documentation Standards:**
```python
class DTESNSpecification:
    """DTESN architecture specification generator.
    
    This class generates kernel specifications based on OEIS A000081
    enumeration and DTESN mathematical foundations.
    
    Attributes:
        max_depth: Maximum tree depth for membrane structure
        oeis_sequence: A000081 enumeration values
    """
    
    def generate_membrane_hierarchy(self, max_depth: int) -> List[MembraneSpec]:
        """Generate P-system membrane hierarchy.
        
        Args:
            max_depth: Maximum tree depth for membrane structure
            
        Returns:
            List of membrane specifications following A000081 enumeration
            
        Raises:
            ValueError: If max_depth exceeds OEIS sequence bounds
        """
        pass
```

**Mathematical Rigor:**
- All implementations must be faithful to OEIS A000081 enumeration
- Include mathematical references in docstrings
- Validate inputs against mathematical constraints
- Use type hints consistently

#### **JavaScript (Documentation and Demos)**
Use modern ES6+ syntax with consistent patterns:

**Naming Conventions:**
- Classes: `PascalCase` (e.g., `DeepTreeEchoVisualization`)
- Functions/variables: `camelCase`
- Constants: `UPPER_CASE`
- Private members: `#privateField` or `_underscore` for older compatibility

**Documentation Standards:**
```javascript
class DeepTreeEchoVisualization {
    /**
     * Interactive visualization of DTESN concepts
     * @param {HTMLElement} container - Container element for visualization
     * @param {Object} options - Configuration options
     * @param {number} options.maxDepth - Maximum tree depth to display
     */
    constructor(container, options = {}) {
        this.container = container;
        this.maxDepth = options.maxDepth || 8;
        this.initializeVisualization();
    }
    
    /**
     * Animate membrane evolution process
     * @param {Array<MembraneState>} membraneStates - Current membrane states
     * @returns {Promise<void>} Animation completion promise
     */
    async animateMembraneEvolution(membraneStates) {
        // Implementation follows OEIS A000081 topology
    }
}
```

**Performance Standards:**
- Interactive response time: < 100ms
- Animation frame rate: 60fps target
- Memory usage: Minimal DOM manipulation
- Use requestAnimationFrame for smooth animations

### **Mathematical Foundation Requirements**

#### **OEIS A000081 Compliance**
All tree structures must follow the enumeration: `1, 1, 2, 4, 9, 20, 48, 115, 286, 719, 1842, 4766, 12486, ...`

```python
def verify_a000081_compliance(tree_structure):
    """Verify that tree structure follows OEIS A000081 enumeration."""
    # Must validate against sequence for given depth
    expected_counts = [1, 1, 2, 4, 9, 20, 48, 115, 286, 719, 1842, 4766]
    depth = get_tree_depth(tree_structure)
    if depth < len(expected_counts):
        return count_rooted_trees(tree_structure) == expected_counts[depth]
    return validate_extended_sequence(tree_structure)
```

#### **DTESN Architecture Principles**
1. **P-System Membrane Computing**: Hierarchical membranes based on rooted trees
2. **B-Series Rooted Tree Ridges**: Elementary differential computation 
3. **Echo State Networks**: Reservoir state evolution with ODE integration

#### **Real-time Performance Constraints**
All implementations must meet these timing requirements:

| Component | Max Latency | Test Command |
|-----------|-------------|--------------|
| Membrane Evolution | ≤ 10μs | Kernel space critical |
| B-Series Computation | ≤ 100μs | Mathematical operators |
| ESN Reservoir Update | ≤ 1ms | Learning algorithms |
| Context Switch | ≤ 5μs | Kernel scheduling |

### **Development Workflow Standards**

#### **Before Code Changes**
1. **Mathematical Validation**: Verify OEIS A000081 compliance
2. **Performance Baseline**: Measure timing for affected components  
3. **Documentation Review**: Check mathematical foundations
4. **Test Infrastructure**: Ensure relevant tests exist

#### **During Development**
1. **Incremental Testing**: Test each component change immediately
2. **Real-time Validation**: Verify timing constraints continuously
3. **Mathematical Accuracy**: Cross-reference with specifications
4. **Documentation Updates**: Update as changes are made

#### **Code Review Checklist**
- [ ] OEIS A000081 enumeration correctly implemented
- [ ] Real-time timing constraints validated
- [ ] Mathematical foundations documented with references
- [ ] Performance requirements met and tested
- [ ] Error handling follows kernel conventions
- [ ] Memory management appropriate for context
- [ ] Documentation comprehensive and accurate

### **Build System Integration**

#### **Required Make Targets**
```bash
# Validation and Testing
make test-oeis          # OEIS A000081 compliance tests
make test-timing        # Real-time performance validation  
make test-math          # Mathematical foundation tests
make test-kernel        # Kernel-specific functionality

# Code Quality
make lint-kernel        # C/C++ kernel code linting
make lint-python        # Python specification linting
make lint-javascript    # JavaScript documentation linting
make validate-dtesn     # DTESN architecture validation

# Documentation
make docs-math          # Mathematical foundation docs
make docs-api           # API documentation generation
make docs-kernel        # Kernel interface documentation
```

#### **Performance Monitoring**
Continuous integration must validate:
- Timing constraint compliance
- Mathematical accuracy
- OEIS sequence correctness
- Memory usage patterns
- Real-time determinism

### **Error Handling Standards**

#### **Kernel Code (C/C++)**
```c
// Use standard kernel error codes
#define DTESN_EINVAL_MEMBRANE  -EINVAL  // Invalid membrane structure
#define DTESN_ENOTIME         -ETIME   // Real-time constraint violated
#define DTESN_EMATH           -EDOM    // Mathematical constraint violation

int dtesn_function(struct dtesn_data *data) {
    if (!data || !data->membrane)
        return -EINVAL;
    
    if (!oeis_a000081_validate(data->tree))
        return DTESN_EMATH;
        
    // Implementation...
    return 0;
}
```

#### **Python Code**
```python
class DTESNError(Exception):
    """Base exception for DTESN operations."""
    pass

class OEISComplianceError(DTESNError):
    """Raised when OEIS A000081 constraints are violated."""
    pass

class PerformanceConstraintError(DTESNError):
    """Raised when real-time constraints cannot be met."""
    pass
```

#### **JavaScript Code**
```javascript
class DTESNVisualizationError extends Error {
    constructor(message, code) {
        super(message);
        this.name = 'DTESNVisualizationError';
        this.code = code;
    }
}

// Usage
if (!this.validateOEISStructure(tree)) {
    throw new DTESNVisualizationError(
        'Tree structure violates OEIS A000081 enumeration', 
        'OEIS_VIOLATION'
    );
}
```

### **Critical Implementation Guidelines**

1. **Mathematical Rigor**: All code must be mathematically sound and verifiable
2. **Real-time Determinism**: No unbounded operations in critical paths
3. **OEIS Compliance**: Tree structures must follow A000081 enumeration exactly
4. **Modular Design**: Follow microkernel principles for isolation
5. **Energy Efficiency**: Optimize for neuromorphic hardware power constraints
6. **Documentation**: Mathematical foundations and timing requirements must be documented
7. **Testing**: Comprehensive validation of mathematical and performance requirements

**When implementing any DTESN component, always verify:**
- ✅ OEIS A000081 enumeration compliance
- ✅ Real-time timing constraints met  
- ✅ Mathematical foundations correct
- ✅ Performance requirements satisfied
- ✅ Error handling comprehensive
- ✅ Documentation complete and accurate

This ensures Echo.Kern maintains its core mission of providing a mathematically rigorous, real-time neuromorphic computing platform.