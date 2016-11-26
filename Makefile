C := gcc
CFLAGS := -Wall -Werror -std=c99 -g

Src := IndexChain FATImage Helpers
Obj := $(addsuffix .o, $(Src))

default: main.o $(Obj)
	@$(C) $(CFLAGS) -o dos_scandisk $^ -lm

test: test.o $(Obj)
	@$(C) $(CFLAGS) -o fat_test $^ -lm
	@./fat_test

clean: 
	@rm -rf *.o
	@rm -rf test
	@rm -rf dos_scandisk

test.o: test.c HelpersTest.h FATImageTest.h IndexChainTest.h
	@$(C) $(CFLAGS) -o $@ -c $<

%.o: %.c
	@$(C) $(CFLAGS) -o $@ -c $^