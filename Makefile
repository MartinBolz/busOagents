CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

TARGET = orchestration

SOURCES = main.cpp CANNode.cpp CANBus.cpp AgentNode.cpp
OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.cpp AgentNode.h CANNode.h CANBus.h CANFrame.h 
	$(CXX) $(CXXFLAGS) -c main.cpp

CANNode.o: CANNode.cpp CANNode.h CANBus.h CANFrame.h
	$(CXX) $(CXXFLAGS) -c CANNode.cpp

CANBus.o: CANBus.cpp CANBus.h CANNode.h CANFrame.h
	$(CXX) $(CXXFLAGS) -c CANBus.cpp

AgentNode.o: AgentNode.cpp AgentNode.h CANNode.h CANBus.h CANFrame.h
	$(CXX) $(CXXFLAGS) -c AgentNode.cpp

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)