#ifndef RISCV_H
#define RISCV_H

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "koopa.h"

class RISCV_Builder {
  class Env {
    //enum Register_State { UNUSED, USED };
    int stack_size = 0;
    int cur_size = 0;
    //std::map<koopa_raw_value_t, std::string>
      //  register_alloc_map;
    //std::map<std::string, Register_State> register_state_map;
    std::map<koopa_raw_value_t, int> addr_map;
   public:
    void init(int size);
    //void state_free(std::string reg);
    int get_addr(koopa_raw_value_t value);
    int get_stack_size() { return stack_size; };
    //std::string get_register(koopa_raw_value_t value);
    //std::string register_check(koopa_raw_value_t value);
  };
  Env env;
  std::ofstream out;
  static int func_size(koopa_raw_function_t func);
  static int bb_size(koopa_raw_basic_block_t bb);
  static int inst_size(koopa_raw_value_t value);
  void load_register(koopa_raw_value_t value, std::string reg);
  void store_stack(koopa_raw_value_t value, std::string reg);
  void raw_visit(const koopa_raw_program_t &raw);
  void raw_visit(const koopa_raw_slice_t &slice);
  void raw_visit(const koopa_raw_function_t &func);
  void raw_visit(const koopa_raw_basic_block_t &bb);
  void raw_visit(const koopa_raw_value_t &value);
  void raw_visit(const koopa_raw_return_t &return_value);
  //void raw_visit(const koopa_raw_integer_t &integer_value);
  void raw_visit(const koopa_raw_binary_t &binary_value, int addr);
  void raw_visit(const koopa_raw_load_t &load_value, int addr);
  void raw_visit(const koopa_raw_store_t &store_value);
  void raw_visit(const koopa_raw_branch_t &branch_value);
  void raw_visit(const koopa_raw_jump_t &jump_value);
 public:
   RISCV_Builder(const char *path) {
      out.open(path);
   };
   void build(koopa_raw_program_t raw);
};

#endif  // RISCV_H
