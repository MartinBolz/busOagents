#ifndef AGENTNODE_H
#define AGENTNODE_H

#include "CANNode.h"
#include <queue>

class AgentNode : public CANNode {
public:
    AgentNode(int id, const std::string& name);

    //tell a target to pivot to a new task, increment the taskID
    int new_task(
        int target,
        const std::string& prompt
    );

    //stops all agents on the bus and resets them, big red button stop everyting!
    //this should really be handled by a controller node, but I am defining a human/controller agent
    void emergencyStop();

    //tell a target to stop its task
    void stop_task(int target, int taskID);

    

    //tell a target to update a task's prompt, not to pivot to a different task.
    void replace_task(
        int target,
        const std::string& prompt
    );

    enum class NodeState{
	    Idle,
		Working,
		Stopping,
		Error,
		Completed
	};



private:
    void completeCurrentTask();

    //these handles are for reacting to a frame and to are dispatched by process frame!
    void handle_new_task(const CANFrame& frame);
    void handle_stop_task(const CANFrame& frame);
    void handle_replace_task(const CANFrame& frame);
    void handle_emergency_stop(const CANFrame& frame);


    //process frame dispatches the handles for the appropriate command sent across the CANbus
    //it rejects messages that are not designated for the AgentNode instance
    void process_frame(const CANFrame& frame) override;

    //returns the current task
    int myTaskID();
    
    //returns the nextTaskId by incrementing when a new task is called
    int nextTask();

    //this is ran after a stopping event or when a completed event happens
    void runNextTask();

    //this resets the agent to the default keeps its taskQueue
    void resetAgent();

    //this resets the agent to the default with empty taskQueue only called my emergency stop
    void emegencyResetAgent();

    //used for keeping state
    NodeState currentState = NodeState::Idle;

    //currentTaskID gets assigned when it receives a task
    int currentTaskID = -1;

    std::string currentPrompt = "";

    //this gets incremented when a new_task is called
    int next_task_id = 0;

    
    std::queue<CANFrame> taskqueue;

    //this vector stores all the messages from the bus that were processed by the Agent
	std::vector<CANFrame> logProcessedMsgQueue;

};

#endif