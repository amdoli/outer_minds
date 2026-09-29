CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g -I.. -I../include
TARGET = main.c ../outer_minds.c ../yyjson.c 
OUTPUT = app
# == TEST FLAGS == 

# READ TEST
READ = -DTEST_READ_ALL 
NODE = -DTEST_READ_NODE
LINE = -DTEST_READ_LINE

# GRAPH TEST
GRAPH = -DTEST_GRAPH


all: compile

compile:
	$(CC) $(CFLAGS) $(TARGET) -o $(OUTPUT)

test_read:
	$(CC) $(CFLAGS) $(READ) $(TARGET) -o $(OUTPUT)

test_read_node:
	$(CC) $(CFLAGS) $(NODE) $(TARGET) -o $(OUTPUT)

test_read_line:
	$(CC) $(CFLAGS) $(LINE) $(TARGET) -o $(OUTPUT)

test_graph:
	$(CC) $(CFLAGS) $(GRAPH) $(TARGET) -o $(OUTPUT)

clean:
	rm $(OUTPUT)

.PHONY: all compile test_read test_read_node test_read_line test_graph clean
