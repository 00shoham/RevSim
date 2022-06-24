BIN=\
  test-rand\
  config-test\
  cumulative-odds\
  simulate\

all: $(BIN)

LUADIR=/usr/include/lua5.3
UTILDIR=../../idanlib/src
UTILSLIB=$(UTILDIR)/utils.a

INCLUDES=-I$(LUADIR) -I$(UTILDIR)
LIBS=$(UTILSLIB) -lm -lpthread -luuid -llua5.3 -lcurl

CC=gcc
CFLAGS=-Wall -O2 $(INCLUDES)
# CFLAGS=-Wall -g -rdynamic -fsanitize=address $(INCLUDES)

SHAREDOBJ=\
  availability.o\
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

install: simulate
	sudo install -o root -g root -m 755 simulate /usr/local/bin

clean: NULL
	rm -f $(SHAREDOBJ) $(BIN)
	rm -f *.o

NULL:
