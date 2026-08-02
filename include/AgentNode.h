#ifndef AGENTNODE_H
#define AGENTNODE_H

#include "CANNode.h"
#include <queue>

class AgentNode : public CANNode {
public:
    AgentNode(int id, const std::string& name);

    // Creates and submits a NewTask frame for another agent.
    // target: ID of the agent receiving the task.
    // prompt: description of the work to perform.
    // Returns the task ID assigned by this sender.
    int new_task(
        int target,
        const std::string& prompt
    );

    // Broadcasts an emergency-stop frame to every agent on the bus.
    // Receiving agents stop their current task and clear queued tasks.
    void emergencyStop();

    // Requests that a target agent stop a particular task.
    // The target ignores the request if taskID is not its current task.
    void stop_task(int target, int taskID);

    

    //tell a target to update a task's prompt, not to pivot to a different task.
    // Updates the prompt of the target agent's active task without
    // assigning a new task ID.
    void replace_task(
        int target,
        const std::string& prompt
    );

    enum class NodeState{
	    Idle, //No active task
		Working, //Currently Working on a task
		Stopping, //Transition away from a task
		Error, //error orrcured
		Completed //current task completed waiting next task
	};



private:
    //this is not implemented but it will change the nodes state
    //probably also will reset the agent
    //and send a frame to the assigner of the task with a report message
    void completeCurrentTask();

    //these handles are for reacting to a frame and to are dispatched by process frame!
    void handle_new_task(const CANFrame& frame);
    void handle_stop_task(const CANFrame& frame);
    void handle_replace_task(const CANFrame& frame);
    void handle_emergency_stop(const CANFrame& frame);


    //process frame dispatches the handles for the appropriate command sent across the CANbus
    //it rejects messages that are not designated for the AgentNode instance
    void process_frame(const CANFrame& frame) override;

    //returns the current taskID
    int myTaskID();
    
    //returns the nextTaskId by incrementing when a new task is called
    int nextTask();

    //not implemented yet
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