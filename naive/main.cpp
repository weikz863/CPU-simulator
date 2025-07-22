#include <iostream>
#include "memory.hpp"
#include "tools.h"
int main() {
  dark::CPU cpu;

  unsigned issue = 0, mode = 0;

  MemModule mem;
  mem.addr = [&]() { return 0X00000834; };
  mem.delta = [&]() { return 0X00000834; };
  mem.value = [&]() { return 0; };
  mem.issue = [&]() { return issue; };
  mem.mode = [&]() { return mode; };
  cpu.add_module(&mem);

  issue = 1;
  mode = 2;
  cpu.run_once();
  issue = 0;
  mode = 0;
  while (!static_cast<unsigned>(mem.fin)) cpu.run_once();
  std::cout << std::hex << static_cast<unsigned>(mem.result) << std::endl;
}