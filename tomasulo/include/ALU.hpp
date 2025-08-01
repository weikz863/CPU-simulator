#pragma once
#ifndef _MY_ALU_HPP_
#define _MY_ALU_HPP_

#include "tools.h"

struct ALUInput {
  Wire<32> value1;
  Wire<32> value2;
  Wire<1> issue;
  Wire<4> mode;
  Wire<5> to_input;
};

struct ALUOutput {
  Register<1> done;
  Register<32> result;
  Register<1> full;
  Register<5> to_output;
};

struct ALUError {
};

struct ALUModule : dark::Module<ALUInput, ALUOutput> {
  static constexpr unsigned LAG = 1;
  static constexpr unsigned MAX_CAPACITY = 8;
  struct instruction {
    unsigned state, value1, value2;
    Bit<4> mode;
    Bit<5> to;
  };
  std::list<instruction> queue;
  ALUModule() : dark::Module<ALUInput, ALUOutput>{}, queue{} {
    ;
  }
	void work() override final {
    if (static_cast<bool>(issue)) {
      if (full) {
        throw ALUError();
      }
      queue.push_back({0u, static_cast<unsigned>(value1), static_cast<unsigned>(value2),
                       mode, to_input});
      done <= false;
      result <= 0;
      to_output <= 0;
    }
    if (!queue.empty()) {
      auto &[state, value1, value2, mode, to] = queue.front();
      if (state == LAG) {
        done <= true;
        to_output <= to;
        state = 0;
        switch (static_cast<unsigned>(mode)) {
          case 0b0000: {                             // add, add
            result <= (value1 + value2);
            break;
          }
          case 0b0001: {                             // sub, subtract
            result <= (value1 - value2);
            break;
          }
          case 0b1110: {                             // and, bitwise and
            result <= (value1 & value2);
            break;
          }
          case 0b1100: {                             // or, bitwise or
            result <= (value1 | value2);
            break;
          }
          case 0b1000: {                             // xor, bitwise xor
            result <= (value1 ^ value2);
            break;
          }
          case 0b0010: {                             // sll, shift left logical
            result <= (value1 << value2);
            break;
          }
          case 0b1010: {                             // srl, shift right logical
            result <= (value1 >> value2);
            break;
          }
          case 0b1011: {                             // sra, shift right arithmetic
            result <= static_cast<unsigned>(static_cast<signed>(value1) >> value2);
            break;
          }
          case 0b0100: {                             // slt, set if less than (signed)
            result <= static_cast<unsigned>(static_cast<signed>(value1) < static_cast<signed>(value2));
            break;
          }
          case 0b0110: {                             // sltu, set if less than (unsigned)
            result <= static_cast<unsigned>(value1 < value2);
            break;
          }
          case 0b0101: {                             // sge, set if greater than or equal (signed), used by bge
            result <= static_cast<unsigned>(static_cast<signed>(value1) >= static_cast<signed>(value2));
            break;
          }
          case 0b0111: {                             // sgeu, set if greater than or equal (unsigned), used by bgeu
            result <= static_cast<unsigned>(value1 >= value2);
            break;
          }
          case 0b1111: {                             // seq, set if equal, used by beq
            result <= static_cast<unsigned>(value1 == value2);
            break;
          }
          case 0b1101: {                             // sne, set if unequal, used by bne
            result <= static_cast<unsigned>(value1 != value2);
            break;
          }
          default: {
            throw ALUError();
            break;
          }
        }
        queue.pop_front();
      } else { // state != LAG
        state++;
        done <= false;
        result <= 0;
        to_output <= 0;
      }
    } else { // !issue && !state, idle
      done <= false;
      result <= 0;
      to_output <= 0;
    }
    full <= (queue.size() == MAX_CAPACITY);
  }
};

#endif