#include <iostream>
#include <string> 
#include <vector>
#include "../include/CANBus.h"
#include "../include/CANNode.h"
#include "../include/CANFrame.h"
#include "../include/AgentNode.h"

int main() {
    CANBus bus("communication");
    std::vector<CANNode*> nodes;

	/*dynamically adding new nodes to the bus
    for (int i = 0; i < 10; i++) {
        CANNode* node =
            new CANNode(i, "Node" + std::to_string(i));

        nodes.push_back(node);
        myFirstBus.connect_node(node);
    }
	*/

	AgentNode* agent1 = new AgentNode(1, "agent1");
	bus.connect_node(agent1);

	AgentNode* agent2 = new AgentNode(2, "agent2");
	bus.connect_node(agent2);

    bus.CANBusPrint();

	agent1->new_task(2, "I want you to say something nice to me");

    bus.tick();

	agent2->NodeInfo();

	agent1->NodeInfo();	

    bus.tick();

    agent1->replace_task(2, "I want you to say something nice about my hair");

    





    return 0;
}
