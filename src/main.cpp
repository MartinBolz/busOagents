#include <iostream>
#include <chrono>
#include <string>

#include "../include/CANBus.h"
#include "../include/AgentNode.h"
#include "../include/LoggerNode.h"

void run_bus_cycle(CANBus& bus) {
    // Tick 1: arbitrate and broadcast one frame.
    bus.tick();

    // Tick 2: nodes process that frame.
    bus.tick();
}

void print_test(const std::string& testName) {
    std::cout << "\n========================================\n";
    std::cout << testName << '\n';
    std::cout << "========================================\n";
}

int main() {
    CANBus bus("MainBus");

    AgentNode agent1(1, "Agent One");
    AgentNode agent2(2, "Agent Two");
    AgentNode human(3, "Human User");
    LoggerNode logger(100, "Logger");

    // Every node that transmits or receives must be connected.
    bus.connect_node(&agent1);
    bus.connect_node(&agent2);
    bus.connect_node(&human);
    bus.connect_node(&logger);

    print_test("DEFAULT NETWORK");
    bus.CANBusPrint();

    // ---------------------------------------------------------
    // TEST 1:
    // Human assigns a new task to Agent One.
    // ---------------------------------------------------------

    print_test("TEST 1: HUMAN SENDS NEW TASK TO AGENT ONE");

    int agent1TaskID = human.new_task(
        agent1.getID(),
        "Tell me something interesting about beer."
    );

    run_bus_cycle(bus);

    // Expected:
    // Agent One stores and begins the task.
    // Agent Two ignores the frame.
    // Human ignores the frame because it is the sender.
    // Logger records the frame.

    // ---------------------------------------------------------
    // TEST 2:
    // Agent One delegates work to Agent Two.
    // ---------------------------------------------------------

    print_test("TEST 2: AGENT ONE DELEGATES TO AGENT TWO");

    int agent2TaskID = agent1.new_task(
        agent2.getID(),
        "Explain how beer fermentation works."
    );

    run_bus_cycle(bus);

    // Expected:
    // Agent Two stores and begins the task.
    // Human ignores it.
    // Logger records it.

    // ---------------------------------------------------------
    // TEST 3:
    // Human changes Agent One's prompt without creating
    // a different task.
    // ---------------------------------------------------------

    print_test("TEST 3: HUMAN REPLACES AGENT ONE TASK PROMPT");

    human.replace_task(
        agent1.getID(),
        "Focus specifically on the history of beer brewing."
    );

    run_bus_cycle(bus);

    // Expected:
    // Agent One keeps the same task ID.
    // Agent One updates its current prompt.
    // Agent Two ignores the frame.
    // Logger records it.

    // ---------------------------------------------------------
    // TEST 4:
    // Agent One replaces Agent Two's prompt.
    // ---------------------------------------------------------

    print_test("TEST 4: AGENT ONE REPLACES AGENT TWO TASK PROMPT");

    agent1.replace_task(
        agent2.getID(),
        "Give a short explanation of yeast's role in fermentation."
    );

    run_bus_cycle(bus);

    // Expected:
    // Agent Two updates its current prompt.
    // Agent Two keeps agent2TaskID.
    // Logger records the frame.

    // ---------------------------------------------------------
    // TEST 5:
    // Human stops Agent Two's current task.
    // ---------------------------------------------------------

    print_test("TEST 5: HUMAN STOPS AGENT TWO TASK");

    human.stop_task(
        agent2.getID(),
        agent2TaskID
    );

    run_bus_cycle(bus);

    // Expected:
    // Agent Two enters Stopping or Idle depending on your logic.
    // Agent One ignores it.
    // Logger records it.

    // ---------------------------------------------------------
    // TEST 6:
    // Attempt to stop the wrong task ID.
    // ---------------------------------------------------------

    print_test("TEST 6: STOP REQUEST WITH WRONG TASK ID");

    human.stop_task(
        agent1.getID(),
        9999
    );

    run_bus_cycle(bus);

    // Expected:
    // Agent One rejects or ignores the request because task 9999
    // is not its current task.
    // Logger still records the attempted command.

    // ---------------------------------------------------------
    // TEST 7:
    // Human stops Agent One's correct task.
    // ---------------------------------------------------------

    print_test("TEST 7: HUMAN STOPS AGENT ONE CORRECTLY");

    human.stop_task(
        agent1.getID(),
        agent1TaskID
    );

    run_bus_cycle(bus);

    // Expected:
    // Agent One stops agent1TaskID.
    // Logger records the frame.

    // ---------------------------------------------------------
    // TEST 8:
    // Both agents receive new tasks before the bus ticks.
    // This demonstrates arbitration.
    // ---------------------------------------------------------

    print_test("TEST 8: MULTIPLE FRAMES WAITING FOR ARBITRATION");

    human.new_task(
        agent1.getID(),
        "Describe the ingredients used to make beer."
    );

    human.new_task(
        agent2.getID(),
        "Describe the difference between ale and lager."
    );

    // There are two queued transmissions, so each one needs
    // its own transmit/process cycle.
    run_bus_cycle(bus);
    run_bus_cycle(bus);

    // Expected:
    // Frames are broadcast according to MsgType priority.
    // Each agent stores only its own task.
    // Logger records both frames.

    
    // ---------------------------------------------------------
    // FINAL STATE
    // ---------------------------------------------------------

    print_test("FINAL BUS STATE");
    bus.CANBusPrint();

    print_test("FINAL AGENT INFORMATION");

    std::cout << "\nAgent One:\n";
    agent1.NodeInfo();

    std::cout << "\nAgent Two:\n";
    agent2.NodeInfo();

    std::cout << "\nHuman User:\n";
    human.NodeInfo();

    std::cout << "\nLogger:\n";
    logger.NodeInfo();

    std::cout << "\nAll demo tests completed.\n";

    return 0;
}
