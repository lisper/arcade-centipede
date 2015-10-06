# Makefile for Atari simulator
# Copyright 1994 Eric Smith
# mods 2015 Brad Parker

LDLIBS = -lm -lSDL
LOCAL_LIBRARIES =
SRCS = main.c sim6502.c debugger.c dis6502.c memory.c game.c misc.c display.c
OBJS = $(SRCS:.c=.o)

CFLAGS += -g
LDOPTIONS = -g

all:: sim

sim: $(OBJS)
	$(CC) -o $@ $(LDOPTIONS) $(OBJS) $(LOCAL_LIBRARIES) $(LDLIBS)


.c.o:
	$(CC) -c $(CFLAGS) $*.c -o $@

clean:
	$(RM) -f *.o core errs ,* *~ *.a .emacs_* tags TAGS make.log MakeOut   "#"*
	$(RM) -f sim

tags::
	$(TAGS) -w *.[ch]
	$(TAGS) -xw *.[ch] > TAGS

# ----------------------------------------------------------------------

main.o: main.c memory.h game.h display.h misc.h sim6502.h

sim6502.o: sim6502.c memory.h game.h misc.h sim6502.h macro6502.h debugger.h

debugger.o: debugger.c memory.h debugger.h sim6502.h dis6502.h game.h display.h misc.h

dis6502.o: dis6502.c memory.h

memory.o: memory.c memory.h display.h game.h misc.h sim6502.h

game.o: game.c display.h memory.h game.h misc.h

misc.o: misc.c

display.o: display.c misc.h display.h memory.h sim6502.h game.h
