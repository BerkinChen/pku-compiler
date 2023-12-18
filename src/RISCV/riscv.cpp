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
  for (int i = 0; i < 8; ++i) {
    register_state_map["a" + std::to_string(i)] = UNUSED;
  }
  for (int i = 0; i < 7; ++i) {
    register_state_map["t" + std::to_string(i)] = UNUSED;
  }
  register_alloc_map.clear();
  addr_map.clear();
}

void RISCV_Builder::Env::state_free(std::string reg) {
  register_state_map[reg] = UNUSED;
}

std::string RISCV_Builder::Env::register_check(koopa_raw_value_t raw) {
  if (register_alloc_map.find(raw) != register_alloc_map.end()) {
    if (register_state_map[register_alloc_map[raw]] == USED) {
      return register_alloc_map[raw];
    }
  }
  return "";
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

std::string RISCV_Builder::Env::get_register(koopa_raw_value_t raw) {
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
  // 随机分配一个寄存器
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 14);
  int t = dis(gen);
  if (t < 7) {
    std::string ret = "t" + std::to_string(t);
    register_state_map["t" + std::to_string(t)] = USED;
    // 清除原来的映射
    for (auto it = register_alloc_map.begin(); it != register_alloc_map.end();
         ++it) {
      if (it->second == "t" + std::to_string(t)) {
        int addr = get_addr(it->first);
        if (addr != -1) {
          ret += "  sw " + it->second + ", " + std::to_string(addr) + "(sp)\n";
        } else {
          assert(false);
        }
        register_alloc_map.erase(it);
        break;
      }
    }
    register_alloc_map[raw] = "t" + std::to_string(t);
    return ret;
  } else {
    std::string ret = "a" + std::to_string(t - 7);
    register_state_map["a" + std::to_string(t - 7)] = USED;
    // 清除原来的映射
    for (auto it = register_alloc_map.begin(); it != register_alloc_map.end();
         ++it) {
      if (it->second == "a" + std::to_string(t - 7)) {
        int addr = get_addr(it->first);
        if (addr != -1) {
          ret += "  sw " + it->second + ", " + std::to_string(addr) + "(sp)\n";
        } else {
          assert(false);
        }
        register_alloc_map.erase(it);
        break;
      }
    }
    register_alloc_map[raw] = "a" + std::to_string(t - 7);
    return ret;
  }
  return "";
}

void RISCV_Builder::load_register(koopa_raw_value_t value, std::string reg) {
  if (value->kind.tag == KOOPA_RVT_LOAD) {
    value = value->kind.data.load.src;
  }
  std::string rs1 = env.register_check(value);
  if (rs1 != "") {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
      out << "  li " + reg + ", " +
                 std::to_string(value->kind.data.integer.value) + "\n";
    } else if (rs1 != reg) {
      out << "  mv " + reg + ", " + rs1 + "\n";
    }
  } else {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
      out << "  li " + reg + ", " +
                 std::to_string(value->kind.data.integer.value) + "\n";
    } else {
      std::cout << value->kind.tag << std::endl;
      int addr = env.get_addr(value);
      if (addr != -1) {
        out << "  lw " + reg + ", " + std::to_string(addr) + "(sp)\n";
      } else {
        assert(false);
      }
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
    raw_visit(kind.data.load);
    break;
  case KOOPA_RVT_STORE:
    raw_visit(kind.data.store);
    break;
  case KOOPA_RVT_BINARY:
    raw_visit(kind.data.binary);
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

void RISCV_Builder::raw_visit(const koopa_raw_binary_t &b_value) {
  auto b_data = (koopa_raw_value_data_t *)b_value.lhs->used_by.buffer[0];
  std::string rd = env.get_register(b_data);
  if (rd.size() > 2) {
    out << rd.substr(2, rd.size() - 2);
    rd = rd.substr(0, 2);
  }
  std::string rs1;
  if (b_value.lhs->kind.tag == KOOPA_RVT_INTEGER &&
      b_value.lhs->kind.data.integer.value == 0) {
    rs1 = "x0";
  } else {
    rs1 = env.get_register(b_value.lhs);
    if (rs1.size() > 2) {
      out << rs1.substr(2, rs1.size() - 2);
      rs1 = rs1.substr(0, 2);
    }
    if (rs1 != "") {
      load_register(b_value.lhs, rs1);
    }
  }
  std::string rs2;
  if (b_value.rhs->kind.tag == KOOPA_RVT_INTEGER &&
      b_value.rhs->kind.data.integer.value == 0) {
    rs2 = "x0";
  } else {
    rs2 = env.get_register(b_value.rhs);
    if (rs2.size() > 2) {
      out << rs2.substr(2, rs2.size() - 2);
      rs2 = rs2.substr(0, 2);
    }
    if (rs2 != "") {
      load_register(b_value.rhs, rs2);
    }
  }
  if (rs1 != "" || rs1 != "x0") {
    env.state_free(rs1);
  }
  if (rs2 != "" || rs2 != "x0") {
    env.state_free(rs2);
  }
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
}

void RISCV_Builder::raw_visit(const koopa_raw_store_t &s_value) {
  std::string rs1 = env.get_register(s_value.value);
  if (rs1.size() > 2) {
    out << rs1.substr(2, rs1.size() - 2);
    rs1 = rs1.substr(0, 2);
  }
  load_register(s_value.value, rs1);
  std::string rs2 = env.get_register(s_value.dest);
  if (rs2.size() > 2) {
    out << rs2.substr(2, rs2.size() - 2);
    rs2 = rs2.substr(0, 2);
  }
  load_register(s_value.value, rs2);
}

void RISCV_Builder::raw_visit(const koopa_raw_load_t &l_value) {
  std::string rs1 = env.get_register(l_value.src);
  if (rs1.size() > 2) {
    out << rs1.substr(2, rs1.size() - 2);
    rs1 = rs1.substr(0, 2);
  }
  load_register(l_value.src, rs1);
}

void RISCV_Builder::build(koopa_raw_program_t raw) {
  raw_visit(raw);
  out.close();
}