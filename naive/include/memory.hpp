#pragma once
#ifndef _MY_MEMORY_HPP_
#define _MY_MEMORY_HPP_

#include <iostream>
#include <unordered_map>
#include "tools.h"

struct MemInput {
  Wire<32> addr;
  Wire<32> delta;
  Wire<32> value;
  Wire<1> activate;
  Wire<1> mode;
};

struct MemOutput {
  Register<1> fin;
  Register<32> result;
};

struct MemError {
};

struct MemModule : dark::Module<MemInput, MemOutput> {
  static unsigned const lag = 3;
  unsigned state = 0, loc = 0, val = 0;
  bool mod = 0; // 0 = read, 1 = write
  std::unordered_map<unsigned, unsigned> mp = {};
	void work() override final {
    if (static_cast<bool>(activate)) {
      if (state) {
        throw MemError();
      }
      state = 1;
      loc = static_cast<unsigned>(addr) + static_cast<unsigned>(delta);
      val = static_cast<unsigned>(value);
      mod = static_cast<bool>(mode);
      fin <= false;
    } else if (state) {
      state++;
      if (state == lag) {
        fin <= true;
        result <= (mod ? 0 : mp[loc]);
        if (mod == 1) {
          mp[loc] = val;
        }
        state = 0;
      }
    } else {
      fin <= false;
      result <= 0;
    }
  }
};
#endif