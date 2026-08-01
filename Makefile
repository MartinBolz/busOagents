CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude
DEPFLAGS = -MMD -MP

TARGET = orchestration
STRESS_TARGET = stress_test

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TESTS_DIR = tests

# Sources
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/CANNode.cpp \
          $(SRC_DIR)/CANBus.cpp \
          $(SRC_DIR)/AgentNode.cpp \
          $(SRC_DIR)/LoggerNode.cpp

STRESS_SOURCES = $(TESTS_DIR)/stress_test.cpp \
                 $(SRC_DIR)/CANNode.cpp \
                 $(SRC_DIR)/CANBus.cpp \
                 $(SRC_DIR)/AgentNode.cpp \
                 $(SRC_DIR)/LoggerNode.cpp
# Objects
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
STRESS_OBJECTS = $(patsubst $(TESTS_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter $(TESTS_DIR)/%.cpp,$(STRESS_SOURCES))) \
                 $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(STRESS_SOURCES)))

# Compiler-generated header dependency files. These ensure that changing a
# header rebuilds every object file that includes it.
DEPS = $(sort $(OBJECTS:.o=.d) $(STRESS_OBJECTS:.o=.d))

.PHONY: all directories clean run run_stress

all: directories $(TARGET) $(STRESS_TARGET)

directories:
	@mkdir -p $(BUILD_DIR) logs

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

$(STRESS_TARGET): directories $(STRESS_OBJECTS)
	$(CXX) $(CXXFLAGS) $(STRESS_OBJECTS) -o $(STRESS_TARGET)

# Compile src/ files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | directories
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Compile tests/ files
$(BUILD_DIR)/%.o: $(TESTS_DIR)/%.cpp | directories
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(BUILD_DIR)/*.csv \
	      $(BUILD_DIR)/orchestration $(BUILD_DIR)/stress_test \
	      $(TARGET) $(STRESS_TARGET)

run: $(TARGET)
	./$(TARGET)

run_stress: $(STRESS_TARGET)
	./$(STRESS_TARGET) > /dev/null

-include $(DEPS)
