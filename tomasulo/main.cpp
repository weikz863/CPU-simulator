#include <iostream>
#include <unordered_map>
#include "tools.h"
// #include "control.hpp"
#include "memory.hpp"
// #include "ALU.hpp"
// #include "IF.hpp"

std::unordered_map<unsigned, Bit<8>> memory_map;

int main() {
  dark::CPU cpu;
  MemModule mem;
  // ALUModule ALU;
  // IFModule IF;
  // ControlModule control;

  cpu.add_module(&mem);
  // cpu.add_module(&ALU);
  // cpu.add_module(&IF);
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