#include <iostream>
#include <cstdio>
#include "memory.hpp"
#include "tools.h"
int main() {
  freopen("../sample/sample.data", "r", stdin);
  
  dark::CPU cpu;

  unsigned issue = 0, mode = 0, addr = 0, value = 0;

  MemModule mem;
  mem.addr = [&]() { return addr; };
  mem.delta = [&]() { return 0; };
  mem.value = [&]() { return value; };
  mem.issue = [&]() { return issue; };
  mem.mode = [&]() { return mode; };
  cpu.add_module(&mem);

  freopen("/dev/tty", "r", stdin);
  std::cin.clear();
  while (std::cin >> std::hex >> addr >> issue >> mode >> std::hex >> value) {
    cpu.run_once();
    issue = 0;
    while (!static_cast<unsigned>(mem.fin)) cpu.run_once();
    std::cout << std::hex << static_cast<unsigned>(mem.result) << std::endl;
  }
}