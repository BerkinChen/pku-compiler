#include "riscv.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <random>

int RISCV_Builder::func_size(koopa_raw_function_t func, bool &call) {
  int size = 0;
  int max_arg = 0;
  for (size_t i = 0; i < func->bbs.len; ++i) {
    auto ptr = func->bbs.buffer[i];
    size +=
        bb_size(reinterpret_cast<koopa_raw_basic_block_t>(ptr), call, max_arg);
  }
  size += max_arg * 4;
  size += func->params.len * 4;
  size += call ? 4 : 0;
  return size;
}

int RISCV_Builder::bb_size(koopa_raw_basic_block_t bb, bool &call,
                           int &max_arg) {
  int size = 0;
  for (size_t i = 0; i < bb->insts.len; ++i) {
    auto ptr = bb->insts.buffer[i];
    if (((koopa_raw_value_t)ptr)->kind.tag == KOOPA_RVT_CALL) {
      call = true;
      max_arg = ((koopa_raw_value_t)ptr)->kind.data.call.args.len > max_arg
                    ? ((koopa_raw_value_t)ptr)->kind.data.call.args.len
                    : max_arg;
    }
    size += inst_size(reinterpret_cast<koopa_raw_value_t>(ptr));
  }
  return size;
}

int RISCV_Builder::inst_size(koopa_raw_value_t inst) {
  if (inst->kind.tag == KOOPA_RVT_ALLOC) {
    return type_size(inst->ty->data.pointer.base);
  }
  return type_size(inst->ty);
}

int RISCV_Builder::type_size(koopa_raw_type_t ty) {
  switch (ty->tag) {
  case KOOPA_RTT_INT32:
    return 4;
  case KOOPA_RTT_UNIT:
    return 0;
  case KOOPA_RTT_POINTER:
    return 4;
  case KOOPA_RTT_ARRAY:
    return array_size(ty);
  default:
    return 0;
  }
}

int RISCV_Builder::array_size(koopa_raw_type_t ty) {
  if (ty->tag == KOOPA_RTT_ARRAY) {
    return array_size(ty->data.array.base) * ty->data.array.len;
  } else {
    return 4;
  }
}

void RISCV_Builder::Env::init(int size, bool call) {
  stack_size = size;
  is_call = call;
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
      if (addr < 2048 && addr >= -2048) {
        out << "  lw " + reg + ", " + std::to_string(addr) + "(sp)\n";
      } else {
        out << "  li t3, " + std::to_string(addr) + "\n";
        out << "  add t3, sp, t3\n";
        out << "  lw " + reg + ", 0(t3)\n";
      }
    } else {
      assert(false);
    }
  }
}

void RISCV_Builder::store_stack(int addr, std::string reg) {
  if (addr != -1) {
    if (addr < 2048 && addr >= -2048) {
      out << "  sw " + reg + ", " + std::to_string(addr) + "(sp)\n";
    } else {
      out << "  li t3, " + std::to_string(addr) + "\n";
      out << "  add t3, sp, t3\n";
      out << "  sw " + reg + ", 0(t3)\n";
    }
  } else {
    assert(false);
  }
}

void RISCV_Builder::raw_visit(const koopa_raw_program_t &raw) {
  if (raw.values.len != 0) {
    out << "  .data\n";
    raw_visit(raw.values);
  }
  out << "\n  .text\n";
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
  if (func->bbs.len == 0)
    return;
  out << "\n  .globl  " << std::string(func->name + 1) + "\n";
  out << std::string(func->name + 1) + ":\n";
  bool call = false;
  int size = func_size(func, call);
  size = (size + 15) / 16 * 16;
  if (size < 2048 && size >= -2048) {
    out << "  addi sp, sp, -" + std::to_string(size) + "\n";
  } else if (size > 0) {
    out << "  li t0, -" + std::to_string(size) + "\n";
    out << "  add sp, sp, t0\n";
  }
  if (call) {
    if (size - 4 < 2048 && size - 4 >= -2048)
      out << "  sw ra, " + std::to_string(size - 4) + "(sp)\n";
    else {
      out << "  li t0, " + std::to_string(size - 4) + "\n";
      out << "  add t0, sp, t0\n";
      out << "  sw ra, 0(t0)\n";
    }
  }
  env.init(size, call);
  env.stack_size -= call ? 4 : 0;
  env.cur_size += (func->params.len > 8 ? func->params.len - 8 : 0) * 4;
  raw_visit(func->bbs);
}

