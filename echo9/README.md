# Echo9 Folder Organization

This folder has been reorganized to separate Echo.Kern related content from unrelated Plan 9 port components.

## Folder Structure

### `echo-kernel-functions/`
Contains Echo.Kern DTESN-related experimental implementations and prototypes:

- **`dtesn-implementations/`** - Production DTESN component implementations
- **`kernel-modules/`** - Kernel module prototypes and development code
- **`neuromorphic-drivers/`** - Hardware abstraction layer prototypes
- **`real-time-extensions/`** - Real-time scheduler extensions and optimizations

### `plan9-port/`
Complete Plan 9 port (plan9port) from the original echo9 contents. This is a comprehensive Unix port of Plan 9 from Bell Labs, providing Plan 9 libraries and programs on Unix systems.

**Original Plan 9 Port Structure:**
- `lib/` - Plan 9 libraries 
- `src/` - Source code for Plan 9 programs
- `bin/` - Binary executables
- `man/` - Manual pages
- Documentation: `README.md`, `INSTALL`, `CONTRIBUTING.md`

### `irrelevant/`
Contains content that doesn't fit into Echo.Kern DTESN development or Plan 9 port categories:

- **`archive/`** - Archived or deprecated content

## Purpose

This reorganization separates:
1. **Echo.Kern Development** - DTESN-related prototypes and kernel development
2. **Plan 9 Port** - Complete, preserved Plan 9 port functionality  
3. **Other Content** - Miscellaneous or unrelated materials

## Usage

- For Echo.Kern DTESN development: Work in `echo-kernel-functions/`
- For Plan 9 port functionality: Use `plan9-port/` (preserves original structure)
- For archival content: Reference `irrelevant/archive/`

---

*Reorganized as part of Echo.Kern documentation analysis and folder structure optimization.*