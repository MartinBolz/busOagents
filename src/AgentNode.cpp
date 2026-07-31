#include <string>
#include <vector>
#include <iostream>
#include <queue>

#include "../include/CANBus.h"
#include "../include/CANNode.h"
#include "../include/CANFrame.h"
#include "../include/AgentNode.h"

AgentNode::AgentNode(int id, const std::string& name): CANNode(id, name){} // Call the base class constructor

//sending new task to an a target on the bus
int AgentNode::new_task(int target, const std::string& prompt) {
    int taskID = nextTask();

    //remember that each CANNode gets assigned a unique ID when an AgentNode is created.
    CANFrame frame(target, CANFrame::MsgType::NewTask, CANNode::getID(), taskID, prompt);
    request_transmission(frame);
    return taskID;
}

//processing and changes internal states of the agent like TaskID and the prompt
//this updates the state of the agent and runs the next task
void AgentNode::handle_new_task(const CANFrame& frame){
    if (currentState == NodeState::Idle){
        currentState = NodeState::Working;

        currentTaskID = frame.task_id;
        currentPrompt = frame.prompt;

        std::cout << getName() 
        << " now exececuting " 
        << currentTaskID 
        << " : " 
        << currentPrompt << std::endl;
        return;
    }
    
    taskqueue.push(frame);

    std::cout << getName()
    << " queued task "
    << frame.task_id
    << std::endl;
}

//this resets the agent to its default
void AgentNode::resetAgent(){
    currentPrompt = "";
    currentTaskID = -1;
    currentState = NodeState::Idle;
}


//this sends stop_task to a target and sends it onto the bus.
void AgentNode::stop_task(int target, int taskID){
    CANFrame frame(target, CANFrame::MsgType::StopTask, CANNode::getID(), taskID, "");
    request_transmission(frame);

}


void AgentNode::handle_stop_task(const CANFrame& frame){
    
    currentState = NodeState::Stopping;

    std::cout << getName() 
    << " stopping exececution " 
    << currentTaskID << std::endl;

    resetAgent();

    runNextTask();
}

/*
        EmergencyStop = 0x001,
        StopTask = 0x010,
        ReplaceTask = 0x020,
        NewTask = 0x100,
        AgentResult = 0x200,
        ReviewRequest = 0x300,
        nullMsg = 0xFFF
*/

//this sends a message to a target on the bus to replace its current prompt
//replace task is only used to refine a prompt, not to pivot to a new task
void AgentNode::replace_task(int target, const std::string& prompt) {
    CANFrame frame(target, CANFrame::MsgType::ReplaceTask, CANNode::getID(), myTaskID(), prompt);
    request_transmission(frame);
}

void AgentNode::handle_replace_task(const CANFrame &frame){
    currentState = NodeState::Working;

    std::cout << getName() << " old prompt was: " << currentPrompt;

    currentPrompt = frame.prompt;

    std::cout << getName() 
    << ":: Prompt refined to;  " 
    << currentPrompt << std::endl;


}

int AgentNode::myTaskID(){
    return currentTaskID;
}

//this only increments the next_task_id and returns it used for sending messages and keeping track of stuff
int AgentNode::nextTask(){
    return next_task_id++;
}

void AgentNode::runNextTask(){
    if (currentState != NodeState::Idle){
        return;
    }

    if (taskqueue.empty()){
        return;
    }

    CANFrame nextFrame = taskqueue.front();
    taskqueue.pop();

    handle_new_task(nextFrame);
}

void AgentNode::completeCurrentTask() {
    std::cout << getName()
              << " completed task "
              << currentTaskID
              << std::endl;

    resetAgent();
    runNextTask();
}


void AgentNode::process_frame(const CANFrame& frame){
    if (frame.target_id != getID() && frame.target_id != 0){
        return;
    }

    switch (frame.identifier)
    {
        case CANFrame::MsgType::NewTask:
            handle_new_task(frame);
            break;
        
        case CANFrame::MsgType::StopTask:
            handle_stop_task(frame);
            break;
        
        case CANFrame::MsgType::ReplaceTask:
            handle_replace_task(frame);
            break;

        default:
            break;
    }
}
