CXX      := mpic++
CXXFLAGS := -std=c++20
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
BIN_DIR  := $(BUILD)/bin
TARGET   := latticeGas_FHP
SRC      := $(wildcard src/*/*.cpp)
SRC      += $(wildcard src/*.cpp)

OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: CXXFLAGS += -O3
all: build $(BIN_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$(TARGET) $^

.PHONY: all build clean debug

build:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g -O0
debug: TARGET = latticeGas_FHP_debug
debug: build $(BIN_DIR)/$(TARGET)


clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(BIN_DIR)/*
	-@rm -rvf $(BUILD)
