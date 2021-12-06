all:
	@echo "Targets: clean, build, run"

build:
	@gcc main.c -o main

clean:
	rm -f main

run: build
	@echo
	@sudo ./main ./main.c
	@echo
