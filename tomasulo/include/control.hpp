#pragma once
#ifndef _MY_CONTROL_HPP_
#define _MY_CONTROL_HPP_

#include "tools.h"
#include <array>
#include <iostream>

struct ControlInput {
  struct ControlFromIF {
    Wire<32> result;
    Wire<1> done;
    Wire<1> busy;
  } from_IF;
  struct ControlFromALU {
    Wire<1> done;
    Wire<32> result;
    Wire<1> full;
    Wire<5> to_output;
  } from_ALU;
  struct ControlFromMemory {
    Wire<32> result;
    Wire<1> done;
    Wire<5> to_output;
    Wire<1> full;
  } from_mem;
};

struct ControlOutput {
  struct ControlToIF {
    Register<32> addr;
    Register<1> issue;
  } to_IF;
  struct ControlToALU {
    Register<32> value1;
    Register<32> value2;
    Register<1> issue;
    Register<4> mode;
    Register<5> to_input;
  } to_ALU;
  struct ControlToMemory {
    Register<32> addr;
    Register<32> delta;
    Register<32> value;
    Register<2> issue;
    Register<3> mode;
    Register<5> to_input;
  } to_mem;
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
  static constexpr int RoBCAPACITY = 8;
  ControlModule() : dark::Module<ControlInput, ControlOutput, ControlPrivate>{} {
    ;
  }
  void work() {
  }
  void commit() {
  }
  // void parse_instruction() {
  //   Bit<32> instruction = 0;
  //   #ifdef _DEBUG
  //   std::cerr << std::hex << static_cast<unsigned>(instruction) << std::endl;
  //   #endif
  //   switch (static_cast<unsigned>(instruction.slice<7>(0))) { // opcode
  //     case 0x33: { // R-type arithmetic
  //       unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
  //       Bit<3> funct3 = instruction.slice<3>(12);
  //       unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
  //       unsigned rs2 = static_cast<unsigned>(instruction.slice<5>(20));
  //       Bit<7> funct7 = instruction.slice<7>(25);
  //       ALU_busy = true;
  //       ALU_to = &reg[rd];
  //       ALU_issue <= 1;
  //       ALU_mode <= Bit<4>({funct3, funct7[5]});
  //       ALU_value1 <= reg[rs1];
  //       ALU_value2 <= reg[rs2];
  //       pc <= pc + 4;
  //       break;
  //     }
  //     case 0x13: { // I-type arithmetic
  //       unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
  //       Bit<3> funct3 = instruction.slice<3>(12);
  //       unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
  //       if (static_cast<unsigned>(funct3) == 1 || static_cast<unsigned>(funct3) == 5) { // I*-type shift 
  //         Bit<5> imm = instruction.slice<5>(20);
  //         Bit<7> funct7 = instruction.slice<7>(25);
  //         ALU_busy = true;
  //         ALU_to = &reg[rd];
  //         ALU_issue <= 1;
  //         ALU_mode <= Bit<4>({funct3, funct7[5]});
  //         ALU_value1 <= reg[rs1];
  //         ALU_value2 <= zero_extend(imm);
  //       } else {
  //         Bit<32> imm = sign_extend(instruction.slice<12>(20));
  //         ALU_busy = true;
  //         ALU_to = &reg[rd];
  //         ALU_issue <= 1;
  //         ALU_mode <= Bit<4>({funct3, Bit<1>(0)});
  //         ALU_value1 <= reg[rs1];
  //         ALU_value2 <= imm;
  //       }
  //       pc <= pc + 4;
  //       break;
  //     }
  //     case 0x03: { // I-type load
  //       unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
  //       Bit<3> funct3 = instruction.slice<3>(12);
  //       unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
  //       Bit<32> imm = sign_extend(instruction.slice<12>(20));
  //       mem_busy = true;
  //       loading_to = &reg[rd];
  //       mem_addr <= reg[rs1];
  //       mem_delta <= imm;
  //       mem_issue <= 1;
  //       mem_mode <= funct3;
  //       pc <= pc + 4;
  //       break;
  //     }
  //     case 0x23: { // S-type store
  //       Bit<5> imm_lower = instruction.slice<5>(7);
  //       Bit<3> funct3 = instruction.slice<3>(12);
  //       unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
  //       unsigned rs2 = static_cast<unsigned>(instruction.slice<5>(20));
  //       Bit<7> imm_higher = instruction.slice<7>(25);
  //       mem_busy = true;
  //       loading_to = &reg[0];
  //       mem_addr <= reg[rs1];
  //       mem_delta <= sign_extend(Bit<12>({imm_higher, imm_lower}));
  //       mem_value <= reg[rs2];
  //       mem_issue <= 2;
  //       mem_mode <= funct3;
  //       pc <= pc + 4;
  //       break;
  //     }
  //     case 0x63: { // B-type branch
  //       Bit<5> imm_lower = instruction.slice<5>(7);
  //       Bit<3> funct3 = instruction.slice<3>(12);
  //       unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
  //       unsigned rs2 = static_cast<unsigned>(instruction.slice<5>(20));
  //       Bit<7> imm_higher = instruction.slice<7>(25);
  //       Bit<13> imm = {imm_higher[6], imm_lower[0], imm_higher.slice<6>(0), imm_lower.slice<4>(1), Bit<1>(0)};
  //       if (static_cast<unsigned>(funct3) == 0 || static_cast<unsigned>(funct3) == 1) { // beq and bne
  //         ALU_busy = true;
  //         ALU_to = &logic;
  //         ALU_issue <= 1;
  //         ALU_mode <= Bit<4>({~funct3, Bit<1>(1)});
  //         ALU_value1 <= reg[rs1];
  //         ALU_value2 <= reg[rs2];
  //       } else {
  //         ALU_busy = true;
  //         ALU_to = &logic;
  //         ALU_issue <= 1;
  //         ALU_mode <= Bit<4>({Bit<1>(0), funct3});
  //         ALU_value1 <= reg[rs1];
  //         ALU_value2 <= reg[rs2];
  //       }
  //       default_jmp <= pc + sign_extend(imm);
  //       pc <= pc + 4;
  //       break;
  //     }
  //     case 0x6f: { // J-type jal, jump and link
  //       unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
  //       Bit<8> imm_19_12 = instruction.slice<8>(12);
  //       Bit<1> imm_11 = instruction.slice<1>(20);
  //       Bit<10> imm_10_1 = instruction.slice<10>(21);
  //       Bit<1> imm_20 = instruction.slice<1>(31);
  //       Bit<21> imm = {imm_20, imm_19_12, imm_11, imm_10_1, Bit<1>(0)};
  //       if (rd) reg[rd] <= pc + 4;
  //       pc <= pc + sign_extend(imm);
  //       finished <= true;
  //       break;
  //     }
  //     case 0x67: { // I-type jalr, jump and link register
  //       unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
  //       Bit<3> funct3 = instruction.slice<3>(12);
  //       unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
  //       Bit<32> imm = sign_extend(instruction.slice<12>(20));
  //       if (rd) reg[rd] <= pc + 4;
  //       #ifdef _DEBUG
  //       std::cerr << "jalr reg[rs1]: " << std::hex << static_cast<unsigned>(reg[rs1]) << std::endl;
  //       #endif
  //       pc <= ((reg[rs1] + static_cast<unsigned>(imm)) & 0xfffffffeu);
  //       finished <= true;
  //       break;
  //     }
  //     case 0x17: { // U-type auipc, add upper immediate to pc
  //       unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
  //       Bit<20> imm_high = instruction.slice<20>(12);
  //       Bit<32> imm = {imm_high, Bit<12>(0)};
  //       if (rd) reg[rd] <= pc + static_cast<unsigned>(imm);
  //       pc <= pc + 4;
  //       finished <= true;
  //       break;
  //     }
  //     case 0x37: { // U-type lui, load upper immediate
  //       unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
  //       Bit<20> imm_high = instruction.slice<20>(12);
  //       Bit<32> imm = {imm_high, Bit<12>(0)};
  //       if (rd) reg[rd] <= static_cast<unsigned>(imm);
  //       pc <= pc + 4;
  //       finished <= true;
  //       break;
  //     }
  //     default: {
  //       throw ControlError();
  //       break;
  //     }
  //   }
  // }
};

#endif