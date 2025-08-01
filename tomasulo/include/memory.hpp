#pragma once
#ifndef _MY_MEMORY_HPP_
#define _MY_MEMORY_HPP_

#include <iostream>
#include <unordered_map>
#include <list>
#include "tools.h"

struct MemInput {
  Wire<32> addr;
  Wire<32> delta;
  Wire<32> value;
  Wire<2> issue;
  Wire<3> mode;
  Wire<5> to_input;
};

struct MemOutput {
  Register<32> result;
  Register<1> done;
  Register<5> to_output;
  Register<1> full;
};

struct MemError {
};

extern std::unordered_map<unsigned, Bit<8>> memory_map;

struct MemModule : dark::Module<MemInput, MemOutput> {
  static constexpr unsigned LAG = 3;
  static constexpr unsigned MAX_CAPACITY = 8;
  struct instruction {
    unsigned state, place, value;
    Bit<2> issue;
    Bit<3> mode;
    Bit<5> to;
  };
  std::unordered_map<unsigned, Bit<8>>& mp;
  std::list<instruction> queue;
  MemModule() : dark::Module<MemInput, MemOutput>{}, queue{}, mp{memory_map} {
  }
	void work() override final {
    if (static_cast<bool>(issue)) {
      if (full) {
        throw MemError();
      }
      queue.push_back({0u, static_cast<unsigned>(addr) + static_cast<unsigned>(delta), static_cast<unsigned>(value), 
                       issue, mode, to_input});
      done <= false;
      result <= 0;
      to_output <= 0;
    }
    if (!queue.empty()) {
      auto &[state, place, value, issue, mode, to] = queue.front();
      if (state == LAG) {
        done <= true;
        to_output <= to;
        switch (static_cast<unsigned>(Bit<5>({issue, mode}))) {
          case 0b01010: {                                                         // lw, load word
            result <= Bit<32>({mp[place + 3], mp[place + 2], mp[place + 1], mp[place]});  // translate back from little endian
            break;
          }
          case 0b01001: {                                                         // lh, load half-word
            result <= sign_extend<32>(Bit<16>({mp[place + 1], mp[place]}));
            break;
          }
          case 0b01101: {                                                         // lhu, load half-word(unsigned)
            result <= zero_extend<32>(Bit<16>({mp[place + 1], mp[place]}));
            break;
          }
          case 0b01000: {                                                         // lb, load byte
            result <= sign_extend<32>(mp[place]);
            break;
          }
          case 0b01100: {                                                         // lbu, load byte(unsigned)
            result <= zero_extend<32>(mp[place]);
            break;
          }
          case 0b10010: case 0b10001: case 0b10000: {                            // all store instructions
            int num_bytes = 1 << static_cast<unsigned>(mode);                   // excellent observation!
            for (int i = 0; i < num_bytes; i++) {
              mp[place + i] = value; // automatic truncation
              value >>= 8;
            }
            result <= 0;
            break;
          }
          default: {
            throw MemError();
            break;
          }
        }
        queue.pop_front();
      } else { // state != LAG
        state++;
        done <= false;
        result <= 0;
      }
    } else { // idle
      done <= false;
      result <= 0;
    }
    full <= (queue.size() == MAX_CAPACITY);
  }
};

#endif