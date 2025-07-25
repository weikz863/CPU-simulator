#include <iostream>
#include "tools.h"
#include "control.hpp"
#include "memory.hpp"
#include "ALU.hpp"

int main() {
  dark::CPU cpu;
  MemModule mem;
  ALUModule ALU;
  ControlModule control;

  mem.addr = [&]() -> auto& { return control.mem_addr; };
  mem.delta = [&]() -> auto& { return control.mem_delta; };
  mem.value = [&]() -> auto& { return control.mem_value; };
  mem.issue = [&]() -> auto& { return control.mem_issue; };
  mem.mode = [&]() -> auto& { return control.mem_mode; };

  ALU.value1 = [&]() -> auto& { return control.ALU_value1; };
  ALU.value2 = [&]() -> auto& { return control.ALU_value2; };
  ALU.issue = [&]() -> auto& { return control.ALU_issue; };
  ALU.mode = [&]() -> auto& { return control.ALU_mode; };

  control.mem_result = [&]() -> auto& { return mem.result; };
  control.mem_done = [&]() -> auto& { return mem.fin; };
  control.ALU_result = [&]() -> auto& { return ALU.result; };
  control.ALU_done = [&]() -> auto& { return ALU.fin; };

  cpu.add_module(&mem);
  cpu.add_module(&ALU);
  cpu.add_module(&control);

  try {
    cpu.run(150, true);
  }
  catch (unsigned x) {
    std::cout << x << std::endl;
  }
  catch (...) {
    std::cout << "oops" << std::endl;
  }
}