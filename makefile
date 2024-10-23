CXX = g++
CXXFLAGS = -Iinclude -Wall -Wextra -std=c++17
LDFLAGS = -lncurses
SRC_DIR = src
BUILD_DIR = build
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
BINARY = $(BUILD_DIR)/soviet-installer

all: $(BINARY)
$(BINARY): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
run: $(BINARY)
	./$(BINARY)
clean:
	rm -rf $(BUILD_DIR)/*.o $(BINARY)

.PHONY: all clean run
