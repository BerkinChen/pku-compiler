#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include "koopa.h"

koopa_raw_slice_t slice(koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t slice(std::vector<const void *> &vec,
                        koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);

koopa_raw_type_t type_kind(koopa_raw_type_tag_t tag);

std::string raw_visit(const koopa_raw_program_t &raw);
std::string raw_visit(const koopa_raw_slice_t &slice);
std::string raw_visit(const koopa_raw_function_t &func);
std::string raw_visit(const koopa_raw_basic_block_t &bb);
std::string raw_visit(const koopa_raw_value_t &value);
std::string raw_visit(const koopa_raw_return_t &value_ret);
std::string raw_visit(const koopa_raw_integer_t &value_int);

void riscv_dump_to_file(koopa_raw_program_t raw, const char *output);

#endif // UTILS_H