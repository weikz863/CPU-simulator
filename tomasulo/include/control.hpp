#pragma once
#ifndef _MY_CONTROL_HPP_
#define _MY_CONTROL_HPP_

#include "tools.h"
#include <array>
#include <iostream>
#include "ArrDeque.hpp"

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
  std::array<Register<5>, 32> reg_name;
  Register<32> pc;
};

struct ControlError {
};

struct ControlModule : dark::Module<ControlInput, ControlOutput, ControlPrivate> {
  static constexpr int RoBCAPACITY = 8;
  struct RoBEntry {
    unsigned instruction, rs1_name, rs2_name, val1, val2, rd_name, rd;
    bool finished;
    enum class InstructionType {
      ARITHMETIC, MEMORY, CONTROL,
    } type;
  };
  ArrDeque<RoBEntry, RoBCAPACITY> reorder_buffer;
  ControlModule() : dark::Module<ControlInput, ControlOutput, ControlPrivate>{} {
    ;
  }
  void work() {
    if (reorder_buffer.size() < RoBCAPACITY && !from_IF.busy) {
      to_IF.addr <= pc;
      to_IF.issue <= true;
    } else {
      to_IF.addr <= 0;
      to_IF.issue <= false;
    }
    if (from_IF.done) {
      parse_instruction();
    }
    bool all_ARITHMETIC = true;
    for (auto it = reorder_buffer.begin(); it != reorder_buffer.end(); ++it) {
      if (insturction.rs1_name == 0 && instruction.rs2_name == 0 && (all_ARITHMETIC || insturction.type == InstructionType::ARITHMETIC)) {
        issue(instruction, static_cast<unsigned>(it));
        break;
      }
      if (insturction.type != InstructionType::ARITHMETIC) all_ARITHMETIC = false;
    }
    if (from_ALU.done) {
      auto it = reorder_buffer.construct_iterator(from_ALU.to_output);
      it->finished = true;
      for (auto& t : reorder_buffer) {
        if (t.rs1_name == from_ALU.to_output) {
          t.rs1_name = 0;
          t.val1 = from_ALU.result;
        }
        if (t.rs2_name == from_ALU.to_output) {
          t.rs2_name = 0;
          t.val2 = from_ALU.result;
        }
      }
    }
    if (from_mem.done) {
      auto it = reorder_buffer.construct_iterator(from_mem.to_output);
      it->finished = true;
      for (auto& t : reorder_buffer) {
        if (t.rs1_name == from_mem.to_output) {
          t.rs1_name = 0;
          t.val1 = from_mem.result;
        }
        if (t.rs2_name == from_mem.to_output) {
          t.rs2_name = 0;
          t.val2 = from_mem.result;
        }
      }
    }
    if (!reorder_buffer.empty() && reorder_buffer.begin()->finished) commit();
  }
  void commit() {
  }
  void parse_instruction() {
    Bit<32> instruction = from_IF.result;
    switch (static_cast<unsigned>(instruction.slice<7>(0))) { // opcode
      case 0x33: { // R-type arithmetic
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        unsigned rs2 = static_cast<unsigned>(instruction.slice<5>(20));
        Bit<7> funct7 = instruction.slice<7>(25);
        Bit<5> tmp_name = reg_name[rd] + 1;
        if (tmp_name == 0) tmp_name = 1;
        reg_name[rd] <= tmp_name;
        reorder_buffer.push_back({static_cast<unsigned>(instruction), reg_name[rs1], reg_name[rs2], 0, 0,
            tmp_name, rd, false, RoBEntry::InstructionType::ARITHMETIC});
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
          Bit<5> tmp_name = reg_name[rd] + 1;
          if (tmp_name == 0) tmp_name = 1;
          reg_name[rd] <= tmp_name;
          reorder_buffer.push_back({static_cast<unsigned>(instruction), reg_name[rs1], 0, 0, zero_extend(imm),
              tmp_name, rd, false, RoBEntry::InstructionType::ARITHMETIC});
          pc <= pc + 4;
        } else {
          Bit<32> imm = sign_extend(instruction.slice<12>(20));
          Bit<5> tmp_name = reg_name[rd] + 1;
          if (tmp_name == 0) tmp_name = 1;
          reg_name[rd] <= tmp_name;
          reorder_buffer.push_back({static_cast<unsigned>(instruction), reg_name[rs1], 0, 0, imm,
              tmp_name, rd, false, RoBEntry::InstructionType::ARITHMETIC});
          pc <= pc + 4;
        }
        break;
      }
      case 0x03: { // I-type load
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        Bit<32> imm = sign_extend(instruction.slice<12>(20));
        Bit<5> tmp_name = reg_name[rd] + 1;
        if (tmp_name == 0) tmp_name = 1;
        reg_name[rd] <= tmp_name;
        reorder_buffer.push_back({static_cast<unsigned>(instruction), reg_name[rs1], 0, 0, imm,
            tmp_name, rd, false, RoBEntry::InstructionType::MEMORY});
        pc <= pc + 4;
        break;
      }
      case 0x23: { // S-type store
        Bit<5> imm_lower = instruction.slice<5>(7);
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        unsigned rs2 = static_cast<unsigned>(instruction.slice<5>(20));
        Bit<7> imm_higher = instruction.slice<7>(25);
        Bit<32> imm = sign_extend(Bit<12>({imm_higher, imm_lower}));
        Bit<5> tmp_name = reg_name[rd] + 1;
        if (tmp_name == 0) tmp_name = 1;
        reg_name[rd] <= tmp_name;
        reorder_buffer.push_back({static_cast<unsigned>(instruction), reg_name[rs1], 0, 0, imm,
            tmp_name, rd, false, RoBEntry::InstructionType::MEMORY});
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
        unsigned predicted = branch_predict(pc + 4, pc + sign_extend(imm));
        unsigned unpredicted = (pc + 4) ^ (pc + sign_extend(imm)) ^ predicted;
        reorder_buffer.push_back({static_cast<unsigned>(instruction), reg_name[rs1], reg_name[rs2], 0, 0,
            predicted, unpredicted, false, RoBEntry::InstructionType::CONTROL});
        pc <= predicted;
        break;
      }
      case 0x6f: { // J-type jal, jump and link
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<8> imm_19_12 = instruction.slice<8>(12);
        Bit<1> imm_11 = instruction.slice<1>(20);
        Bit<10> imm_10_1 = instruction.slice<10>(21);
        Bit<1> imm_20 = instruction.slice<1>(31);
        Bit<21> imm = {imm_20, imm_19_12, imm_11, imm_10_1, Bit<1>(0)};
        // TODO: turn this to arithmetic xor
        // if (rd) reg[rd] <= pc + 4;
        pc <= pc + sign_extend(imm);
        break;
      }
      case 0x67: { // I-type jalr, jump and link register
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<3> funct3 = instruction.slice<3>(12);
        unsigned rs1 = static_cast<unsigned>(instruction.slice<5>(15));
        Bit<32> imm = sign_extend(instruction.slice<12>(20));
        // TODO: block instruction flow
        // if (rd) reg[rd] <= pc + 4;
        // pc <= ((reg[rs1] + static_cast<unsigned>(imm)) & 0xfffffffeu);
        // finished <= true;
        break;
      }
      case 0x17: { // U-type auipc, add upper immediate to pc
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<20> imm_high = instruction.slice<20>(12);
        Bit<32> imm = {imm_high, Bit<12>(0)};
        // TODO: turn this to arithmetic xor
        // if (rd) reg[rd] <= pc + static_cast<unsigned>(imm);
        pc <= pc + 4;
        break;
      }
      case 0x37: { // U-type lui, load upper immediate
        unsigned rd = static_cast<unsigned>(instruction.slice<5>(7));
        Bit<20> imm_high = instruction.slice<20>(12);
        Bit<32> imm = {imm_high, Bit<12>(0)};
        // TODO: turn this to arithmetic xor
        // if (rd) reg[rd] <= static_cast<unsigned>(imm);
        pc <= pc + 4;
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