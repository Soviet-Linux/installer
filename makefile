CXX = g++

CXXFLAGS = -Wall -Wextra -pedantic -std=c++17

SRC_DIR = src
BUILD_DIR = build

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

TARGET = $(BUILD_DIR)/soviet-installer

all: $(TARGET)

$(TARGET): $(SRC_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_FILES) -o $(TARGET) -lncurses

run: $(TARGET)
	$(TARGET)

# Clean target
clean:
	rm -f $(TARGET)

.PHONY: all run clean
