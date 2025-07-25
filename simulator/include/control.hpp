#pragma once
#ifndef _MY_CONTROL_HPP_
#define _MY_CONTROL_HPP_

#include "tools.h"
#include <array>

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

struct ControlPrivate {
  std::array<Register<32>, 32> reg;
  Register<32> pc;
  Register<32> logic;
  Register<32> current_instruction;
  Register<32> default_jmp; // this should be in reserve station, but there isn't.
};

struct ControlError {
};

struct ControlModule : dark::Module<ControlInput, ControlOutput> {
  static constexpr int RoBCAPACITY = 1; // this cannot be changed!
  int RoBsize;
  bool mem_busy, ALU_busy;
  Register<32> *loading_to, *ALU_to;
  ControlModule() : dark::Module<ControlInput, ControlOutput>{}, current_instruction{0}, RoBsize{0}, 
      mem_busy{0}, ALU_busy{0}, reg{}, pc{0}, loading_to{&reg[0]}, ALU_to{&reg[0]} {
    ;
  }
  void work() {
    if (static_cast<unsigned>(mem_done)) {
      mem_busy = false;
      *loading_to <= mem_result;
      if (loading_to == &current_instruction) {
        parse_instruction();
      } else { // this is a load or store instruction finished
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
    }
    if (static_cast<unsigned>(ALU_done)) {
      ALU_busy = false;
      *ALU_to <= ALU_result;
      if (ALU_to == &logic) { // encounter branch
        if (static_cast<unsigned>(ALU_result)) { // all guessing "not jump"
          pc <= default_jmp;
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
  void commit() {
    if (static_cast<unsigned>(current_instruction) == 0x0ff00513u) {
      throw static_cast<unsigned>(reg[10]) & 0xffu;
    }
    RoBsize--;
  }
  void parse_instruction() {
    switch (static_cast<unsigned>(mem_result.slice<7>(0))) { // opcode
      case 0x33: { // R-type arithmetic
        break;
      }
      case 0x13: { // I-type arithmetic
        break;
      }
      case 0x03: { // I-type load
        break;
      }
      case 0x23: { // S-type store
        break;
      }
      case 0x63: { // B-type branch
        break;
      }
      case 0x6f: { // J-type jal, jump and link
        break;
      }
      case 0x67: { // I-type jalr, jump and link register
        break;
      }
      case 0x17: { // U-type auipc, add upper immediate to pc
        break;
      }
      case 0x37: { // U-type lui, load upper immediate
        break;
      }
      default: {
        throw ControlError();
        break;
      }
    }
  }
};

#endif