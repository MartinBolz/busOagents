#include "CANNode.h"
#include "CANBus.h"

#include <iostream>


CANNode::CANNode(const int id, const std::string& name){
    this->id = id;
    this->name = name;
    bus = nullptr;
}

void CANNode::NodeInfo(){
    std::cout << std::endl << ":::CANNODE-" << name << "-INFO::::::: " << std::endl;
    std::cout << "Node ID: " << id << std::endl;
    std::cout << "Node Name: " << name << std::endl;
    if (bus) {
        std::cout << "Connected to Bus: " << bus->getBusName() << std::endl;
    } else {
        std::cout << "Not connected to any bus." << std::endl;
    }

    for (auto& frame : logTransmissions) {
        std::cout << "Transmit Frame - Identifier: "         
                << std::hex
                << static_cast<std::uint16_t>(frame.identifier)
                << std::dec  
                << ", target: " << frame.target_id
                << ", task_id: " << frame.task_id
                << ", Sender: " << frame.sender 
                << ", prompt " << frame.prompt << std::endl;
    }

    auto temp_queue = receive_msg_queue;

    while (!temp_queue.empty()) {
        const auto& frame = temp_queue.top();
        
        std::cout << "Receive Queue Frame - Identifier: 0x" 
                << std::hex << static_cast<std::uint16_t>(frame.identifier) << std::dec 
                << ", target: " << frame.target_id << std::endl;

        temp_queue.pop();
    }

    for (auto& frame : logProcessedMsgQueue) {
        std::cout << "logProcessedMsgQueue Frame - Identifier: "   
                << std::hex
                << static_cast<std::uint16_t>(frame.identifier)
                << std::dec                  
                << ", target: " << frame.target_id
                << ", task_id: " << frame.task_id
                << ", Sender: " << frame.sender 
                << ", prompt " << frame.prompt << std::endl;
    }

    std::cout << "--------------------" << std::endl;
}


//this function creates a frame, runs request transmission, and runs transmit frame
//it is called by the node to create a frame and send it onto the bus.
//if the bus is busy then the frame will be queued and sent when the bus is free.
void CANNode::create_frame(int target, CANFrame::MsgType msgType, int taskId, const std::string& data){
    CANFrame frame(target, msgType, getID(), taskId, data);
    request_transmission(frame);
}

//this function is called by the node to request a transmission of a frame onto the bus.
void CANNode::request_transmission(const CANFrame& frame){
    if (bus == nullptr) {
        std::cout << name << " is not connected to any bus. Cannot transmit frame." << std::endl;
        return;
    }

    //arbitrate the frame and log its presence in general
    bus->arbitrate(frame);
    logTransmissions.push_back(frame);

    std::cout << name
        << " submitted frame 0x"
        << std::hex
        << static_cast<std::uint16_t>(frame.identifier)
        << std::dec
        << " for arbitration"
        << std::endl;
}





void CANNode::receive_frame(const CANFrame &frame){
    // Simulate a hardware filter: Ignore frames not meant for us (0 is broadcast)
    if (frame.target_id != id && frame.target_id != 0) {
        return; 
    }

    receive_msg_queue.push(frame);

    std::cout << name << " received frame with identifier: " 
        << std::hex
        << static_cast<std::uint16_t>(frame.identifier)
        << std::dec  
        << " with id task_id of: " << frame.task_id 
        << " and from sender: "<< frame.sender << std::endl;
    logProcessedMsgQueue.push_back(frame);
}

void CANNode::process_next_frame(){
    if (receive_msg_queue.empty()){
        return;
    }

    CANFrame frame = receive_msg_queue.top();
    receive_msg_queue.pop();

    process_frame(frame);
    logProcessedMsgQueue.push_back(frame);
}

void CANNode::process_frame(const CANFrame& frame) {
    // Generic CAN nodes do not interpret agent commands.
}


//Connect bus is called only inside of CANBUS obj when the CANBUS::connect_node()node is called.
void CANNode::connectBus(CANBus* newBus){
    this->bus = newBus;
}

const std::string& CANNode::getName() const {
    return name;
}

int CANNode::getID() const {
    return id;
}
