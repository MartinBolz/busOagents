#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <streambuf>
#include <string>
#include <vector>

#include "../include/AgentNode.h"
#include "../include/CANBus.h"

namespace {

class NullBuffer : public std::streambuf {
protected:
    int_type overflow(int_type character) override {
        return traits_type::not_eof(character);
    }
};

class ScopedCoutSilencer {
public:
    ScopedCoutSilencer()
        : previousBuffer(std::cout.rdbuf(&nullBuffer)) {}

    ~ScopedCoutSilencer() {
        std::cout.rdbuf(previousBuffer);
    }

    ScopedCoutSilencer(const ScopedCoutSilencer&) = delete;
    ScopedCoutSilencer& operator=(const ScopedCoutSilencer&) = delete;

private:
    NullBuffer nullBuffer;
    std::streambuf* previousBuffer;
};

class CountingNode : public CANNode {
public:
    CountingNode(int id, const std::string& name)
        : CANNode(id, name) {}

    void receive_frame(const CANFrame&) override {
        ++receivedFrames;
    }

    std::size_t frameCount() const {
        return receivedFrames;
    }

private:
    std::size_t receivedFrames = 0;
};

} // namespace

int main() {
    constexpr std::size_t ticksPerMessage = 2;

    std::cerr
        << "\n========================================\n"
        << "STRESS TEST COMPLEXITY NOTICE\n"
        << "========================================\n"
        << "Let A = number of agents and P = messages per agent.\n"
        << "Total messages: A * P\n"
        << "The current bus scans every connected node during both\n"
        << "broadcast and processing, so runtime grows approximately as\n"
        << "O(A * total messages), or O(A^2 * P).\n"
        << "Memory usage grows approximately as O(A + total messages).\n"
        << "Large agent counts become expensive very quickly.\n"
        << "========================================\n";

    long long requestedAgentCount = 0;
    long long requestedMessagesPerAgent = 0;

    std::cerr << "Enter number of agents (minimum 2): ";
    if (!(std::cin >> requestedAgentCount)) {
        std::cerr << "Invalid agent count.\n";
        return 1;
    }

    std::cerr << "Enter messages per agent (minimum 1): ";
    if (!(std::cin >> requestedMessagesPerAgent)) {
        std::cerr << "Invalid message count.\n";
        return 1;
    }

    if (requestedAgentCount < 2 ||
        requestedAgentCount > std::numeric_limits<int>::max()) {
        std::cerr << "Agent count must be between 2 and "
                  << std::numeric_limits<int>::max() << ".\n";
        return 1;
    }

    if (requestedMessagesPerAgent < 1) {
        std::cerr << "Messages per agent must be at least 1.\n";
        return 1;
    }

    const int agentCount = static_cast<int>(requestedAgentCount);
    const std::size_t messagesPerAgent =
        static_cast<std::size_t>(requestedMessagesPerAgent);

    if (messagesPerAgent >
        std::numeric_limits<std::size_t>::max() /
            static_cast<std::size_t>(agentCount)) {
        std::cerr << "Requested workload is too large to represent safely.\n";
        return 1;
    }

    const std::size_t totalMessages =
        static_cast<std::size_t>(agentCount) * messagesPerAgent;

    if (totalMessages >
        std::numeric_limits<std::size_t>::max() / ticksPerMessage) {
        std::cerr << "Requested tick count is too large to represent safely.\n";
        return 1;
    }

    const long double estimatedNodeVisits =
        static_cast<long double>(ticksPerMessage) *
        (static_cast<long double>(agentCount) + 1.0L) *
        static_cast<long double>(totalMessages);

    std::cerr << "\nRequested workload:\n"
              << "  Agents:                 " << agentCount << '\n'
              << "  Messages per agent:     " << messagesPerAgent << '\n'
              << "  Total messages:         " << totalMessages << '\n'
              << "  Approximate node visits: " << estimatedNodeVisits << '\n'
              << "Continue? [y/N]: ";

    char confirmation = 'n';
    if (!(std::cin >> confirmation) ||
        (confirmation != 'y' && confirmation != 'Y')) {
        std::cerr << "Stress test cancelled.\n";
        return 0;
    }

    CANBus bus("StressTestBus");
    std::vector<std::unique_ptr<AgentNode>> agents;
    agents.reserve(agentCount);
    CountingNode frameCounter(-1, "FrameCounter");

    for (int id = 0; id < agentCount; ++id) {
        agents.push_back(
            std::make_unique<AgentNode>(id, "Agent" + std::to_string(id))
        );
        bus.connect_node(agents.back().get());
    }
    bus.connect_node(&frameCounter);

    // Use a fixed seed so every run sends the same workload. The offset starts
    // at one so a node never targets itself; the bus intentionally excludes a
    // frame's sender from its broadcast loop.
    std::mt19937 generator(27);
    std::uniform_int_distribution<int> targetOffset(1, agentCount - 1);

    std::chrono::steady_clock::time_point queueStart;
    std::chrono::steady_clock::time_point queueEnd;
    std::chrono::steady_clock::time_point processingEnd;

    {
        // Production classes currently print every frame. Silence stdout inside
        // the benchmark so direct execution remains readable. The final summary
        // is written to stderr after stdout is restored.
        ScopedCoutSilencer silenceOutput;

        queueStart = std::chrono::steady_clock::now();

        for (std::size_t message = 0; message < totalMessages; ++message) {
            int sender = static_cast<int>(
                message % static_cast<std::size_t>(agentCount)
            );
            int target =
                (sender + targetOffset(generator)) % agentCount;

            agents[sender]->new_task(
                target,
                "Stress workload message " + std::to_string(message)
            );
        }

        queueEnd = std::chrono::steady_clock::now();

        for (std::size_t tick = 0;
             tick < totalMessages * ticksPerMessage;
             ++tick) {
            bus.tick();
        }

        processingEnd = std::chrono::steady_clock::now();
    }

    std::chrono::duration<double, std::milli> queueTime =
        queueEnd - queueStart;
    std::chrono::duration<double, std::milli> processingTime =
        processingEnd - queueEnd;
    std::chrono::duration<double, std::milli> totalTime =
        processingEnd - queueStart;

    double messagesPerSecond =
        totalMessages / (totalTime.count() / 1000.0);
    bool busReturnedToIdle = bus.state() == BusState::Idle;
    bool allFramesObserved =
        frameCounter.frameCount() == totalMessages;
    bool testPassed = busReturnedToIdle && allFramesObserved;

    std::cerr << "\n========================================\n"
              << "STRESS TEST SUMMARY\n"
              << "========================================\n"
              << "Agents:              " << agentCount << '\n'
              << "Messages per agent:  " << messagesPerAgent << '\n'
              << "Messages submitted:  " << totalMessages << '\n'
              << "Frames observed:     " << frameCounter.frameCount() << '\n'
              << "Bus ticks:           "
              << totalMessages * ticksPerMessage << '\n'
              << "Queueing time:       " << queueTime.count() << " ms\n"
              << "Processing time:     " << processingTime.count() << " ms\n"
              << "Total time:          " << totalTime.count() << " ms\n"
              << "Throughput:          " << messagesPerSecond
              << " messages/second\n"
              << "Final bus state:     "
              << (busReturnedToIdle ? "Idle" : "Not Idle") << '\n'
              << "Result:              "
              << (testPassed ? "PASS" : "FAIL") << '\n'
              << "========================================\n";

    return testPassed ? 0 : 1;
}
