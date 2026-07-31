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
	void receive_frame(const CANFrame &frame);

	void process_next_frame();

	virtual void process_frame(const CANFrame &frame); 
	virtual ~CANNode() = default;


	//getters
    const std::string& getName() const;
    int getID() const;


	protected:
	//this creates a message frame and sends it onto the bus, if the bus is busy then the frame will be queued and sent when the bus is free.
	void create_frame(int target, CANFrame::MsgType msgType, int taskId, const std::string& data);
	
	private:
	

	void transmit_frame(const CANFrame& frame);

	//Connect bus is called only inside of CANBUS obj when the CANBUS::connect_node()node is called.
	void connectBus(CANBus* bus);

	int id;	
	CANBus* bus;
    std::string name;

	
	//this vector stores all of the messages from the bus regardless who sent it
	std::priority_queue<CANFrame> receive_msg_queue;

	//this vector stores all the messages from the bus that were processed
	std::vector<CANFrame> logProcessedMsgQueue;
	//this vector stores all the messages ever sent from this node
	std::vector<CANFrame> logTransmissions;
	
};

#endif