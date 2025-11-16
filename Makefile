# @file Makefile
# @brief generates "dns" binary, executes tests
#
# @author Jaroslav Mervart
# @login xmervaj00
# @date 2025-10-4

# Compiler & flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -Wshadow -pedantic -g -O0
INCLUDES = -Iinclude

# Conditional debug flag
ifeq ($(BUILD_DEBUG),1)
    CXXFLAGS += -DDEBUG_PRINT
endif

# Directories
SRC_DIR = src
OBJ_DIR = build

# Files
TARGET = dns
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Rules
all: $(OBJ_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Utility targets
test: $(TARGET)
	./testing/test.sh

debug:
	$(MAKE) clean
	$(MAKE) BUILD_DEBUG=1

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean debug test