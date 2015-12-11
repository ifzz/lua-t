# vim: ts=3 sw=3 st=3 sts=3 sta noet tw=80 list
#
# \file      src/test/Makefile
# \brief     Makefile for the lua-t library source code
# \author    tkieslich
# \copyright See Copyright notice at the end of t.h

T_SRC=t_tim.c \
		t_pck.c


#Create a Makefile in the out folcder and invoke there
#
LVER=5.3
PREFIX=$(shell pkg-config --variable=prefix lua)
INCDIR=$(shell pkg-config --variable=includedir lua)
INCS=-I$(INCDIR) -I../ -I./
LDFLAGS:=$(LDFLAGS) -lcrypt
# clang can be substituted with gcc (command line args compatible)
CC=clang
LD=clang
CFLAGS:=$(MYCFLAGS) -Wall -Wextra -O0 -std=c99 -fpic $(T_PRE)

T_PFX=$(addprefix test_, $(T_SRC))
T_OBJ=$(T_SRC:.c=.o)
T_EXE=$(T_SRC:.c=)

all: $(T_SRC) $(T_PFX) $(T_OBJ) t_unittest.o $(T_EXE)
#	$(foreach exe,$(T_EXE),./$(exe);)

test_%.c : %.c
	cat ../$<  $< > $@

%.o: test_%.c
	$(CC) -x c $(INCS) $(CFLAGS) -c $< -o $@

%: %.o
	mv ../$< ../$<.bak
	$(LD) t_unittest.o ../*o $< -o $@ $(LDFLAGS) $(LIBS)
	mv ../$<.bak ../$<

_unittest.o: t_unittest.c
	$(CC) $(CFLAGS) -c t_unittest.c -o t_unittest.o


clean:
	-rm $(T_PFX) $(T_OBJ) $(T_EXE) t_unittest.o