void RISCV_Builder::raw_visit(const koopa_raw_basic_block_t &bb) {
  std::string name = bb->name + 1;
  if (name != "entry")
    out << name << ":\n";
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
  case KOOPA_RVT_BRANCH:
    raw_visit(kind.data.branch);
    break;
  case KOOPA_RVT_JUMP:
    raw_visit(kind.data.jump);
    break;
  case KOOPA_RVT_CALL:
    raw_visit(kind.data.call, addr);
    break;
  case KOOPA_RVT_GLOBAL_ALLOC:
    global_alloc(value);
    break;
  case KOOPA_RVT_GET_ELEM_PTR:
    raw_visit(kind.data.get_elem_ptr, addr);
    break;
  case KOOPA_RVT_GET_PTR:
    raw_visit(kind.data.get_ptr, addr);
    break;
  default:
    assert(false);
  }
}

void RISCV_Builder::raw_visit(const koopa_raw_return_t &ret_value) {
  if (ret_value.value != nullptr) {
    load_register(ret_value.value, "a0");
  }
  if (env.is_call) {
    if (env.stack_size < 2048 && env.stack_size >= -2048)
      out << "  lw ra, " + std::to_string(env.stack_size) + "(sp)\n";
    else {
      out << "  li t0, " + std::to_string(env.stack_size) + "\n";
      out << "  add t0, sp, t0\n";
      out << "  lw ra, 0(t0)\n";
    }
  }
  int size = env.stack_size;
  size += env.is_call ? 4 : 0;
  if (size < 2048 && size >= -2048) {
    out << "  addi sp, sp, " + std::to_string(size) + "\n";
  } else if (size > 0) {
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
  store_stack(addr, rd);
}

void RISCV_Builder::raw_visit(const koopa_raw_store_t &s_value) {
  if (s_value.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    out << "  la t1, " + std::string(s_value.dest->name + 1) + "\n";
    load_register(s_value.value, "t0");
    out << "  sw t0, 0(t1)\n";
  } else if (s_value.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
             s_value.dest->kind.tag == KOOPA_RVT_GET_PTR) {
    load_register(s_value.dest, "t1");
    load_register(s_value.value, "t0");
    out << "  sw t0, 0(t1)\n";
  } else {
    int addr = env.get_addr(s_value.dest);
    if (s_value.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
      if (s_value.value->kind.data.func_arg_ref.index < 8) {
        store_stack(
            addr,
            "a" + std::to_string(s_value.value->kind.data.func_arg_ref.index));
      } else {
        if ((s_value.value->kind.data.func_arg_ref.index - 8) * 4 < 2048 &&
            (s_value.value->kind.data.func_arg_ref.index - 8) * 4 >= -2048)
          out << "  lw t0, "
              << (s_value.value->kind.data.func_arg_ref.index - 8) * 4
              << "(sp)\n";
          else {
            out << "  li t3, "
                << (s_value.value->kind.data.func_arg_ref.index - 8) * 4
                << "\n";
            out << "  add t3, sp, t3\n";
            out << "  lw t0, 0(t3)\n";
          }
        store_stack(addr, "t0");
      }
    } else {
      load_register(s_value.value, "t0");
      store_stack(addr, "t0");
    }
  }
}

void RISCV_Builder::raw_visit(const koopa_raw_load_t &l_value, int addr) {
  std::string rs1 = "t0";
  if (l_value.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    out << "  la " + rs1 + ", " + std::string(l_value.src->name + 1) + "\n";
    out << "  lw " + rs1 + ", 0(" + rs1 + ")\n";
  } else if (l_value.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
             l_value.src->kind.tag == KOOPA_RVT_GET_PTR) {
    load_register(l_value.src, rs1);
    out << "  lw " + rs1 + ", 0(" + rs1 + ")\n";
  } else {
    load_register(l_value.src, rs1);
  }
  store_stack(addr, rs1);
}

void RISCV_Builder::raw_visit(const koopa_raw_branch_t &b_value) {
  load_register(b_value.cond, "t0");
  out << "  bnez t0, " + std::string(b_value.true_bb->name + 1) + "\n";
  out << "  j " + std::string(b_value.false_bb->name + 1) + "\n";
}

void RISCV_Builder::raw_visit(const koopa_raw_jump_t &j_value) {
  out << "  j " + std::string(j_value.target->name + 1) + "\n";
}

void RISCV_Builder::raw_visit(const koopa_raw_call_t &c_value, int addr) {
  for (int i = 0; i < c_value.args.len && i < 8; ++i) {
    auto ptr = c_value.args.buffer[i];
    load_register(reinterpret_cast<koopa_raw_value_t>(ptr),
                  "a" + std::to_string(i));
  }
  bool call = false;
  int size = func_size(c_value.callee, call);
  size = (size + 15) / 16 * 16;
  for (int i = 8; i < c_value.args.len; ++i) {
    auto ptr = c_value.args.buffer[i];
    load_register(reinterpret_cast<koopa_raw_value_t>(ptr), "t0");
    store_stack((i - 8) * 4 - size, "t0");
  }
  out << "  call " + std::string(c_value.callee->name + 1) + "\n";
  if (addr != -1)
    store_stack(addr, "a0");
}

void RISCV_Builder::global_alloc(const koopa_raw_value_t &g_value) {
  out << "\n  .global " + std::string(g_value->name + 1) + "\n";
  out << std::string(g_value->name + 1) + ":\n";
  if (g_value->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_INTEGER)
    out << "  .word " +
               std::to_string(g_value->kind.data.global_alloc.init->kind.data
                                  .integer.value) +
               "\n";
  if (g_value->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT) {
    out << "  .zero " +
               std::to_string(type_size(g_value->ty->data.pointer.base)) + "\n";
  }
  if (g_value->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_AGGREGATE) {
    // TODO: aggregate
    raw_visit(g_value->kind.data.global_alloc.init->kind.data.aggregate
              );
  }
}

void RISCV_Builder::raw_visit(const koopa_raw_aggregate_t &agg_value
                              ) {
  for (size_t i = 0; i < agg_value.elems.len; ++i) {
    auto ptr = agg_value.elems.buffer[i];
    koopa_raw_value_t value = reinterpret_cast<koopa_raw_value_t>(ptr);
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
      out << "  .word " +
                 std::to_string(value->kind.data.integer.value) + "\n";
    }
    else if (value->kind.tag == KOOPA_RVT_AGGREGATE) {
      raw_visit(value->kind.data.aggregate);
    }
    else 
      assert(false);
  }
}

