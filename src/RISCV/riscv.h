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
    //std::map<koopa_raw_value_t, std::string>
      //  register_alloc_map;
    //std::map<std::string, Register_State> register_state_map;
    std::map<koopa_raw_value_t, int> addr_map;
   public:
    bool is_call = false;
    int stack_size = 0;
    int cur_size = 0;
    void init(int size, bool call);
    //void state_free(std::string reg);
    int get_addr(koopa_raw_value_t value);
    //std::string get_register(koopa_raw_value_t value);
    //std::string register_check(koopa_raw_value_t value);
  };
  Env env;
  std::ofstream out;
  static int func_size(koopa_raw_function_t func, bool &call);
  static int bb_size(koopa_raw_basic_block_t bb, bool &call, int &max_arg);
  static int inst_size(koopa_raw_value_t value);
  static int type_size(koopa_raw_type_t ty);
  static int array_size(koopa_raw_type_t value);
  void load_register(koopa_raw_value_t value, std::string reg);
  void store_stack(int addr, std::string reg);
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
  void raw_visit(const koopa_raw_call_t &call_value, int addr);
  void global_alloc(const koopa_raw_value_t &global_alloc_value);
  void raw_visit(const koopa_raw_aggregate_t &aggregate_value);
  void raw_visit(const koopa_raw_get_elem_ptr_t &get_elem_ptr_value, int addr);
  void raw_visit(const koopa_raw_get_ptr_t &get_ptr_value, int addr);
 public:
   RISCV_Builder(const char *path) {
      out.open(path);
   };
   void build(koopa_raw_program_t raw);
};

#endif  // RISCV_H
