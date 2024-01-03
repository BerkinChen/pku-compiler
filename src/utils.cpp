#include "utils.h"
#include <assert.h>
#include <cstring>
#include <fstream>
#include <iostream>

void SymbolList::addSymbol(std::string symbol, Value value) {
  symbol_list_vector.back()[symbol] = value;
}

Value SymbolList::getSymbol(std::string symbol) {
  for (auto it = symbol_list_vector.rbegin(); it != symbol_list_vector.rend();
       ++it) {
    if (it->find(symbol) != it->end()) {
      return it->at(symbol);
    }
  }
  assert(false);
}

void SymbolList::newScope() {
  symbol_list_vector.push_back(std::map<std::string, Value>());
}

void SymbolList::delScope() { symbol_list_vector.pop_back(); }

void BlockManager::init(std::vector<const void *> *block_list_vector) {
  this->block_list_vector = block_list_vector;
}

void BlockManager::delBlock() {
  if (block_list_vector->size() > 0) {
    if (tmp_inst_list.size() > 0) {
      koopa_raw_basic_block_data_t *last =
          (koopa_raw_basic_block_data_t *)block_list_vector->back();
      for (size_t i = 0; i < tmp_inst_list.size(); i++) {
        koopa_raw_value_t inst = (koopa_raw_value_t)tmp_inst_list[i];
        if (inst->kind.tag == KOOPA_RVT_RETURN ||
            inst->kind.tag == KOOPA_RVT_JUMP ||
            inst->kind.tag == KOOPA_RVT_BRANCH) {
          tmp_inst_list.resize(i + 1);
          break;
        }
      }
      if (!last->insts.buffer) {
        last->insts = slice(tmp_inst_list, KOOPA_RSIK_VALUE);
      }
      tmp_inst_list.clear();
    }
  }
}

void BlockManager::newBlock(koopa_raw_basic_block_data_t *basic_block) {
  delBlock();
  basic_block->insts.buffer = nullptr;
  basic_block->insts.len = 0;
  block_list_vector->push_back(basic_block);
}

void BlockManager::addInst(const void *inst) { tmp_inst_list.push_back(inst); }

void BlockManager::delUnreachableBlock() {
  std::map<koopa_raw_basic_block_t, bool> reachable;
  if (block_list_vector->size() > 0) {
    for (size_t i = 0; i < block_list_vector->size(); i++) {
      auto block = (koopa_raw_basic_block_data_t *)block_list_vector->at(i);
      reachable[block] = false;
      if (i == 0)
        reachable[block] = true;
    }
    for (size_t i = 0; i < block_list_vector->size(); i++) {
      auto block = (koopa_raw_basic_block_data_t *)block_list_vector->at(i);
      for (size_t j = 0; j < block->insts.len; j++) {
        auto inst = (koopa_raw_value_t)block->insts.buffer[j];
        if (inst->kind.tag == KOOPA_RVT_JUMP) {
          reachable[inst->kind.data.jump.target] = true;
        } else if (inst->kind.tag == KOOPA_RVT_BRANCH) {
          reachable[inst->kind.data.branch.true_bb] = true;
          reachable[inst->kind.data.branch.false_bb] = true;
        }
      }
    }
    for (size_t i = 0; i < block_list_vector->size(); i++) {
      auto block = (koopa_raw_basic_block_data_t *)block_list_vector->at(i);
      if (!reachable[block]) {
        block_list_vector->erase(block_list_vector->begin() + i);
        i--;
      }
    }
  }
}

bool BlockManager::checkBlock() {
  if (block_list_vector->size() > 0) {
    auto block = (koopa_raw_basic_block_data_t *)block_list_vector->back();
    if (block->insts.buffer) {
      for (size_t i = 0; i < block->insts.len; i++) {
        auto inst = (koopa_raw_value_t)block->insts.buffer[i];
        if (inst->kind.tag == KOOPA_RVT_RETURN) {
          return true;
        }
      }
    }
  }
  return false;
}

