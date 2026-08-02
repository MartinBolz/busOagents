#ifndef LOGGERNODE_H
#define LOGGERNODE_H

#include "CANNode.h"
#include <vector>
#include <string>
#include <fstream>

/*
LoggerNode passively records every frame broadcast on the bus.

Unlike a normal CANNode, it does not reject frames based on target ID.
Frames are temporarily stored in memory and periodically appended to
a CSV file.
*/
class LoggerNode : public CANNode{
    public:
    //LogNode parameterized constructor
    LoggerNode(int id, const std::string& name);

    //Destructor flushes the message buffer to a csv named 
    //based on the bus that this instance connected to
    ~LoggerNode() override;

    //Overide recieve_frame so it ignores the targetID of the frame and just accepts every msg
    void receive_frame(const CANFrame& msg) override;

    //process frame does nothing we arent processing anything
    void process_frame(const CANFrame& frame) override;

    
    

    //This flushes the msgBuffer so we can get the messages out of ram and into a .csv
    void flush_2_csv();








    private:
    //this buffer vector stores all the intercepted messages and is flushed occasionally
    const size_t FLUSH_THREASHOLD = 1000;
    std::vector<CANFrame> msgBuffer;

    std::string logPath;
    
};

#endif