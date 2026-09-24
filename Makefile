CC =	gcc
CFLAGS =	-Wall	-Wextra	-g
TARGET =	outer_minds.c	yyjson.c
OUTPUT =	app

all:	compile 

compile:
	$(CC)	$(CFLAGS)	$(TARGET)	-o $(OUTPUT)

test:
	$(CC) $(CFLAGS) -DTEST	$(TARGET) -o	$(OUTPUT)

test_graph:
	$(CC)	$(CFLAGS)	-DTEST_GRAPH	$(TARGET)	-o	$(OUTPUT)

clean:
	rm	$(OUTPUT)

.PHONY:	all	compile	test	test_graph	clean
