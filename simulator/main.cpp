#include <iostream>
#include "tools.h"

int main() {
  Bit<32> tmp = 0x010237;
  std::cout << std::hex << static_cast<unsigned>(tmp.slice<7>(0)) << std::endl;
}