void LoopManager::addWhile(koopa_raw_basic_block_t head,
                           koopa_raw_basic_block_t tail) {
  while_list.push_back(While(head, tail));
}

void LoopManager::delWhile() { while_list.pop_back(); }

koopa_raw_basic_block_t LoopManager::getHead() {
  if (while_list.size() == 0) {
    return nullptr;
  } else {
    return while_list.back().head;
  }
}

koopa_raw_basic_block_t LoopManager::getTail() {
  if (while_list.size() == 0) {
    return nullptr;
  } else {
    return while_list.back().tail;
  }
}

koopa_raw_slice_t slice(koopa_raw_slice_item_kind_t kind) {
  koopa_raw_slice_t ret;
  ret.kind = kind;
  ret.buffer = nullptr;
  ret.len = 0;
  return ret;
}

koopa_raw_slice_t slice(std::vector<const void *> &vec,
                        koopa_raw_slice_item_kind_t kind) {
  koopa_raw_slice_t ret;
  ret.kind = kind;
  ret.buffer = new const void *[vec.size()];
  std::copy(vec.begin(), vec.end(), ret.buffer);
  ret.len = vec.size();
  return ret;
}

koopa_raw_slice_t slice(const void *data, koopa_raw_slice_item_kind_t kind) {
  koopa_raw_slice_t ret;
  ret.kind = kind;
  ret.buffer = new const void *[1];
  ret.buffer[0] = data;
  ret.len = 1;
  return ret;
}

koopa_raw_type_kind *type_kind(koopa_raw_type_tag_t tag) {
  koopa_raw_type_kind *ret = new koopa_raw_type_kind();
  ret->tag = tag;
  return ret;
}

koopa_raw_type_kind *pointer_type_kind(koopa_raw_type_tag_t tag) {
  koopa_raw_type_kind *ret = new koopa_raw_type_kind();
  ret->tag = KOOPA_RTT_POINTER;
  ret->data.pointer.base = type_kind(tag);
  return ret;
}

koopa_raw_type_kind *array_type_kind(koopa_raw_type_tag_t tag, size_t size) {
  koopa_raw_type_kind *ret = new koopa_raw_type_kind();
  ret->tag = KOOPA_RTT_ARRAY;
  ret->data.array.base = type_kind(tag);
  ret->data.array.len = size;
  return ret;
}

koopa_raw_value_data *jump_value(koopa_raw_basic_block_t tar) {
  koopa_raw_value_data *jump = new koopa_raw_value_data_t();
  jump->ty = type_kind(KOOPA_RTT_UNIT);
  jump->name = nullptr;
  jump->used_by = slice(KOOPA_RSIK_VALUE);
  jump->kind.tag = KOOPA_RVT_JUMP;
  jump->kind.data.jump.args = slice(KOOPA_RSIK_VALUE);
  jump->kind.data.jump.target = tar;
  return jump;
}

koopa_raw_value_data *ret_value(koopa_raw_type_tag_t tag) {
  koopa_raw_value_data *ret = new koopa_raw_value_data_t();
  ret->ty = type_kind(KOOPA_RTT_UNIT);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_RETURN;
  if (tag == KOOPA_RTT_UNIT) {
    ret->kind.data.ret.value = nullptr;
  } else {
    koopa_raw_value_data *zero = new koopa_raw_value_data_t();
    zero->ty = type_kind(KOOPA_RTT_INT32);
    zero->name = nullptr;
    zero->used_by = slice(KOOPA_RSIK_VALUE);
    zero->kind.tag = KOOPA_RVT_INTEGER;
    zero->kind.data.integer.value = 0;
    ret->kind.data.ret.value = zero;
  }
  return ret;
}

koopa_raw_value_data *zero_init(koopa_raw_type_kind *type) {
  koopa_raw_value_data *ret = new koopa_raw_value_data_t();
  ret->ty = type;
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_ZERO_INIT;
  return ret;
}