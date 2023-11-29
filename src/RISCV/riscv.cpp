#include "riscv.h"

#include <cassert>
#include <cstring>
#include <fstream>

std::string raw_visit(const koopa_raw_program_t &raw) {
  std::string ret = "  .text\n  .globl main\n";
  // TODO: values
  ret += raw_visit(raw.funcs);
  return ret;
}

std::string raw_visit(const koopa_raw_slice_t &slice) {
  std::string ret = "";
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        ret += raw_visit(reinterpret_cast<const koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        ret += raw_visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        ret += raw_visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
  return ret;
}

std::string raw_visit(const koopa_raw_function_t &func) {
  // TODO: params
  // TODO: used_by
  char *name = new char[strlen(func->name) - 1];
  memcpy(name, func->name + 1, strlen(func->name) - 1);
  std::string ret = std::string(name) + ":\n";
  ret += raw_visit(func->bbs);
  return ret;
}

std::string raw_visit(const koopa_raw_basic_block_t &bb) {
  // TODO: used_by
  // TODO: params
  std::string ret = "";
  ret += raw_visit(bb->insts);
  return ret;
}

std::string raw_visit(const koopa_raw_value_t &value) {
  std::string ret = "";
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      ret += raw_visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      ret += raw_visit(kind.data.integer);
      break;
    default:
      assert(false);
  }
  return ret;
}

std::string raw_visit(const koopa_raw_return_t &value_ret) {
  std::string ret = "  li a0, " + raw_visit(value_ret.value) + "\n";
  ret += "  ret\n";
  return ret;
}

std::string raw_visit(const koopa_raw_integer_t &value_int) {
  std::string ret = std::to_string(value_int.value);
  return ret;
}

void raw_dump_to_riscv(koopa_raw_program_t raw, const char *output) {
  std::ofstream fout(output);
  fout << raw_visit(raw);
  fout.close();
}