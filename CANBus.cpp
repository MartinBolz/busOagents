/*this is the CANBus class it represents the physical bus and the wires that make up the bus.
- this represents the shared network between all the nodes on the bus.
- knows which nodes are connected
- coordinates simultaneous transmission attempts
-determines the arbitratrion winner
- distributes the transmitted frame tpo every connected node

*/

#include "CANBus.h"
#include "CANNode.h"

#include <iostream>


CANBus::CANBus(std::string name) {
    busBusy = false;
    BusName = name;
}

void CANBus::connect_node(CANNode* node) {
    connectedNodes.push_back(node);
    node->connectBus(this);
}

void CANBus::broadcast_frame(CANFrame frame) {
    busBusy = true;
    currentFrame = frame;

    // Broadcast the frame to all connected nodes except the sender
    for (CANNode* node : connectedNodes) {
        if (node->getID() != frame.sender) {
            node->receive_frame(frame);
        }
    }

    std::cout << BusName
            << " broadcasting frame with identifier: "
            << std::hex
            << static_cast<std::uint16_t>(frame.identifier)
            << std::dec  
            << " from sender: "<< frame.sender 
            << ", to target: " << frame.target_id
            << ", task_id of: " << frame.task_id
            << std::endl;

    busBusy = false;
}

//this is how the system moves
void CANBus::tick(){
    //arbitrate the frame depending on the MsgType 
    //broadcast that frame to all the nodes that are connected to the bus
    //The nodes that need this message that share the same targetID of the Frame shares the ID of the node
    if (!processingPhase){
        //arbitrate then broadcast
        process_transmission();
        processingPhase = true;
    }
    //each node processes whats inside its message queue.
    //then all agent updates its status depending on the message that was sent. 
    else{
        //nodes all process one queued frame
        for (CANNode* node : connectedNodes){
            node -> process_next_frame();
        }
        processingPhase = false;
    }
}

void CANBus::process_transmission(){

    if (arbitration.empty()){
        return;
    }

    CANFrame mostImportantMessage = arbitration.top();
    arbitration.pop();

    broadcast_frame(mostImportantMessage);
}

void CANBus::arbitrate(const CANFrame &frame){
    arbitration.push(frame);
}


bool CANBus::is_bus_busy() const{
    return busBusy;
}

const std::string& CANBus::getBusName() const {
    return BusName;
}

void CANBus::CANBusPrint(){
    std::cout << std::endl << ":::CANBUS PRINTING::::::: " << std::endl;
    std::cout << "Bus Name: " << BusName << std::endl;
    std::cout << "Bus Busy: " << (busBusy ? "Yes" : "No") << std::endl;
    std::cout << "Connected Nodes: " << std::endl;
    for (const auto& node : connectedNodes) {
        std::cout << "\t name: " <<  node->getName() << "  id: " << node->getID() << std::endl;
    }
    std::cout << std::endl;
}