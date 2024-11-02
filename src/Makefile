BIN=\
  test-rand\
  config-test\
  cumulative-odds\
  rev-sim\

all: $(BIN)

UTILDIR=../../idanlib/src
UTILSLIB=$(UTILDIR)/utils.a

LUADIR := $(shell $(UTILDIR)/lua-dir.sh)
LUALIB := $(shell $(UTILDIR)/lua-ver.sh)

INCLUDES=$(LUADIR) -I$(UTILDIR)
LIBS=$(UTILSLIB) -lm -lpthread -luuid $(LUALIB) -lcurl


CC=gcc
CFLAGS=-Wall -g -O2 $(INCLUDES)
# CFLAGS=-Wall -g -rdynamic -fsanitize=address $(INCLUDES)

SHAREDOBJ=\
  availability.o\
  cash-event.o\
  config.o\
  events.o\
  holiday.o\
  org.o\
  product.o\
  random.o\
  revenue.o\
  sales-rep.o\
  sales-sim.o\
  sales-stage.o\
  summary.o\
  vacation.o\

ALLOBJ=\
  $(SHAREDOBJ)\

%.o: %.c $(wildcard *.h) Makefile
	$(CC) $(CFLAGS) -c $<

$(BIN): % : %.o $(ALLOBJ) $(UTILSLIB)
	$(CC) $(CFLAGS) -o $@ $< $(ALLOBJ) $(LIBS)

install: rev-sim
	sudo install -o root -g root -m 755 rev-sim /usr/local/bin

clean: NULL
	rm -f $(SHAREDOBJ) $(BIN)
	rm -f *.o

NULL:
