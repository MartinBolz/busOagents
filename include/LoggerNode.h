#ifndef LOGGERNODE_H
#define LOGGERNODE_H

#include "CANNode.h"
#include <vector>
#include <string>
#include <fstream>

class LoggerNode : public CANNode{
    public:
    //LogNode parameterized constructor
    LoggerNode(int id, const std::string& name);

    //we have to overide the destructor to flush at the end of runtime because its
    //likely we wont be at the FLUSH_THREASHOLD
    ~LoggerNode() override;

    //Overide recieve_frame so it ignores the targetID of the frame and just accepts every msg
    void receive_frame(const CANFrame& msg) override;

    //Overides process_frame to pull out the data
    void process_frame(const CANFrame& frame) override;

    
    

    //We flush right before the threshold? This flushes the msgBuffer so we can get the messages out of ram and into a .csv
    void flush_2_csv();








    private:
    //this buffer vector stores all the intercepted messages and is flushed occasionally
    const size_t FLUSH_THREASHOLD = 1000;
    std::vector<CANFrame> msgBuffer;

    std::string logPath;
    
};

#endif