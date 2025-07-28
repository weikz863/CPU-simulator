#pragma once
#ifndef _MY_CONTROL_HPP_
#define _MY_CONTROL_HPP_

#include "tools.h"
#include <array>
#include <iostream>

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
  Register<1> finished;
};

struct ControlError {
};

struct ControlModule : dark::Module<ControlInput, ControlOutput, ControlPrivate> {
  static constexpr int RoBCAPACITY = 1; // this cannot be changed!
  int RoBsize;
  bool mem_busy, ALU_busy;
  Register<32> *loading_to, *ALU_to;
  ControlModule() : dark::Module<ControlInput, ControlOutput, ControlPrivate>{}, RoBsize{0}, 
      mem_busy{0}, ALU_busy{0}, loading_to{&reg[0]}, ALU_to{&reg[0]} {
    ;
  }
  void work() {
    if (static_cast<unsigned>(finished)) {
      finished <= 0;
      commit();
    }
    if (static_cast<unsigned>(mem_done)) {
      mem_busy = false;
      if (loading_to != &reg[0]) *loading_to <= mem_result;
      if (loading_to == &current_instruction) {
        parse_instruction();
      } else { // this is a load or store instruction finished
        finished <= 1;
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
      if (static_cast<unsigned>(current_instruction) == 0x0ff00513u) {
        throw static_cast<unsigned>(reg[10]) & 0xffu;
      }
      if (ALU_to != &reg[0]) *ALU_to <= ALU_result;
      if (ALU_to == &logic) { // encounter branch
        if (static_cast<unsigned>(ALU_result)) { // all guessing "not jump"
          pc <= default_jmp;
        }
        finished <= 1;
      } else { // this is an arithmetic instruction finished
        finished <= 1;
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
    #ifdef _DEBUG
    // std::cerr << "reg[10] = " << std::dec << static_cast<unsigned>(reg[10]) << std::endl;
    std::cerr << "commit()" << std::endl;
    #endif
    // if (static_cast<unsigned>(current_instruction) == 0x0ff00513u) {
    //   throw static_cast<unsigned>(reg[10]) & 0xffu;
    // }
    RoBsize--;
  }
  void parse_instruction() {
    Bit<32> instruction = mem_result;
    #ifdef _DEBUG
    std::cerr << std::hex << static_cast<unsigned>(instruction) << std::endl;
    #endif
    switch (static_cast<unsigned>(instruction.slice<7>(0))) { // opcode
      case 0x33: { // R-type arithmetic
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        unsigned rs2 = static_cast<unsigned>(instruction.slice<5>(20));
        Bit<7> funct7 = instruction.slice<7>(25);
        ALU_busy = true;
        ALU_to = &reg[rd];
        ALU_issue <= 1;
        ALU_mode <= Bit<4>({funct3, funct7[5]});
        ALU_value1 <= reg[rs1];
        ALU_value2 <= reg[rs2];
        pc <= pc + 4;
        break;
      }
      case 0x13: { // I-type arithmetic
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        if (static_cast<unsigned>(funct3) == 1 || static_cast<unsigned>(funct3) == 5) { // I*-type shift 
          Bit<5> imm = instruction.slice<5>(20);
          Bit<7> funct7 = instruction.slice<7>(25);
          ALU_busy = true;
          ALU_to = &reg[rd];
          ALU_issue <= 1;
          ALU_mode <= Bit<4>({funct3, funct7[5]});
          ALU_value1 <= reg[rs1];
          ALU_value2 <= zero_extend(imm);
        } else {
          Bit<32> imm = sign_extend(instruction.slice<12>(20));
          ALU_busy = true;
          ALU_to = &reg[rd];
          ALU_issue <= 1;
          ALU_mode <= Bit<4>({funct3, Bit<1>(0)});
          ALU_value1 <= reg[rs1];
          ALU_value2 <= imm;
        }
        pc <= pc + 4;
        break;
      }
      case 0x03: { // I-type load
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        Bit<32> imm = sign_extend(instruction.slice<12>(20));
        mem_busy = true;
        loading_to = &reg[rd];
        mem_addr <= reg[rs1];
        mem_delta <= imm;
        mem_issue <= 1;
        mem_mode <= funct3;
        pc <= pc + 4;
        break;
      }
      case 0x23: { // S-type store
        Bit<5> imm_lower = instruction.slice<5>(7);
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        unsigned rs2 = static_cast<unsigned>(instruction.slice<5>(20));
        Bit<7> imm_higher = instruction.slice<7>(25);
        mem_busy = true;
        loading_to = &reg[0];
        mem_addr <= reg[rs1];
        mem_delta <= sign_extend(Bit<12>({imm_higher, imm_lower}));
        mem_value <= reg[rs2];
        mem_issue <= 2;
        mem_mode <= funct3;
        pc <= pc + 4;
        break;
      }
      case 0x63: { // B-type branch
        Bit<5> imm_lower = instruction.slice<5>(7);
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        unsigned rs2 = static_cast<unsigned>(instruction.slice<5>(20));
        Bit<7> imm_higher = instruction.slice<7>(25);
        Bit<13> imm = {imm_higher[6], imm_lower[0], imm_higher.slice<6>(0), imm_lower.slice<4>(1), Bit<1>(0)};
        if (static_cast<unsigned>(funct3) == 0 || static_cast<unsigned>(funct3) == 1) { // beq and bne
          ALU_busy = true;
          ALU_to = &logic;
          ALU_issue <= 1;
          ALU_mode <= Bit<4>({~funct3, Bit<1>(1)});
          ALU_value1 <= reg[rs1];
          ALU_value2 <= reg[rs2];
        } else {
          ALU_busy = true;
          ALU_to = &logic;
          ALU_issue <= 1;
          ALU_mode <= Bit<4>({Bit<1>(0), funct3});
          ALU_value1 <= reg[rs1];
          ALU_value2 <= reg[rs2];
        }
        default_jmp <= pc + sign_extend(imm);
        pc <= pc + 4;
        break;
      }
      case 0x6f: { // J-type jal, jump and link
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<8> imm_19_12 = instruction.slice<8>(12);
        Bit<1> imm_11 = instruction.slice<1>(20);
        Bit<10> imm_10_1 = instruction.slice<10>(21);
        Bit<1> imm_20 = instruction.slice<1>(31);
        Bit<21> imm = {imm_20, imm_19_12, imm_11, imm_10_1, Bit<1>(0)};
        if (rd) reg[rd] <= pc + 4;
        pc <= pc + sign_extend(imm);
        finished <= true;
        break;
      }
      case 0x67: { // I-type jalr, jump and link register
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        Bit<32> imm = sign_extend(instruction.slice<12>(20));
        if (rd) reg[rd] <= pc + 4;
        #ifdef _DEBUG
        std::cerr << "jalr reg[rs1]: " << std::hex << static_cast<unsigned>(reg[rs1]) << std::endl;
        #endif
        pc <= ((reg[rs1] + static_cast<unsigned>(imm)) & 0xfffffffeu);
        finished <= true;
        break;
      }
      case 0x17: { // U-type auipc, add upper immediate to pc
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<20> imm_high = instruction.slice<20>(12);
        Bit<32> imm = {imm_high, Bit<12>(0)};
        if (rd) reg[rd] <= pc + static_cast<unsigned>(imm);
        pc <= pc + 4;
        finished <= true;
        break;
      }
      case 0x37: { // U-type lui, load upper immediate
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<20> imm_high = instruction.slice<20>(12);
        Bit<32> imm = {imm_high, Bit<12>(0)};
        if (rd) reg[rd] <= static_cast<unsigned>(imm);
        pc <= pc + 4;
        finished <= true;
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