#include "../include/LoggerNode.h"
#include <iostream>

//Logger node is just a CANnode with a few other features
//when its created it just reserves space on the heap for CANframes
LoggerNode::LoggerNode(int id, const std::string& name) : CANNode(id, name) {
    msgBuffer.reserve(FLUSH_THREASHOLD);
}


//Pushes every message to the msgBuffer
//this is overrided because recieve_frame is a CANNode function that rejects frames
//depending on the target
void LoggerNode::receive_frame(const CANFrame& msg){
    msgBuffer.push_back(msg);
}

//we dont need to process anything we just log 
void LoggerNode::process_frame(const CANFrame& frame){
    return;
}


void LoggerNode::flush_2_csv(){
    std::ofstream csv(log);

    if (!csv.is_open()){
        std::cerr << "Error: Could not open CSV for writing.\n";
    }
    //CANFrame(int target, MsgType msgType, int sender, int taskID, const std::string& data)
    csv << "Target,MsgType,Sender,TaskID,Prompt\n";

    for (const auto& frame : msgBuffer){
        csv << frame.target_id << ","
            << std::hex << static_cast<std::uint16_t>(frame.identifier) << std::dec << ","
            << frame.sender << ","
            << frame.task_id << ","
            << "\"" << frame.prompt << "\"\n";
    }

    csv.close();

    std::cout << "Logger: updated " << log << std::endl;


}