void RISCV_Builder::raw_visit(const koopa_raw_get_elem_ptr_t &gep_value,
                              int addr) {
  if (gep_value.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    out << "  la t0, " + std::string(gep_value.src->name + 1) + "\n";
  } else {
    int src_addr = env.get_addr(gep_value.src);
    if (src_addr != -1) {
      if (src_addr < 2048 && src_addr >= -2048)
        out << "  addi t0, sp, " + std::to_string(src_addr) + "\n";
      else {
        out << "  li t3, " + std::to_string(src_addr) + "\n";
        out << "  add t0, sp, t3\n";
      }
    } else {
      assert(false);
    }
    if (gep_value.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
        gep_value.src->kind.tag == KOOPA_RVT_GET_PTR) {
      out << "  lw t0, 0(t0)\n";
    }
  }
  load_register(gep_value.index, "t1");
  int size = array_size(gep_value.src->ty->data.pointer.base->data.array.base);
  out << "  li t2, " + std::to_string(size) + "\n";
  out << "  mul t1, t1, t2\n";
  out << "  add t0, t0, t1\n";
  store_stack(addr, "t0");
}
void RISCV_Builder::raw_visit(const koopa_raw_get_ptr_t &gp_value, int addr) {
  if (gp_value.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    out << "  la t0, " + std::string(gp_value.src->name + 1) + "\n";
  } else {
    int src_addr = env.get_addr(gp_value.src);
    if (src_addr != -1) {
      if (src_addr < 2048 && src_addr >= -2048)
        out << "  addi t0, sp, " + std::to_string(src_addr) + "\n";
      else {
        out << "  li t3, " + std::to_string(src_addr) + "\n";
        out << "  add t0, sp, t3\n";
      }
    } else {
      assert(false);
    }
    if (gp_value.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
        gp_value.src->kind.tag == KOOPA_RVT_GET_PTR) {
      out << "  lw t0, 0(t0)\n";
    }
  }
  load_register(gp_value.index, "t1");
  int size = array_size(gp_value.src->ty->data.pointer.base);
  out << "  li t2, " + std::to_string(size) + "\n";
  out << "  mul t1, t1, t2\n";
  out << "  add t0, t0, t1\n";
  store_stack(addr, "t0");
}

void RISCV_Builder::build(koopa_raw_program_t raw) {
  raw_visit(raw);
  out.close();
}