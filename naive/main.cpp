#include <iostream>
#include "tools.h"

using TimerInput = dark::details::empty_class;
struct TimerOutput {
  Register<32> timer_output;
};
struct Timer : dark::Module<TimerInput, TimerOutput> {
  unsigned time;
  Timer() : dark::Module<TimerInput, TimerOutput>{}, time(0) {
    ;
  }
  void work() override final {
    time++;
    if (time % 3 == 0) timer_output <= time;
  }
};

struct CheckerInput {
  Wire<32> checker_input;
};
using CheckerOutput = dark::details::empty_class;
struct Checker : dark::Module<CheckerInput, CheckerOutput> {
  Checker() : dark::Module<CheckerInput, CheckerOutput>{} {
  }
  void work() override final {
    std::cout << static_cast<unsigned>(checker_input) << std::endl;
  }
};

int main() {
  dark::CPU cpu;
  Timer timer;
  Checker checker;
  checker.checker_input = [&]() -> auto& { return timer.timer_output; };
  cpu.add_module(&timer);
  cpu.add_module(&checker);

  cpu.run(20, true);
}