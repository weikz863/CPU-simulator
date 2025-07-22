#include <iostream>
#include "memory.hpp"
#include "tools.h"
int main() {
  MemModule mem;
  dark::CPU cpu;
  cpu.add_module(&mem);
  cpu.run(10, true);
}