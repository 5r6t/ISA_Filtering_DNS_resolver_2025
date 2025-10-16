# @file Makefile
# @brief generates "dns" binary, executes tests
#
# @author Jaroslav Mervart
# @login xmervaj00
# @date 2025-10-4

# --- Compiler & flags ---
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -g
INCLUDES = -Iinclude

# make DEBUG=1
ifdef DEBUG
    CXXFLAGS += -DDEBUG_PRINT
endif

# --- Directories ---
SRC_DIR = src
OBJ_DIR = build

# --- Files ---
TARGET = dns
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# --- Rules ---
all: $(OBJ_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# --- Utility targets ---
run: $(TARGET)
	sudo ./$(TARGET) -s 8.8.8.8 -p 1053 -f testing/example_list.txt

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# --- Phony targets ---
.PHONY: all clean run
