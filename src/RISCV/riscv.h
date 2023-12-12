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
    enum Register_State { UNUSED, USED };
    std::map<koopa_raw_value_t, std::string>
        register_alloc_map;
    std::map<std::string, Register_State> register_state_map;
   public:
    void state_init();
    void state_free(std::string reg);
    std::string register_check(koopa_raw_value_t value);
    std::string register_alloc(koopa_raw_value_t value);
  };
  Env env;
  std::string load_register(koopa_raw_value_t value, std::string reg);
  std::string raw_visit(const koopa_raw_program_t &raw);
  std::string raw_visit(const koopa_raw_slice_t &slice);
  std::string raw_visit(const koopa_raw_function_t &func);
  std::string raw_visit(const koopa_raw_basic_block_t &bb);
  std::string raw_visit(const koopa_raw_value_t &value);
  std::string raw_visit(const koopa_raw_return_t &return_value);
  //std::string raw_visit(const koopa_raw_integer_t &integer_value);
  std::string raw_visit(const koopa_raw_binary_t &binary_value);
 public:
  RISCV_Builder() = default;
  void build(koopa_raw_program_t raw, const char *path);
};

#endif  // RISCV_H
