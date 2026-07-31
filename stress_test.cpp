#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include "CANBus.h"
#include "AgentNode.h"

int main() {
    CANBus bus("StressTestBus");
    std::vector<AgentNode*> agents;

    const int NUM_AGENTS = 5000;
    const int MESSAGES_PER_AGENT = 20;
    const int TOTAL_MESSAGES = NUM_AGENTS * MESSAGES_PER_AGENT;

    std::cerr << "Creating " << NUM_AGENTS << " agents..." << std::endl;
    for (int i = 0; i < NUM_AGENTS; ++i) {
        AgentNode* agent = new AgentNode(i, "Agent" + std::to_string(i));
        bus.connect_node(agent);
        agents.push_back(agent);
    }

    std::cerr << "Queuing " << TOTAL_MESSAGES << " tasks onto the bus..." << std::endl;
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_AGENTS; ++i) {
        for (int j = 0; j < MESSAGES_PER_AGENT; ++j) {
            int target = rand() % NUM_AGENTS; // pick a random agent
            agents[i]->new_task(target, "Stress test task " + std::to_string(j));
        }
    }

    std::cerr << "Simulating bus ticks to process all messages..." << std::endl;
    
    // In your implementation, 1 message takes 2 ticks (1 for broadcast, 1 for process_next_frame)
    // So we need TOTAL_MESSAGES * 2 ticks.
    long total_ticks = TOTAL_MESSAGES * 2;
    
    for (long i = 0; i < total_ticks; ++i) {
        bus.tick();
    }

    // Stop timing
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    std::cerr << "\n=========================================\n";
    std::cerr << "🚀 STRESS TEST COMPLETE\n";
    std::cerr << "Total Agents: " << NUM_AGENTS << "\n";
    std::cerr << "Total Messages: " << TOTAL_MESSAGES << "\n";
    std::cerr << "Time elapsed: " << elapsed.count() << " ms\n";
    std::cerr << "Messages per second: " << (TOTAL_MESSAGES / (elapsed.count() / 1000.0)) << "\n";
    std::cerr << "=========================================\n";

    // Clean up memory
    for (auto agent : agents) {
        delete agent;
    }

    return 0;
}
