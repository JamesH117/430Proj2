all: main.c
	gcc -o raycast main.c

clean:
	rm -rf raycast *~

