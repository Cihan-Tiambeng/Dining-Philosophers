all: phil

phil: phil.c
	gcc -Wall -o phil phil.c  -L. -lm -lpthread
