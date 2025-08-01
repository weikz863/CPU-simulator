#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include "tools.h"
// #include "control.hpp"
#include "memory.hpp"
// #include "ALU.hpp"
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
  // ALUModule ALU;
  IFModule IF;
  // ControlModule control;

  cpu.add_module(&mem);
  // cpu.add_module(&ALU);
  cpu.add_module(&IF);
  // cpu.add_module(&control);

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