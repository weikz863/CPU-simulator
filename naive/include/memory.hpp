#pragma once
#ifndef _MY_MEMORY_HPP_
#define _MY_MEMORY_HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include "tools.h"

struct MemInput {
  Wire<32> addr;
  Wire<32> delta;
  Wire<32> value;
  Wire<2> issue;
  Wire<3> mode;
};

struct MemOutput {
  Register<1> fin;
  Register<32> result;
};

struct MemError {
};

struct MemModule : dark::Module<MemInput, MemOutput> {
  static unsigned const lag = 3;
  unsigned state, loc, val;
  Bit<2> issue_;
  Bit<3> mode_;
  std::unordered_map<unsigned, Bit<8>> mp;
  MemModule() : dark::Module<MemInput, MemOutput>{}, state{0}, loc{0}, val{0}, issue_{0}, mode_{0}, mp{} {
    unsigned place = 0, tmp = 0;
    std::string s;
    while (std::cin >> s) {
      if (s[0] == '@') {
        std::istringstream ss(std::string(s.data() + 1));
        ss >> std::hex >> place;
      } else {
        std::istringstream ss(s.data());
        ss >> std::hex >> tmp;
        mp[place] = tmp;
        place++;
      }
    }
  }
	void work() override final {
    if (static_cast<bool>(issue)) {
      if (state) {
        throw MemError();
      }
      state = 1;
      loc = static_cast<unsigned>(addr) + static_cast<unsigned>(delta);
      val = static_cast<unsigned>(value);
      issue_ = issue;
      mode_ = mode;
      fin <= false;
    } else if (state) {
      state++;
      if (state == lag) {
        fin <= true;
        state = 0;
        switch (static_cast<unsigned>(Bit<5>({issue_, mode_}))) {
          case 0b01010: { // lw, load word
            result <= Bit<32>({mp[loc + 3], mp[loc + 2], mp[loc + 1], mp[loc]}); // translate back from little endian
            break;
          }
          default: {
            result <= 0;
            break;
          }
        }
      }
    } else {
      fin <= false;
      result <= 0;
    }
  }
};

#endif