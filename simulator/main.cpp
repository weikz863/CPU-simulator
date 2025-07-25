#include <iostream>
#include "tools.h"
#include "control.hpp"
#include "memory.hpp"
#include "ALU.hpp"

int main() {
  Bit<12> tmp = 0xfff;
  std::cout << std::hex << to_signed(tmp) << std::endl;
}