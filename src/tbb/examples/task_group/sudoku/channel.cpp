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


std::vector<std::queue<int> > gQueues;
const unsigned int gKDefaultnumberOfChannels = 20;

/*
 * Either all threads see the updated value of count at once,
 * or all threads continue to see the old value.
 * Operations on count are atomic and cannot be interrupted by the
 * vagaries of process or thread scheduling.
 * No matter how threads are scheduled,
 * there's no way count would have different values in different threads.
 */
atomic<int> trigger;


int getRandom() {
    std::srand(std::time(nullptr));
    return std::rand();
}


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
      if (!isValidIndex(indexChannel)) {
        std::cout << "On " << operation
                  << " not a valid channel: " << indexChannel << ".\n";
        return;
      }

      if (operation == "push") {
        gQueues[indexChannel].push(content);
      }

      if (operation == "pop") {
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
       // TODO uncomment this for debugging, very useful.
       //std::cout << operation << "\n";
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


/*
 * `select` area.
 * We consider the default case of waiting 5 seconds, when this timer
 * is reached, we cancel the entire task group.
 */
class SelectInstruction {
public:
  SelectInstruction() {
  }

  ~SelectInstruction() {
  }

  void execute(task_group& tg) {
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
      //    https://software.intel.com/en-us/node/506289.
      // enum task_group_status {
      //     not_complete, // Not cancelled and not all tasks in group have completed.
      //     complete,     // Not cancelled and all tasks in group have completed
      //     canceled      // Task group received cancellation request
      // };
      if (tg.wait() == complete) {
        std::cout << "Default case reached.\n";
        break;
      }
    }

    tg.wait();
  }
};


void showExampleGeneral() {
  std::cout << "\n---------- Example - General behaviour ----------\n";

  gQueues.clear();
  gQueues.resize(gKDefaultnumberOfChannels); // TODO change this

  // IMPORTANT
  // We have to harcode this statement everytime in the injected code area, because this is the place where it works to be, 
  // in order to have the desired behaviour. 
  trigger = 0;

  task_group tg;

  // queue indices starting with 1, to have effect the fetch_and_add(number)
  tg.run(Hello("push", 1, 5));
  tg.run(Hello("push", 1, 8));
  tg.run(Hello("push", 2, 10));
  tg.run(Hello("push", 3, 5));

  const unsigned int randomIndexChannel = (getRandom() % 3) + 1;

  std::cout << "Gonna pop from channel " << randomIndexChannel << ".\n";
  // This pop is hardcoded for the example, but it actually triggers
  // when the first pop is encountered.
  tg.run(Hello("pop", randomIndexChannel));
  //tg.run(Hello("pop", 4));

  tg.run(DefaultTimer(5));


  SelectInstruction selectInstruction;
  selectInstruction.execute(tg);
}


void showExamplePopFromEmptyChannel() {
  std::cout << "\n---------- Example - pop() from empty channel ----------\n";

  gQueues.clear();
  gQueues.resize(gKDefaultnumberOfChannels); // TODO change this

  trigger = 0;
  task_group tg;

  // queue indices starting with 1, to have effect the fetch_and_add(number)
  tg.run(Hello("push", 1, 5));
  tg.run(Hello("push", 1, 8));
  tg.run(Hello("push", 2, 10));
  tg.run(Hello("push", 3, 5));

  const unsigned int indexChannel = 4;

  std::cout << "Gonna pop from channel " << indexChannel << ".\n";
  // This pop is hardcoded for the example, but it actually triggers
  // when the first pop is encountered.
  tg.run(Hello("pop", indexChannel));

  tg.run(DefaultTimer(5));

  SelectInstruction selectInstruction;
  selectInstruction.execute(tg);
}

/*
 * Triggering the default case of `select` when we don't encounter any pop()
 * from a channel, aka
 * when we don't receive any item from a channel after `numberOfSeconds` seconds.
 */
void showExampleDefaultCase(const int& numberOfSeconds = 5) {
  std::cout << "\n---------- Example - reaching default case after "
            << numberOfSeconds << " seconds ----------\n";

  gQueues.clear();
  gQueues.resize(gKDefaultnumberOfChannels); // TODO change this

  trigger = 0;
  task_group tg;

  // queue indices starting with 1, to have effect the fetch_and_add(number)
  tg.run(Hello("push", 1, 5));
  tg.run(Hello("push", 1, 8));
  tg.run(Hello("push", 2, 10));
  tg.run(Hello("push", 3, 5));

  // We haven't called any pop(), so the default case will be triggered.
  tg.run(DefaultTimer(numberOfSeconds));

  SelectInstruction selectInstruction;
  selectInstruction.execute(tg);
}


void showExampleNotAValidChannel() {
  std::cout << "\n---------- Example - not a valid channel ----------\n";

  gQueues.clear();
  gQueues.resize(gKDefaultnumberOfChannels); // TODO change this

  trigger = 0;
  task_group tg;

  // queue indices starting with 1, to have effect the fetch_and_add(number)
  tg.run(Hello("push", 1, 5));
  tg.run(Hello("push", 1, 8));
  tg.run(Hello("push", 2, 10));
  tg.run(Hello("push", 3, 5));

  tg.run(Hello("push", gKDefaultnumberOfChannels + 1, 1));
  tg.run(Hello("pop", gKDefaultnumberOfChannels + 2, 2));


  // We haven't called any pop(), so the default case will be triggered.
  tg.run(DefaultTimer(5));

  SelectInstruction selectInstruction;
  selectInstruction.execute(tg);
}



void runExampleWithTaskGroup() {
  showExampleGeneral();
  showExamplePopFromEmptyChannel();
  showExampleDefaultCase();
  showExampleNotAValidChannel();
}


int main() {
    runExampleWithTaskGroup();

    return 0;
}
