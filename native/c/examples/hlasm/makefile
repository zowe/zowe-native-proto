# Assembler program makefile

# Default assembler and linker
AS=as
LD=ld

# Source files
SOURCES = testasm1.s testasm4.s testaut1.s

# Object files
OBJECTS = $(SOURCES:.s=.o)

# Executables
EXECUTABLES = $(SOURCES:.s=)

# Default target
all: $(EXECUTABLES)

# Rule to build executables from object files
%: %.o
	$(LD) -o $@ $<

# Rule to build object files from source files
%.o: %.s
	$(AS) -a=$*.s.lst -o $@ $<

# Clean target
clean:
	rm -f $(OBJECTS) $(EXECUTABLES)

# Run target (example for testauth)
run-testaut1: testaut1
	./testaut1

# Run target (example for testasm1)
run-testasm1: testasm1
	./testasm1

# Run target (example for testasm4)
run-testasm4: testasm4
	./testasm4

.PHONY: all clean run-testaut1 run-testasm1 run-testasm4