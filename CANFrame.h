#ifndef CANFRAME_H
#define CANFRAME_H

#include <string>

class CANFrame {
public:
    //These are the message types that can be sent on the bus. They are used to identify the type of message being sent.
    //these are given a unique identifier so that they can be used in the arbitration process. The lower the identifier the higher the priority.
    enum class MsgType : std::uint16_t{
        EmergencyStop = 0x001,
        StopTask = 0x010,
        ReplaceTask = 0x020,
        NewTask = 0x100,
        AgentResult = 0x200,
        ReviewRequest = 0x300,
        nullMsg = 0xFFF
    };


    //default constructor to make a frame with no data
    CANFrame() : target_id(-1), identifier(MsgType::nullMsg), sender(-1), task_id(-1), prompt("") {}

    //parameterized constructor
    CANFrame(int target, MsgType msgType, int sender, int taskID, const std::string& data)
        : target_id(target), identifier(msgType), sender(sender), task_id(taskID), prompt(data) {}
 
    //This operator is inverted because during arbitration we want the smallest value to be the highest priority
    //this is because we are going to be using a prority queue within CANbus messages
    bool operator< (const CANFrame& other) const{
        auto Msg = static_cast<std::uint16_t>(identifier); 
        auto otherMsg = static_cast<std::uint16_t>(other.identifier);
        return Msg > otherMsg;
    }


    int target_id; //the id of the node that this message is intended for, 0 means broadcast to all nodes
    MsgType identifier; //priority order used in arbitration, lower is higher priority msg
    int sender; //who sent the message?
    int task_id; //this gets incremented by the node/sender
    std::string prompt;
};

#endif