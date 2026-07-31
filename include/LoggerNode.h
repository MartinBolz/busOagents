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

    //Overide recieve_frame so it ignores the targetID of the frame and just accepts every msg
    void receive_frame(const CANFrame& msg);

    //Overides process_frame to pull out the data
    void process_frame(const CANFrame& frame) override;

    //This flushes the msgBuffer so we can get the messages out of ram and into a .csv
    void flush_2_csv();








    private:
    //this buffer vector stores all the intercepted messages and is flushed occasionally
    const size_t FLUSH_THREASHOLD = 1000;
    std::vector<CANFrame> msgBuffer;

    std::string log = "messageLog.csv";
};

#endif