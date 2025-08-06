# Deep Tree Echo Kernel Development Instructions

**ALWAYS follow these instructions first. Only search for additional information or use bash commands if the information here is incomplete or found to be in error.**

## Working Effectively

### Initial Setup & Dependencies
- Ensure Python 3 is available: `python3 --version` (tested with Python 3.12.3)
- Ensure Node.js is available: `node --version` (tested with Node.js v20.19.4)
- Repository is standalone - **NO BUILD STEP REQUIRED**
- **NO PACKAGE INSTALLATION NEEDED** - all dependencies are standard library

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
- **Execution time**: < 0.02 seconds
- **Output**: Creates/updates `echo_kernel_specification.md`

### Validation & Testing

#### Essential Validation Steps
1. **Server Response Test**:
   ```bash
   curl -I localhost:8000
   # Expected: HTTP/1.0 200 OK, Content-type: text/html
   ```

2. **JavaScript Syntax Validation**:
   ```bash
   node -c app.js
   # Expected: No output (success), exit code 0
   ```

3. **Python Script Execution**:
   ```bash
   time python3 echo_kernel_spec.py
   # Expected: Success message, real time < 0.1s
   ```

4. **File Integrity Check**:
   ```bash
   # Verify all core files are accessible
   curl -s localhost:8000/app.js | wc -l  # Should return 703
   curl -s localhost:8000/style.css | wc -l  # Should return 1314+
   curl -s localhost:8000 | grep -q "Deep Tree Echo"  # Should find title
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
- **Server startup**: < 1 second
- **Page load time**: < 2 seconds (dependent on browser)
- **JavaScript execution**: Immediate (modern browsers)
- **Interactive response**: < 100ms for user actions
- **Python script**: < 0.02 seconds execution time

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
   # Or check what's using port 8000
   lsof -i :8000
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
1. **ALWAYS** validate syntax: `node -c app.js` 
2. **ALWAYS** refresh browser and test user scenarios
3. **ALWAYS** check browser console for new errors
4. **ALWAYS** test interactive features work correctly
5. If changing Python code: `python3 echo_kernel_spec.py`

#### **Testing Checklist**
- [ ] Server starts successfully
- [ ] All files load without 404 errors
- [ ] No JavaScript console errors
- [ ] Interactive nodes respond to hover/click
- [ ] Reflection panel opens and accepts input
- [ ] New echoes can be added and appear correctly
- [ ] Animations and visual effects work
- [ ] Responsive design functions properly
- [ ] Python script executes successfully (if modified)

### **Critical Reminders**
- **NEVER CANCEL server processes** - let them run until testing complete
- **ALWAYS test manual scenarios** - automated checks don't cover user interaction
- **No build system exists** - changes are immediately reflected on refresh
- **All dependencies are built-in** - no npm install or pip install needed
- **Application is fully self-contained** - works offline except for external font
- **Timing is fast** - if operations take more than 1 second, investigate issues

This is a **real-time interactive application**. Always validate that user interactions work correctly before considering changes complete.