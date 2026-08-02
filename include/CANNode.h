#ifndef CANNODE_H
#define CANNODE_H

#include <vector>
#include <string>
#include <queue>

#include "CANFrame.h"
//forward declaration of the CANBus class
class CANBus;

/*this is the CANNode class 
- represents a node on the bus
- creates and sends messages
- places the frame onto the bus
- monitors the bus during arbitration
- recieves frames from the bus
- decides if the frame is for this node or not

*/
class CANNode{
	friend class CANBus;



	public:
	//paramaterized constructor
	CANNode(const int id, const std::string& name);
	
	//this function prints the node info, including the node id, name, and the frames in the transmit and receive queues.
	void NodeInfo();



	//this function is called by the node to request a transmission of a frame onto the bus.
	void request_transmission(const CANFrame & frame);
	
	//this function is called by the bus to send a frame to this node, and it adds the frame to the receive queue.
	virtual void receive_frame(const CANFrame &frame);

	//process next frame tells the node to process the most important message from its inbox
	//process_frame is overriden for the AgentNode class to accept the message or reject it.
	void process_next_frame();

	//process frame is a virtual function for all of its children 
	// Generic CAN nodes do not interpret frames, they only handle sending them out to its children
	virtual void process_frame(const CANFrame &frame); 

	//this destructor is virtual so it allows CANNodes children to do specific cleanup when a node is deleted
	virtual ~CANNode() = default;


	//getters

	//returns the name of the node
    const std::string& getName() const;
    
	//returns the name of the bus its attached too
	const std::string& getBusName() const;
	
	//returns the node's unique id
	int getID() const;
	


	protected:
	//this creates a message frame and sends it onto the bus, if the bus is busy then the frame will be queued and sent when the bus is free.
	void create_frame(int target, CANFrame::MsgType msgType, int taskId, const std::string& data);
	
	private:

	//Connect bus is called only inside of CANBUS obj when the CANBUS::connect_node()node is called.
	void connectBus(CANBus* bus);

	int id;	
	CANBus* bus;
	std::string busName;
    std::string name;

	
	//this sorts and stores all of the messages from the bus regardless who sent it
	std::priority_queue<CANFrame> inbox;
};

#endif