#pragma once
#ifndef _MY_ALU_HPP_
#define _MY_ALU_HPP_

#include "tools.h"

struct ALUInput {
  Wire<32> value1;
  Wire<32> value2;
  Wire<1> issue;
  Wire<4> mode;
};

struct ALUOutput {
  Register<1> fin;
  Register<32> result;
};

struct ALUError {
};

struct ALUModule : dark::Module<ALUInput, ALUOutput> {
  static constexpr unsigned LAG = 1;
  unsigned value1_, value2_, state;
  Bit<4> mode_;
  ALUModule() : dark::Module<ALUInput, ALUOutput>{}, value1_{0}, value2_{0}, state{0}, mode_{0} {
    ;
  }
	void work() override final {
    if (static_cast<bool>(issue)) {
      if (state) {
        throw ALUError();
      }
      state = 1;
      value1_ = static_cast<unsigned>(value1);
      value2_ = static_cast<unsigned>(value2);
      mode_ = mode;
      fin <= false;
      result <= 0;
    } else if (state) {
      if (state == LAG) {
        fin <= true;
        state = 0;
        switch (static_cast<unsigned>(mode_)) {
          case 0b0000: {                             // add, add
            result <= (value1_ + value2_);
            break;
          }
          case 0b0001: {                             // sub, subtract
            result <= (value1_ - value2_);
            break;
          }
          case 0b1110: {                             // and, bitwise and
            result <= (value1_ & value2_);
            break;
          }
          case 0b1100: {                             // or, bitwise or
            result <= (value1_ | value2_);
            break;
          }
          case 0b1000: {                             // xor, bitwise xor
            result <= (value1_ ^ value2_);
            break;
          }
          case 0b0010: {                             // sll, shift left logical
            result <= (value1_ << value2_);
            break;
          }
          case 0b1010: {                             // srl, shift right logical
            result <= (value1_ >> value2_);
            break;
          }
          case 0b1011: {                             // sra, shift right arithmetic
            result <= static_cast<unsigned>(static_cast<signed>(value1_) >> value2_);
            break;
          }
          case 0b0100: {                             // slt, set if less than (signed)
            result <= static_cast<unsigned>(static_cast<signed>(value1_) < static_cast<signed>(value2_));
            break;
          }
          case 0b0110: {                             // sltu, set if less than (unsigned)
            result <= static_cast<unsigned>(value1_ < value2_);
            break;
          }
          case 0b0101: {                             // sge, set if greater than or equal (signed), used by bge
            result <= static_cast<unsigned>(static_cast<signed>(value1_) >= static_cast<signed>(value2_));
            break;
          }
          case 0b0111: {                             // sgeu, set if greater than or equal (unsigned), used by bgeu
            result <= static_cast<unsigned>(value1_ >= value2_);
            break;
          }
          case 0b1111: {                             // seq, set if equal, used by beq
            result <= static_cast<unsigned>(value1_ == value2_);
            break;
          }
          case 0b1101: {                             // sne, set if unequal, used by bne
            result <= static_cast<unsigned>(value1_ != value2_);
            break;
          }
          default: {
            throw ALUError();
            break;
          }
        }
      } else { // state != LAG
        state++;
        fin <= false;
        result <= 0;
      }
    } else { // !issue && !state, idle
      fin <= false;
      result <= 0;
    }
  }
};

#endif