.PHONY: all clean build run

all: build

build:
	@echo "Building..."
	gcc Shell_project.c job_control.c -o Shell

run:
	@echo "Running..."
	./Shell

clean:
	@echo "Cleaning..."
	rm -f Shell

help:
	@echo "Available targets:"
	@echo "  make build  - Build the project"
	@echo "  make run    - Run the project"
	@echo "  make clean  - Clean build artifacts"
	@echo "  make help   - Show this help message"