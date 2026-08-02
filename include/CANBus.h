#ifndef CANBUS_H
#define CANBUS_H

#include <string>
#include <vector>
#include <queue>

#include "CANFrame.h"

/*this is the CANBus class it represents the physical bus and the wires that make up the bus.
- this represents the shared network between all the nodes on the bus.
- knows which nodes are connected
- coordinates simultaneous transmission attempts
-determines the arbitratrion winner
- distributes the transmitted frame tpo every connected node

*/

class CANNode;

enum class BusState {
    Idle,
    Transmitting,
    NodesProcessing
};

class CANBus{
	public:
	//creates a bus with a name given
	CANBus(std::string name);
	
	//prints the bus with the provided display name
	void CANBusPrint();

	//adds a node to the network to and gives the node arefernce to this bus
	void connect_node(CANNode* node);

	//places a frame into arbitration priority queue changes the state of the bus
	void arbitrate(const CANFrame &frame);

	//removes the highest priority frame and broadcasts to all connected nodes
	void transmit();

	//Allows every connected node to process one queued message
	void nodesProcessMsg();

	//returns the current state of the bus
	BusState state() const;

	//returns the name of the bus
	const std::string& getBusName() const;

	//advances the simulation by one bus phase
	void tick();


	private:


	//this needs to check if the bus is busy and if it is not then it needs to broadcast the frame to all connected nodes.
	void broadcast_frame(const CANFrame &frame);

	std::string printState() const;

	//all of the nodes connected to the CANBUS
	std::vector<CANNode *> connectedNodes;

	//arbitration is solved by the inverted < between MsgTypes in a Frame
	std::priority_queue<CANFrame> arbitration;
	
	BusState myState = BusState::Idle;
	std::string BusName;
};

#endif