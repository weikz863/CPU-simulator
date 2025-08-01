#pragma once
#ifndef _MY_INSTRUCTION_FETCH_HPP_
#define _MY_INSTRUCTION_FETCH_HPP_

#include <iostream>
#include <unordered_map>
#include "tools.h"

extern std::unordered_map<unsigned, Bit<8>> memory_map;

struct IFInput {
  Wire<32> addr;
  Wire<1> issue;
};

struct IFOutput {
  Register<32> result;
  Register<1> done;
};

struct IFError {
};

extern std::unordered_map<unsigned, Bit<8>> memory_map;

struct IFModule : dark::Module<IFInput, IFOutput> {
  static constexpr unsigned LAG = 1;
  unsigned address, state;
  const std::unordered_map<unsigned, Bit<8>>& mp;
  IFModule() : dark::Module<IFInput, IFOutput>{}, state{0}, address{0}, mp{memory_map} {
  }
	void work() override final {
    if (static_cast<bool>(issue)) {
      if (state) {
        throw IFError();
      }
      state = 1;
      address = static_cast<unsigned>(addr);
      done <= false;
      result <= 0;
    } else if (state) {
      if (state == LAG) {
        result <= Bit<32>({mp.at(address + 3), mp.at(address + 2), mp.at(address + 1), mp.at(address)});
        done <= true;
        state = 0;
      } else { // state != LAG
        result <= 0;
        done <= false;
        state++;
      }
    } else { // !issue && !state, idle
      result <= 0;
      done <= false;
    }
  }
};

#endif