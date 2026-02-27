# Generic target that runs a shell script with the target name as argument
build-%:
	chmod +x gen_chdsect.sh
	mkdir -p build-out
	@echo "Running target: $*"
	@./gen_chdsect.sh "$*"

SRCS := $(wildcard *.s)

# Build all .s files
all: $(SRCS:%=build-%)
	@echo "All chdsect headers generated."

# Help target
help:
	@echo "Usage: make build-<target_name>"
	@echo "This will run: ./gen_chdsect.sh <target_name>"
	@echo ""
	@echo "Examples:"
	@echo "  make build-cvt.s    -> ./gen_chdsect.sh cvt.s"
	@echo "  make build-dcbd.s   -> ./gen_chdsect.sh dcbd.s"

# Prevent make from trying to build files that don't exist
.PHONY: % all help