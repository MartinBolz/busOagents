#include "../include/LoggerNode.h"
#include <iostream>
#include <filesystem>

//Logger node is just a CANnode with a few other features
//when its created it just reserves space on the heap for CANframes
LoggerNode::LoggerNode(int id, const std::string& name) : CANNode(id, name) {
    msgBuffer.reserve(FLUSH_THREASHOLD);
}

LoggerNode::~LoggerNode() {
    flush_2_csv();
}


//Pushes every message to the msgBuffer
//this is overrided because recieve_frame is a CANNode function that rejects frames
//depending on the target
void LoggerNode::receive_frame(const CANFrame& msg){
    msgBuffer.push_back(msg);

    if (msgBuffer.size() >= FLUSH_THREASHOLD){
        flush_2_csv();
    }
}

//we dont need to process anything we just log 
void LoggerNode::process_frame(const CANFrame& frame){
    return;
}


void LoggerNode::flush_2_csv(){
    if (msgBuffer.empty()){
        return;
    }

    if (logPath.empty()){
        logPath = getBusName() + "MessageLog.csv";
    }

    std::filesystem::path fullLogPath = std::filesystem::path("../logs") / (logPath);


    bool needsHeader =
    !std::filesystem::exists(fullLogPath) ||
    std::filesystem::file_size(fullLogPath) == 0;

    //we are always logging always and appending to the log
    std::ofstream csv(fullLogPath, std::ios::app);


    if (!csv.is_open()){
        std::cerr << "Error: Could not open CSV for writing.\n";
        return;
    }

    if (needsHeader){
        //CANFrame(int target, MsgType msgType, int sender, int taskID, const std::string& data)
        csv << "Target,MsgType,Sender,TaskID,Prompt\n";
    }

    for (const auto& frame : msgBuffer){
        csv << frame.target_id << ","
            << "0x"<< std::hex << static_cast<std::uint16_t>(frame.identifier) << std::dec << ","
            << frame.sender << ","
            << frame.task_id << ","
            << "\"" << frame.prompt << "\"\n";
        
    }

    csv.close();
    
    msgBuffer.clear();

    std::cout << "Logger: updated " << fullLogPath << " and cleared msgBuffer " << std::endl;


}

