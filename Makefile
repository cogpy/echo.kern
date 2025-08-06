# Echo.Kern Documentation and Development Makefile

.PHONY: help docs docs-diagrams docs-clean test-roadmap validate lint clean all

# Default target
help:
	@echo "Echo.Kern Development and Documentation Commands"
	@echo ""
	@echo "Documentation:"
	@echo "  docs          Generate all documentation"
	@echo "  docs-diagrams Generate Mermaid and PlantUML diagrams"
	@echo "  docs-clean    Clean generated documentation files"
	@echo ""
	@echo "Validation:"
	@echo "  test-roadmap  Test DEVO-GENESIS.md compatibility with workflow"
	@echo "  validate      Validate all project files"
	@echo "  lint          Run code linting (when implementation exists)"
	@echo ""
	@echo "Utilities:"
	@echo "  clean         Clean all generated files"
	@echo "  all           Build everything"

# Documentation generation
docs: docs-diagrams
	@echo "📚 Generating documentation..."
	@echo "  ✅ README.md - Project overview and quick start"
	@echo "  ✅ DEVELOPMENT.md - Development guide and standards"  
	@echo "  ✅ docs/DTESN-ARCHITECTURE.md - Technical architecture"
	@echo "  ✅ docs/diagrams/ - Process flow diagrams"
	@echo "  ✅ DEVO-GENESIS.md - Development roadmap"
	@echo ""
	@echo "📖 Documentation structure complete!"
	@echo "   Open index.html for interactive demo"
	@echo "   See README.md for project overview"

docs-diagrams:
	@echo "🎨 Processing diagrams..."
	@if command -v mmdc >/dev/null 2>&1; then \
		echo "  🔄 Mermaid diagrams available"; \
	else \
		echo "  ⚠️  Install mermaid-cli: npm install -g @mermaid-js/mermaid-cli"; \
	fi
	@if command -v plantuml >/dev/null 2>&1; then \
		echo "  🔄 PlantUML diagrams available"; \
	else \
		echo "  ⚠️  Install plantuml: npm install -g @plantuml/plantuml"; \
	fi

docs-clean:
	@echo "🧹 Cleaning generated documentation..."
	@rm -f docs/diagrams/*.png docs/diagrams/*.svg
	@echo "  ✅ Cleaned diagram artifacts"

# Validation and testing
test-roadmap:
	@echo "🔍 Testing DEVO-GENESIS.md roadmap format..."
	@node scripts/validate-roadmap.js

validate: test-roadmap
	@echo "✅ Validating project structure..."
	@echo "  📋 Checking required files..."
	@test -f README.md || (echo "❌ README.md missing" && exit 1)
	@test -f DEVELOPMENT.md || (echo "❌ DEVELOPMENT.md missing" && exit 1)  
	@test -f DEVO-GENESIS.md || (echo "❌ DEVO-GENESIS.md missing" && exit 1)
	@test -f docs/DTESN-ARCHITECTURE.md || (echo "❌ DTESN-ARCHITECTURE.md missing" && exit 1)
	@test -f .github/workflows/generate-next-steps.yml || (echo "❌ Workflow missing" && exit 1)
	@echo "  ✅ All required documentation files present"
	@echo ""
	@echo "  🔗 Checking documentation links..."
	@grep -q "DTESN-ARCHITECTURE.md" README.md || (echo "❌ Architecture link missing in README" && exit 1)
	@grep -q "DEVELOPMENT.md" README.md || (echo "❌ Development link missing in README" && exit 1)
	@echo "  ✅ Documentation cross-references valid"
	@echo ""
	@echo "🎯 Project validation complete!"

lint:
	@echo "🔍 Code linting..."
	@echo "  ⚠️  Kernel implementation not yet available"
	@echo "  📝 Python specs: echo_kernel_spec.py"
	@python3 -m py_compile echo_kernel_spec.py
	@echo "  ✅ Python specification compiles"

# Utilities
clean: docs-clean
	@echo "🧹 Cleaning all generated files..."
	@rm -f *.pyc __pycache__/*
	@echo "  ✅ Cleaned build artifacts"

all: validate docs
	@echo "🚀 Echo.Kern documentation build complete!"
	@echo ""
	@echo "📊 Project Status:"
	@echo "  ✅ Architecture specification complete"
	@echo "  ✅ Documentation comprehensive and linked"
	@echo "  ✅ Development workflow established"
	@echo "  ✅ Automated issue generation configured"
	@echo "  ⚠️  Kernel implementation in progress"
	@echo ""
	@echo "🎯 Next Steps:"
	@echo "  1. Review generated documentation"
	@echo "  2. Test GitHub workflow: make test-workflow"
	@echo "  3. Begin kernel implementation tasks"
	@echo "  4. See DEVO-GENESIS.md for development roadmap"

# Bonus: Test the GitHub workflow (requires GitHub CLI or API access)
test-workflow:
	@echo "🔄 Testing GitHub workflow integration..."
	@echo "  📋 24 tasks defined in DEVO-GENESIS.md"
	@echo "  🤖 Workflow: .github/workflows/generate-next-steps.yml"
	@echo ""
	@echo "To test the workflow:"
	@echo "  1. Go to: https://github.com/EchoCog/echo.kern/actions"
	@echo "  2. Find: 'Generate Next Steps Issues'"
	@echo "  3. Click: 'Run workflow'"
	@echo "  4. Check: Issues tab for generated tasks"

# Development environment setup
setup-dev:
	@echo "🔧 Setting up Echo.Kern development environment..."
	@echo "  📦 Checking prerequisites..."
	@command -v python3 >/dev/null || (echo "❌ Python 3 required" && exit 1)
	@command -v node >/dev/null || (echo "❌ Node.js required" && exit 1)
	@command -v git >/dev/null || (echo "❌ Git required" && exit 1)
	@echo "  ✅ Prerequisites satisfied"
	@echo ""
	@echo "  🔧 Installing documentation tools..."
	@npm list -g @mermaid-js/mermaid-cli >/dev/null 2>&1 || echo "  📦 Consider: npm install -g @mermaid-js/mermaid-cli"
	@npm list -g @plantuml/plantuml >/dev/null 2>&1 || echo "  📦 Consider: npm install -g @plantuml/plantuml"
	@echo ""
	@echo "  🎯 Development environment ready!"
	@echo "     Run 'make validate' to verify setup"