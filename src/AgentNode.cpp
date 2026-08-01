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
    CANFrame frame(target, CANFrame::MsgType::NewTask, getID(), taskID, prompt);
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

//this resets the agent to its default along with keeping its taskQueue
void AgentNode::resetAgent(){
    currentPrompt = "";
    currentTaskID = -1;
    currentState = NodeState::Idle;

}

//this resets the agent to the default with empty taskQueue only called my emergency stop
void AgentNode::emegencyResetAgent(){
    resetAgent();
    while (!taskqueue.empty()){
        taskqueue.pop();
    }
    
}

//stop everything on the bus and reset agents
//the controller/human agent must reassign each tasks to the agents after this 
void AgentNode::emergencyStop(){
    //the target is zero meaning it is for all agents on the bus
    CANFrame frame(0, CANFrame::MsgType::EmergencyStop, getID(), -1, "");
    request_transmission(frame);
}

void AgentNode::handle_emergency_stop(const CANFrame& frame){
    //dont stop the task if its not my Id and is not an emergency stop target 
    if (frame.target_id != getID() && frame.target_id != 0){
        return;
    }
    
    currentState = NodeState::Stopping;

    std::cout << getName() 
    << " Emergency Stopped Exececution " 
    << currentTaskID << std::endl;

    emegencyResetAgent();

}

//this sends stop_task to a target and sends it onto the bus.
void AgentNode::stop_task(int target, int taskID){
    CANFrame frame(target, CANFrame::MsgType::StopTask, getID(), taskID, "");
    request_transmission(frame);

}


void AgentNode::handle_stop_task(const CANFrame& frame){
    //dont stop the task if its not for me
    if (frame.target_id != getID()){
        return;
    }
    
    //dont stop the task if I am idle
    if (currentState == NodeState::Idle){
        return;
    }

    //dont stop my current task matches if it doesn't match my taskID 
    if (frame.task_id != myTaskID()){
        return;
    }
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
//replace task is only used to refine a prompt, not to pivot to a new task so it will keep the same taskID
void AgentNode::replace_task(int target, const std::string& prompt) {
    CANFrame frame(target, CANFrame::MsgType::ReplaceTask, getID(), myTaskID(), prompt);
    request_transmission(frame);
}

void AgentNode::handle_replace_task(const CANFrame &frame){
    if (frame.target_id != getID()){
        return;
    }

    if (currentState != NodeState::Working) {
        return;
    }
    
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


//process frame rejects messages that are not designated for the AgentNode instance
void AgentNode::process_frame(const CANFrame& frame){
    if (frame.target_id != getID() && frame.target_id != 0){
        return;
    }

    logProcessedMsgQueue.push_back(frame);

    switch (frame.identifier)
    {
        case CANFrame::MsgType::EmergencyStop:
            handle_emergency_stop(frame);
            break;

        case CANFrame::MsgType::NewTask:
            handle_new_task(frame);
            break;
        
        case CANFrame::MsgType::StopTask:
            handle_stop_task(frame);
            break;
        
        case CANFrame::MsgType::ReplaceTask:
            handle_replace_task(frame);
            break;

        case CANFrame::MsgType::ReviewRequest:
            std::cout << "\t reviewing request not implemented yet: but will send the results of what the agent did researched/solved/planned ect... to another agent" << std::endl;
            break;
        
        case CANFrame::MsgType::AgentResult:
            std::cout << "\t Agent results not implemented yet: but it will return what the agent did researched/solved/planned ect..." << std::endl;
            break;
        
        case CANFrame::MsgType::nullMsg:
            //this should never ever happen will probably have to use an std::optional to replace a nullMSG
            break;

        default:
            break;
    }
}
