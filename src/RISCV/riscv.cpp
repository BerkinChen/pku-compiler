#include "riscv.h"

#include <cassert>
#include <cstring>
#include <iostream>

void RISCV_Builder::Env::state_init() {
  for (int i = 0; i < 8; ++i) {
    register_state_map["a" + std::to_string(i)] = UNUSED;
  }
  for (int i = 0; i < 7; ++i) {
    register_state_map["t" + std::to_string(i)] = UNUSED;
  }
}

void RISCV_Builder::Env::state_free(std::string reg) {
  register_state_map[reg] = UNUSED;
}

std::string RISCV_Builder::Env::register_check(koopa_raw_value_t raw) {
  if (register_alloc_map.find(raw) != register_alloc_map.end()) {
    return register_alloc_map[raw];
  } else {
    return "Null";
  }
}

std::string RISCV_Builder::Env::register_alloc(koopa_raw_value_t raw) {
  if (register_alloc_map.find(raw) != register_alloc_map.end()) {
    return register_alloc_map[raw];
  }
  for (int i = 0; i < 7; ++i) {
    if (register_state_map["t" + std::to_string(i)] == UNUSED) {
      register_state_map["t" + std::to_string(i)] = USED;
      register_alloc_map[raw] = "t" + std::to_string(i);
      return "t" + std::to_string(i);
    }
  }
  for (int i = 0; i < 8; ++i) {
    if (register_state_map["a" + std::to_string(i)] == UNUSED) {
      register_state_map["a" + std::to_string(i)] = USED;
      register_alloc_map[raw] = "a" + std::to_string(i);
      return "a" + std::to_string(i);
    }
  }
  return "Null";
}

std::string RISCV_Builder::load_register(koopa_raw_value_t value,
                                         std::string reg) {
  std::string ret = "";
  std::string rs1 = env.register_check(value);
  if (rs1 != "Null") {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
      ret += "  li " + reg + ", " +
             std::to_string(value->kind.data.integer.value) + "\n";
    } else if (rs1 != reg) {
      ret += "  mv " + reg + ", " + rs1 + "\n";
    }
  } else {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
      ret += "  li " + reg + ", " +
             std::to_string(value->kind.data.integer.value) + "\n";
    }
  }
  return ret;
}

std::string RISCV_Builder::raw_visit(const koopa_raw_program_t &raw) {
  std::string ret = "  .text\n  .globl main\n";
  // TODO: values
  ret += raw_visit(raw.funcs);
  return ret;
}

std::string RISCV_Builder::raw_visit(const koopa_raw_slice_t &slice) {
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

std::string RISCV_Builder::raw_visit(const koopa_raw_function_t &func) {
  // TODO: params
  // TODO: used_by
  char *name = new char[strlen(func->name) - 1];
  memcpy(name, func->name + 1, strlen(func->name) - 1);
  std::string ret = std::string(name) + ":\n";
  ret += raw_visit(func->bbs);
  return ret;
}

std::string RISCV_Builder::raw_visit(const koopa_raw_basic_block_t &bb) {
  // TODO: used_by
  // TODO: params
  std::string ret = "";
  ret += raw_visit(bb->insts);
  return ret;
}

std::string RISCV_Builder::raw_visit(const koopa_raw_value_t &value) {
  std::string ret = "";
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      ret += raw_visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // ret += raw_visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      ret += raw_visit(kind.data.binary);
      break;
    default:
      assert(false);
  }
  return ret;
}

std::string RISCV_Builder::raw_visit(const koopa_raw_return_t &ret_value) {
  std::string ret = "";
  ret += load_register(ret_value.value, "a0");
  ret += "  ret\n";
  return ret;
}

std::string RISCV_Builder::raw_visit(const koopa_raw_binary_t &b_value) {
  std::string ret = "";
  auto b_data = (koopa_raw_value_data_t *)b_value.lhs->used_by.buffer[0];
  std::string rd = env.register_alloc(b_data);
  std::string rs1;
  if (b_value.lhs->kind.tag == KOOPA_RVT_INTEGER &&
      b_value.lhs->kind.data.integer.value == 0) {
    rs1 = "x0";
  } else {
    rs1 = env.register_alloc(b_value.lhs);
    if (rs1 != "Null") {
      ret += load_register(b_value.lhs, rs1);
    }
  }
  std::string rs2;
  if (b_value.rhs->kind.tag == KOOPA_RVT_INTEGER &&
      b_value.rhs->kind.data.integer.value == 0) {
    rs2 = "x0";
  } else {
    rs2 = env.register_alloc(b_value.rhs);
    if (rs2 != "Null") {
      ret += load_register(b_value.rhs, rs2);
    }
  }
  if (rs1 != "Null" || rs1 != "x0") {
    env.state_free(rs1);
  }
  if (rs2 != "Null" || rs2 != "x0") {
    env.state_free(rs2);
  }
  switch (b_value.op) {
    case KOOPA_RBO_ADD:
      ret += "  add " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_SUB:
      ret += "  sub " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_MUL:
      ret += "  mul " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_DIV:
      ret += "  div " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_MOD:
      ret += "  rem " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_AND:
      ret += "  and " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_OR:
      ret += "  or " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_EQ:
      ret += "  xor " + rd + ", " + rs1 + ", " + rs2 + "\n";
      ret += "  seqz " + rd + ", " + rd + "\n";
      break;
    case KOOPA_RBO_NOT_EQ:
      ret += "  xor " + rd + ", " + rs1 + ", " + rs2 + "\n";
      ret += "  snez " + rd + ", " + rd + "\n";
      break;
    case KOOPA_RBO_GT:
      ret += "  sgt " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_LT:
      ret += "  slt " + rd + ", " + rs1 + ", " + rs2 + "\n";
      break;
    case KOOPA_RBO_GE:
      ret += "  slt " + rd + ", " + rs1 + ", " + rs2 + "\n";
      ret += "  seqz " + rd + ", " + rd + "\n";
      break;
    case KOOPA_RBO_LE:
      ret += "  sgt " + rd + ", " + rs1 + ", " + rs2 + "\n";
      ret += "  seqz " + rd + ", " + rd + "\n";
      break;
    default:
      break;
  }
  return ret;
}

void RISCV_Builder::build(koopa_raw_program_t raw, const char *path) {
  env.state_init();
  std::string ret = raw_visit(raw);
  std::ofstream out(path);
  out << ret;
  out.close();
}