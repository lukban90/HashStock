CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g -lm
OBJECTS = hashfile.o product.o tokenizer.o price.o
EXES = buildinventory runtransactions

all:
	$(MAKE) $(EXES)

buildinventory: $(OBJECTS) buildinventory.o
	$(CC) -o buildinventory $(LDFLAGS) $(OBJECTS) buildinventory.o

runtransactions: $(OBJECTS) runtransactions.o
	$(CC) -o runtransactions $(LDFLAGS) $(OBJECTS) runtransactions.o

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
