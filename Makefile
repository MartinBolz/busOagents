CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude

TARGET = build/orchestration
STRESS_TARGET = build/stress_test

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TESTS_DIR = tests

# Sources
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/CANNode.cpp $(SRC_DIR)/CANBus.cpp $(SRC_DIR)/AgentNode.cpp
STRESS_SOURCES = $(TESTS_DIR)/stress_test.cpp $(SRC_DIR)/CANNode.cpp $(SRC_DIR)/CANBus.cpp $(SRC_DIR)/AgentNode.cpp

# Objects
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
STRESS_OBJECTS = $(patsubst $(TESTS_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter $(TESTS_DIR)/%.cpp,$(STRESS_SOURCES))) \
                 $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(STRESS_SOURCES)))

all: directories $(TARGET)

orchestration: $(TARGET)
stress_test: $(STRESS_TARGET)

directories:
	@mkdir -p $(BUILD_DIR) logs

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

$(STRESS_TARGET): directories $(STRESS_OBJECTS)
	$(CXX) $(CXXFLAGS) $(STRESS_OBJECTS) -o $(STRESS_TARGET)

# Compile src/ files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile tests/ files
$(BUILD_DIR)/%.o: $(TESTS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET) $(STRESS_TARGET)

run: $(TARGET)
	./$(TARGET)

run_stress: $(STRESS_TARGET)
	./$(STRESS_TARGET) > /dev/null