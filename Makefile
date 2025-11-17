# @file Makefile
# @brief generates "dns" binary, executes tests
#
# @author Jaroslav Mervart
# @login xmervaj00
# @date 2025-10-4
# Portable Makefile for GNU make and BSD make (EVA)
# Avoids GNU-only features like wildcard, patsubst, ifeq, pattern rules
SHELL=/usr/bin/env bash
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -Wshadow -pedantic
INCLUDES = -Iinclude

SRC_DIR = src
OBJ_DIR = build

TARGET = dns

SRCS = \
    src/common.cpp \
    src/dns_parser.cpp \
    src/filter.cpp \
    src/main.cpp \
    src/resolver.cpp \
    src/sig.cpp \
    src/udp.cpp

OBJS = \
    build/common.o \
    build/dns_parser.o \
    build/filter.o \
    build/main.o \
    build/resolver.o \
    build/sig.o \
    build/udp.o
	


all: $(TARGET)

$(TARGET): $(OBJ_DIR) $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Explicit rules for EACH object file
build/common.o: src/common.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/common.cpp -o build/common.o

build/dns_parser.o: src/dns_parser.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/dns_parser.cpp -o build/dns_parser.o

build/filter.o: src/filter.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/filter.cpp -o build/filter.o

build/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/main.cpp -o build/main.o

build/resolver.o: src/resolver.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/resolver.cpp -o build/resolver.o

build/sig.o: src/sig.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/sig.cpp -o build/sig.o

build/udp.o: src/udp.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c src/udp.cpp -o build/udp.o

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

debug: CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -Wshadow -pedantic -O0 -g -DDEBUG_PRINT
debug: clean all

test_parse_name:
	$(CXX) $(CXXFLAGS) $(INCLUDES) testing/test_parse_name.cpp \
		src/common.cpp src/dns_parser.cpp \
		-o testing/test_dns_name

test: all
	$(MAKE) test_parse_name
	$(SHELL) testing/test.sh

clean:
	rm -rf $(OBJ_DIR) $(TARGET) testing/test_dns_name

.PHONY: all clean test test_parse_name
