#pragma once
#ifndef _MY_CONTROL_HPP_
#define _MY_CONTROL_HPP_

#include "tools.h"

struct ControlInput {
  Wire<32> mem_result;
  Wire<1> mem_done;
  Wire<32> ALU_result;
  Wire<1> ALU_done;
};

struct ControlOutput {
  Register<32> mem_addr;
  Register<32> mem_delta;
  Register<32> mem_value;
  Register<2> mem_issue;
  Register<3> mem_mode;
  Register<32> ALU_value1;
  Register<32> ALU_value2;
  Register<1> ALU_issue;
  Register<4> ALU_mode;
};

struct ControlModule : dark::Module<ControlInput, ControlOutput> {
  static constexpr int RoBCAPACITY = 1; // this cannot be changed!
  Bit<32> current_command;
  int RoBsize;
  bool mem_busy, ALU_busy;
  Bit<32> reg[32], pc, *loading_to;
  ControlModule() : dark::Module<ControlInput, ControlOutput>{}, current_command{0}, RoBsize{0}, 
      mem_busy{0}, ALU_busy{0}, reg{}, pc{0}, loading_to{&reg[0]} {
    ;
  }
  void work() {
    if (static_cast<unsigned>(mem_done)) {
      mem_busy = false;
      *loading_to = mem_result;
    }
    if (!mem_busy && RoBsize < RoBCAPACITY) {
      loading_to = &current_command;
    }
    if (static_cast<unsigned>(mem_issue)) {
      mem_issue <= 0;
    }
    if (static_cast<unsigned>(ALU_issue)) {
      ALU_issue <= 0;
    }
  }
};

#endif