#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "tbb/atomic.h"
#include "tbb/tick_count.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/task_group.h"

using namespace tbb;


int getRandom() {
    std::srand(std::time(nullptr));
    return std::rand();
}


std::vector<std::queue<int> > gQueues;
atomic<int> trigger;


class Hello {
    std::string operation;
    const int kSecondsLimit;
public:
    Hello(
        std::string operation, int indexChannel, int content = 0
    ): kSecondsLimit(5) {
      if (operation == "push") {
        if (!isValidIndex(indexChannel)) {
          std::cout << "Not a valid index.\n";
          return;
        }
        gQueues[indexChannel].push(content);
      }
      if (operation == "pop") {
        if (!isValidIndex(indexChannel)) {
          std::cout << "Not a valid index.\n";
          return;
        }
        if (gQueues[indexChannel].size() < 1) {
          std::cout << "Can't pop on empty channel " << indexChannel << ".\n";
        }
        gQueues[indexChannel].pop();
        // Trigger the behavior that a pop was encountered
        trigger.fetch_and_add(indexChannel);
      }
      if (operation != "push" && operation != "pop") {
        std::cout << "Not a valid operation.\n";
        return;
      }
      this->operation = operation;
    }

    bool isValidIndex(int indexChannel) {
      return (indexChannel >= 0 && indexChannel < gQueues.size());
    }

    void operator()() const {
       std::this_thread::sleep_for(
           std::chrono::seconds(getRandom() % kSecondsLimit)
       );
       std::cout << operation << "\n";
    }
};


void runExampleWithTaskGroup() {
    std::string operation;
    int x;
    int indexChannel;

    trigger.fetch_and_add(0);

    gQueues.resize(20); // TODO change this
    task_group tg;

    // queue indices starting with 1, to have effect the fetch_and_add(number)
    tg.run(Hello("push", 1, 5));
    tg.run(Hello("push", 1, 8));
    tg.run(Hello("push", 2, 10));
    // This pop is hardcoded for the example, but it actually triggers
    // when the first pop is encountered
    tg.run(Hello("pop", 1));

    /*
     * `select` are.
     * We consider the default case of waiting 10 seconds, when this timer
     * is reached, we cancel the entire task group.
     */
    while (true) {
      if (trigger != 0) {
        std::cout << "Received item from channel " << trigger << ".\n";
        break;
      }
    }

    tg.wait();
}


int main() {
    runExampleWithTaskGroup();

    return 0;
}
