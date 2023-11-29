#ifndef RISCV_H
#define RISCV_H

#include <string>
#include <vector>
#include "koopa.h"

std::string raw_visit(const koopa_raw_program_t &raw);
std::string raw_visit(const koopa_raw_slice_t &slice);
std::string raw_visit(const koopa_raw_function_t &func);
std::string raw_visit(const koopa_raw_basic_block_t &bb);
std::string raw_visit(const koopa_raw_value_t &value);
std::string raw_visit(const koopa_raw_return_t &value_ret);
std::string raw_visit(const koopa_raw_integer_t &value_int);

void raw_dump_to_riscv(koopa_raw_program_t raw, const char *output);

#endif  // RISCV_H