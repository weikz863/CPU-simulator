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
  Bit<32> current_instruction;
  int RoBsize;
  bool mem_busy, ALU_busy;
  Bit<32> reg[32], pc, logic, *loading_to, *ALU_to;
  Bit<32> default_jmp; // this should be in reserve station, but there isn't.
  ControlModule() : dark::Module<ControlInput, ControlOutput>{}, current_instruction{0}, RoBsize{0}, 
      mem_busy{0}, ALU_busy{0}, reg{}, pc{0}, loading_to{&reg[0]}, ALU_to{&reg[0]} {
    ;
  }
  void work() {
    if (static_cast<unsigned>(mem_done)) {
      mem_busy = false;
      *loading_to = mem_result;
      if (loading_to == &current_instruction) {
        parse_instruction();
      } else { // this is a load instruction finished
        commit();
      }
    }
    if (!mem_busy && RoBsize < RoBCAPACITY) { // load new instruction
      loading_to = &current_instruction;
      mem_busy = true;
      RoBsize++;
      mem_addr <= pc;
      mem_delta <= 0;
      mem_issue <= 1; // 1 = load
      mem_mode <= 2; // 2 = word
      pc += 4;
    }
    if (static_cast<unsigned>(ALU_done)) {
      ALU_busy = false;
      *ALU_to = ALU_result;
      if (ALU_to == &logic) { // encounter branch
        if (static_cast<unsigned>(logic)) { // all guessing "not jump"
          pc = default_jmp;
        }
      } else { // this is an arithmetic instruction finished
        commit();
      }
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