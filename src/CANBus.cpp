/*this is the CANBus class it represents the physical bus and the wires that make up the bus.
- this represents the shared network between all the nodes on the bus.
- knows which nodes are connected
- coordinates simultaneous transmission attempts
-determines the arbitratrion winner
- distributes the transmitted frame tpo every connected node

*/

#include "../include/CANBus.h"
#include "../include/CANNode.h"

#include <iostream>


CANBus::CANBus(std::string name) {
    myState = BusState::Idle;
    BusName = name;
    
}

void CANBus::connect_node(CANNode* node) {
    connectedNodes.push_back(node);
    node->connectBus(this);
}


//broadcast that frame to all the nodes that are connected to the bus
//The nodes that need this message that share the same targetID of the Frame shares the ID of the node
void CANBus::broadcast_frame(const CANFrame& frame) {
    myState = BusState::Transmitting;


    // Broadcast the frame to all connected nodes except the sender
    for (CANNode* node : connectedNodes) {
        if (node->getID() != frame.sender) {
            node->receive_frame(frame);
        }
    }

    std::cout << BusName
            << printState() << " frame with identifier: "
            << std::hex << static_cast<std::uint16_t>(frame.identifier) << std::dec  
            << " from sender: "<< frame.sender 
            << ", to target: " << frame.target_id
            << ", task_id of: " << frame.task_id
            << std::endl;
}

//this is how the system moves
void CANBus::tick(){
    switch(myState){

        //if the bus is idle start transmitting 
        case BusState::Idle: 
            //if there are no messages return
            if (arbitration.empty()){
                return;
            }
            myState = BusState::Transmitting;
            transmit();
            return;

        //each node processes whats inside its message queue.
        //then all agent updates its status depending on the message that was sent. 
        case BusState::NodesProcessing:
            //nodes all process one queued frame
            nodesProcessMsg();
            myState = BusState::Idle;
            return;

        //tramsmission state happens linearly: bus idle -> bus trannsmit msg then Nodes Processing ASAP -> Bus return to Idle
        case BusState::Transmitting:
            return;
    }
}

void CANBus::transmit(){
    //pull most important message from msg priority queue and remove it
    broadcast_frame(arbitration.top());
    arbitration.pop();
    
    //updating the state to NodesProcessing on next tick
    myState = BusState::NodesProcessing;
}

//all the nodes start processing the next message in their inbox based on the priority queue
void CANBus::nodesProcessMsg(){
    for (CANNode* node : connectedNodes){
        node -> process_next_frame();
    }
}


//arbitrate the frame depending on the MsgType this is because arbitration is a priority queue.
void CANBus::arbitrate(const CANFrame &frame){
    std::cout << BusName 
    << " arbitrated frame " 
    << std::hex << static_cast<uint16_t>(frame.identifier) << std::dec 
    << " from " << frame.sender << std::endl;

    arbitration.push(frame);
}

BusState CANBus::state() const{
    return myState;
}


const std::string& CANBus::getBusName() const {
    return BusName;
}


std::string CANBus::printState() const {
    switch (myState) {
        case BusState::Idle:
            return "Idle";
        case BusState::Transmitting:
            return "Transmitting";
        case BusState::NodesProcessing:
            return "NodesProcessing";
    }

    return "Unknown";
}

void CANBus::CANBusPrint(){
    std::cout << std::endl << ":::CANBUS PRINTING::::::: " << std::endl;
    std::cout << "Bus Name: " << BusName << std::endl;
    std::cout << "Bus state: " << printState() << std::endl;
    std::cout << "Connected Nodes: " << std::endl;
    for (const auto& node : connectedNodes) {
        std::cout << "\t name: " <<  node->getName() << "  id: " << node->getID() << std::endl;
    }
    std::cout << std::endl;
}