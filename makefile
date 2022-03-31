all: clean lab2 
	./a.out
lab2: lab2.c
	gcc lab2.c -Wall -Werror -Wextra
clean:
	rm -rf *.tmp *.out
	