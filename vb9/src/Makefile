# VB9 Portable Makefile
# For Linux/Unix systems without Plan 9

CC = gcc
CFLAGS = -Wall -O2 -std=c99
TARGET = vb9demo
SIZELIMIT = 1400000

all: $(TARGET)

$(TARGET): vb9portable.c
	$(CC) $(CFLAGS) -o $(TARGET) vb9portable.c

# Check if we're under the VB6 1.4MB limit
checksize: $(TARGET)
	@echo "VB9 Runtime Size Check:"
	@size=$(shell stat -f%z $(TARGET) 2>/dev/null || stat -c%s $(TARGET) 2>/dev/null); \
	echo "$(TARGET): $$size bytes"; \
	if [ $$size -gt $(SIZELIMIT) ]; then \
		echo "ERROR: Exceeds 1.4MB VB6 limit!"; \
		exit 1; \
	else \
		echo "SUCCESS: Under VB6 1.4MB target!"; \
	fi

# Run the demo
run: $(TARGET)
	./$(TARGET)

# Clean up
clean:
	rm -f $(TARGET)
	rm -rf forms/

# Install
install: $(TARGET) checksize
	cp $(TARGET) /usr/local/bin/vb9demo

# Show the VB6 workflow
demo: $(TARGET)
	@echo "=== The VB6 Workflow in VB9 ==="
	@echo "1. Build the runtime (1.4MB max)"
	@echo "2. Run the form designer"
	@echo "3. Draw your controls"
	@echo "4. Set event handlers"  
	@echo "5. Everything becomes files"
	@echo "6. Drawing IS computing"
	@echo ""
	./$(TARGET)

help:
	@echo "VB9 - Visual Basic for Plan 9 (Portable)"
	@echo "Targets:"
	@echo "  all       - Build the demo"
	@echo "  run       - Run the demo"
	@echo "  demo      - Show VB6 workflow"
	@echo "  checksize - Verify 1.4MB limit"
	@echo "  clean     - Remove build files"
	@echo "  install   - Install to system"

.PHONY: all run demo checksize clean install help