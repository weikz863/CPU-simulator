#pragma once
#ifndef _MY_MEMORY_HPP_
#define _MY_MEMORY_HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
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
  unsigned value1_, value2_;
  Bit<4> mode_;
  ALUModule() : dark::Module<ALUInput, ALUOutput>{}, value1_{0}, value2_{0}, mode_{0} {
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
        switch (static_cast<unsigned>(Bit<5>({issue_, mode_}))) {
          
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