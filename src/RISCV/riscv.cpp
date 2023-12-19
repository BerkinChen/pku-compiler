#include "riscv.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <random>

int RISCV_Builder::func_size(koopa_raw_function_t func) {
  int size = 0;
  for (size_t i = 0; i < func->bbs.len; ++i) {
    auto ptr = func->bbs.buffer[i];
    size += bb_size(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
  }
  return size;
}

int RISCV_Builder::bb_size(koopa_raw_basic_block_t bb) {
  int size = 0;
  for (size_t i = 0; i < bb->insts.len; ++i) {
    auto ptr = bb->insts.buffer[i];
    size += inst_size(reinterpret_cast<koopa_raw_value_t>(ptr));
  }
  return size;
}

int RISCV_Builder::inst_size(koopa_raw_value_t inst) {
  switch (inst->ty->tag) {
  case KOOPA_RTT_INT32:
    return 4;
  case KOOPA_RTT_UNIT:
    return 0;
  case KOOPA_RTT_POINTER:
    return 4;
  default:
    return 0;
  }
}

void RISCV_Builder::Env::init(int size) {
  stack_size = size;
  cur_size = 0;
  addr_map.clear();
}

int RISCV_Builder::Env::get_addr(koopa_raw_value_t raw) {
  if (addr_map.find(raw) != addr_map.end()) {
    return addr_map[raw];
  } else {
    int t = inst_size(raw);
    if (t == 0)
      return -1;
    addr_map[raw] = cur_size;
    cur_size += t;
    return addr_map[raw];
  }
}

void RISCV_Builder::load_register(koopa_raw_value_t value, std::string reg) {
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    out << "  li " + reg + ", " +
               std::to_string(value->kind.data.integer.value) + "\n";
  } else {
    int addr = env.get_addr(value);
    if (addr != -1) {
      out << "  lw " + reg + ", " + std::to_string(addr) + "(sp)\n";
    } else {
      assert(false);
    }
  }
}

void RISCV_Builder::store_stack(koopa_raw_value_t value, std::string reg) {
  int addr = env.get_addr(value);
  if (addr != -1) {
    out << "  sw " + reg + ", " + std::to_string(addr) + "(sp)\n";
  } else {
    assert(false);
  }
}

void RISCV_Builder::raw_visit(const koopa_raw_program_t &raw) {
  out << "  .text\n";
  // TODO: values
  raw_visit(raw.funcs);
}

void RISCV_Builder::raw_visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    switch (slice.kind) {
    case KOOPA_RSIK_FUNCTION:
      raw_visit(reinterpret_cast<const koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      // 访问基本块
      raw_visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      // 访问指令
      raw_visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    default:
      // 我们暂时不会遇到其他内容, 于是不对其做任何处理
      assert(false);
    }
  }
}

void RISCV_Builder::raw_visit(const koopa_raw_function_t &func) {
  // TODO: params
  // TODO: used_by
  char *name = new char[strlen(func->name) - 1];
  memcpy(name, func->name + 1, strlen(func->name) - 1);
  out << ".globl  " << std::string(name) + "\n";
  out << std::string(name) + ":\n";
  int size = func_size(func);
  size = (size + 15) / 16 * 16;
  if (size < 2048) {
    out << "  addi sp, sp, -" + std::to_string(size) + "\n";
  } else {
    out << "  li t0, -" + std::to_string(size) + "\n";
    out << "  add sp, sp, t0\n";
  }
  env.init(size);
  raw_visit(func->bbs);
}

void RISCV_Builder::raw_visit(const koopa_raw_basic_block_t &bb) {
  // TODO: used_by
  // TODO: params
  raw_visit(bb->insts);
}

void RISCV_Builder::raw_visit(const koopa_raw_value_t &value) {

  const auto &kind = value->kind;
  int addr = env.get_addr(value);
  switch (kind.tag) {
  case KOOPA_RVT_RETURN:
    raw_visit(kind.data.ret);
    break;
  case KOOPA_RVT_INTEGER:
    // raw_visit(kind.data.integer);
    break;
  case KOOPA_RVT_ALLOC:
    break;
  case KOOPA_RVT_LOAD:
    raw_visit(kind.data.load, addr);
    break;
  case KOOPA_RVT_STORE:
    raw_visit(kind.data.store);
    break;
  case KOOPA_RVT_BINARY:
    raw_visit(kind.data.binary, addr);
    break;
  default:
    assert(false);
  }
}

void RISCV_Builder::raw_visit(const koopa_raw_return_t &ret_value) {
  load_register(ret_value.value, "a0");
  int size = env.get_stack_size();
  if (size < 2048) {
    out << "  addi sp, sp, " + std::to_string(size) + "\n";
  } else {
    out << "  li t0, " + std::to_string(size) + "\n";
    out << "  add sp, sp, t0\n";
  }
  out << "  ret\n";
}

void RISCV_Builder::raw_visit(const koopa_raw_binary_t &b_value, int addr) {
  std::string rd = "t0";
  std::string rs1 = "t0";
  std::string rs2 = "t1";
  load_register(b_value.lhs, rs1);
  load_register(b_value.rhs, rs2);
  switch (b_value.op) {
  case KOOPA_RBO_ADD:
    out << "  add " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_SUB:
    out << "  sub " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_MUL:
    out << "  mul " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_DIV:
    out << "  div " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_MOD:
    out << "  rem " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_AND:
    out << "  and " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_OR:
    out << "  or " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_EQ:
    out << "  xor " + rd + ", " + rs1 + ", " + rs2 + "\n";
    out << "  seqz " + rd + ", " + rd + "\n";
    break;
  case KOOPA_RBO_NOT_EQ:
    out << "  xor " + rd + ", " + rs1 + ", " + rs2 + "\n";
    out << "  snez " + rd + ", " + rd + "\n";
    break;
  case KOOPA_RBO_GT:
    out << "  sgt " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_LT:
    out << "  slt " + rd + ", " + rs1 + ", " + rs2 + "\n";
    break;
  case KOOPA_RBO_GE:
    out << "  slt " + rd + ", " + rs1 + ", " + rs2 + "\n";
    out << "  seqz " + rd + ", " + rd + "\n";
    break;
  case KOOPA_RBO_LE:
    out << "  sgt " + rd + ", " + rs1 + ", " + rs2 + "\n";
    out << "  seqz " + rd + ", " + rd + "\n";
    break;
  default:
    break;
  }
  out << "  sw " + rd + ", " + std::to_string(addr) + "(sp)\n";
}

void RISCV_Builder::raw_visit(const koopa_raw_store_t &s_value) {
  std::string rs1 = "t0";
  load_register(s_value.value, rs1);
  store_stack(s_value.dest, rs1);
}

void RISCV_Builder::raw_visit(const koopa_raw_load_t &l_value, int addr) {
  std::string rs1 = "t0";
  load_register(l_value.src, rs1);
  out << "  sw " + rs1 + ", " + std::to_string(addr) + "(sp)\n";
}

void RISCV_Builder::build(koopa_raw_program_t raw) {
  raw_visit(raw);
  out.close();
}