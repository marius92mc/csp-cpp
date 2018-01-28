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
      if (operation != "push" && operation != "pop") {
        std::cout << "Not a valid operation.\n";
        return;
      }

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
          return;
        }
        gQueues[indexChannel].pop();
        // Trigger the behavior that a pop was encountered
        trigger.fetch_and_add(indexChannel);
      }

      this->operation = operation;
    }

    void operator()() const {
       std::this_thread::sleep_for(
           std::chrono::seconds(getRandom() % kSecondsLimit)
       );
       std::cout << operation << "\n";
    }

private:
    bool isValidIndex(const int& indexChannel) {
      return (indexChannel >= 0 && indexChannel < gQueues.size());
    }
};


class DefaultTimer {
  int countSeconds;
public:
  DefaultTimer(int seconds) {
    countSeconds = seconds;
  }

  void operator()() const {
    std::this_thread::sleep_for(std::chrono::seconds(countSeconds));
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
    // when the first pop is encountered.
    tg.run(Hello("pop", 1));
    //tg.run(Hello("pop", 3));

    tg.run(DefaultTimer(5));

    /*
     * `select` are.
     * We consider the default case of waiting 10 seconds, when this timer
     * is reached, we cancel the entire task group.
     */
    while (true) {
      if (trigger != 0) {
        // A pop from a queue was encountered, aka
        // an item from a channel was received,
        // more precisely from the channel with index `trigger`,
        // because of the `trigger.fetch_and_add(indexChannel)` call.
        std::cout << "Received item from channel " << trigger << ".\n";

        // When an item is received, cancel all the other channels.
        tg.cancel();
        break;
      }

      // Handling the `default` case of `select`.
      // When it finished all the tasks, including the costliest one
      // represented by the `DefaultTimer()`, we inspect the returned value,
      // which is from an enum, and the value `1` corresponse to
      // `complete, // Not cancelled and all tasks in group have completed`,
      // in which case it means that we have reached the default case, so
      // we print the message on console and exit from loop.
      // `task_group` - https://software.intel.com/en-us/node/506287
      // `enum task_group_status`, as returned value of `wait()` -
      // 		https://software.intel.com/en-us/node/506289.
      // enum task_group_status {
      //     not_complete, // Not cancelled and not all tasks in group have completed.
      //     complete,     // Not cancelled and all tasks in group have completed
      //     canceled      // Task group received cancellation request
      // };
      if (tg.wait() == 1) {
        std::cout << "Default case reached.\n";
        break;
      }
    }

    tg.wait();
}


int main() {
    runExampleWithTaskGroup();

    return 0;
}
