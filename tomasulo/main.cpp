#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include "tools.h"
#include "control.hpp"
#include "memory.hpp"
#include "ALU.hpp"
#include "IF.hpp"

std::unordered_map<unsigned, Bit<8>> memory_map;

int main() {
  unsigned place = 0, tmp = 0;
  std::string s;
  while (std::cin >> s) {
    if (s[0] == '@') {
      std::istringstream ss(std::string(s.data() + 1));
      ss >> std::hex >> place;
    } else {
      std::istringstream ss(s.data());
      ss >> std::hex >> tmp;
      memory_map[place] = tmp;
      place++;
    }
  }
  dark::CPU cpu;
  MemModule mem;
  ALUModule ALU;
  IFModule IF;
  ControlModule control;

  ALU.value1 = [&]() -> auto& { return control.to_ALU.value1; };
  ALU.value2 = [&]() -> auto& { return control.to_ALU.value2; };
  ALU.issue = [&]() -> auto& { return control.to_ALU.issue; };
  ALU.mode = [&]() -> auto& { return control.to_ALU.mode; };
  ALU.to_input = [&]() -> auto& { return control.to_ALU.to_input; };

  IF.addr = [&]() -> auto& { return control.to_IF.addr; };
  IF.issue = [&]() -> auto& { return control.to_IF.issue; };

  mem.addr = [&]() -> auto& { return control.to_mem.addr; };
  mem.delta = [&]() -> auto& { return control.to_mem.delta; };
  mem.value = [&]() -> auto& { return control.to_mem.value; };
  mem.issue = [&]() -> auto& { return control.to_mem.issue; };
  mem.mode = [&]() -> auto& { return control.to_mem.mode; };
  mem.to_input = [&]() -> auto& { return control.to_mem.to_input; };

  control.from_IF.result = [&]() -> auto& { return IF.result; };
  control.from_IF.done = [&]() -> auto& { return IF.done; };
  control.from_IF.busy = [&]() -> auto& { return IF.busy; };

  control.from_ALU.done = [&]() -> auto& { return ALU.done; };
  control.from_ALU.result = [&]() -> auto& { return ALU.result; };
  control.from_ALU.full = [&]() -> auto& { return ALU.full; };
  control.from_ALU.to_output = [&]() -> auto& { return ALU.to_output; };
  
  control.from_mem.result = [&]() -> auto& { return mem.result; };
  control.from_mem.done = [&]() -> auto& { return mem.done; };
  control.from_mem.to_output = [&]() -> auto& { return mem.to_output; };
  control.from_mem.full = [&]() -> auto& { return mem.full; };

  cpu.add_module(&mem);
  cpu.add_module(&ALU);
  cpu.add_module(&IF);
  cpu.add_module(&control);

  try {
    cpu.run(1000000000, true);
  }
  catch (unsigned x) {
    std::cout << x << std::endl;
  }
  catch (...) {
    std::cout << "oops" << std::endl;
  }
}