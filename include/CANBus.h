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

class CANBus{
	public:
	CANBus(std::string name);
	void CANBusPrint();

	void connect_node(CANNode* node);

	void arbitrate(const CANFrame &frame);

	void process_transmission();


	bool is_bus_busy() const;

	const std::string& getBusName() const;

	void tick();


	private:

	//this needs to check if the bus is busy and if it is not then it needs to broadcast the frame to all connected nodes.
	void broadcast_frame(CANFrame frame);

	//all of the nodes connected to the CANBUS
	std::vector<CANNode *> connectedNodes;

	//arbitration is solved by the inverted < between MsgTypes in a Frame
	std::priority_queue<CANFrame> arbitration;
	
	CANFrame currentFrame;
	bool busBusy;	
	bool processingPhase = false;
	std::string BusName;
};

#endif