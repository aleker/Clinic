IDIR =../include
CC=gcc
CFLAGS= -Wall -I$(IDIR)
ODIR=obj
LDIR =../lib

_DEPS = inf122464_pfun.h inf122464_rfun.h inf122464_lfun.h inf122464_all.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ1 = inf122464_p.o inf122464_pfun.o inf122464_all.o
OBJ1 = $(patsubst %,$(ODIR)/%,$(_OBJ1))
_OBJ2 = inf122464_r.o inf122464_rfun.o inf122464_all.o
OBJ2 = $(patsubst %,$(ODIR)/%,$(_OBJ2))
_OBJ3 = inf122464_l.o inf122464_lfun.o inf122464_all.o
OBJ3 = $(patsubst %,$(ODIR)/%,$(_OBJ3))

#------------------------------------
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: Patient Registration Doctor

Patient: $(OBJ1)
	gcc -o $@ $^ $(CFLAGS)
Registration: $(OBJ2)
	gcc -o $@ $^ $(CFLAGS)
Doctor: $(OBJ3)
	gcc -o $@ $^ $(CFLAGS)
#------------------------------------
.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 