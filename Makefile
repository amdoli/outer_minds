CC =	gcc
CFLAGS =	-std=c99 -Wall	-Wextra	-Iinclude -g
TARGET =	main.c outer_minds.c	yyjson.c
OUTPUT =	app

all:	compile 

compile:
	$(CC)	$(CFLAGS)	$(TARGET)	-o $(OUTPUT)

test:
	$(CC) $(CFLAGS) -DTEST	$(TARGET) -o	$(OUTPUT)

test_graph:
	$(CC)	$(CFLAGS)	-DTEST_GRAPH	$(TARGET)	-o	$(OUTPUT)

test_read:
	$(CC) $(CFLAGS) -DTEST_READ $(TARGET) -o $(OUTPUT)

clean:
	rm	$(OUTPUT)

.PHONY:	all	compile	test	test_graph	test_read	clean
