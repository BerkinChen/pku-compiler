#include "utils.h"
#include <assert.h>
#include <cstring>
#include <iostream>
#include <fstream>

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

bool BlockManager::checkBlock() {
  if (block_list_vector->size() > 0 && tmp_inst_list.size() > 0) {
    for (size_t i = 0; i < tmp_inst_list.size(); i++) {
      koopa_raw_value_t inst = (koopa_raw_value_t)tmp_inst_list[i];
      if (inst->kind.tag == KOOPA_RVT_RETURN ||
          inst->kind.tag == KOOPA_RVT_JUMP ||
          inst->kind.tag == KOOPA_RVT_BRANCH) {
        return true;
      }
    }
  }
  return false;
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
    else {
      block_list_vector->pop_back();
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

koopa_raw_type_t type_kind(koopa_raw_type_tag_t tag) {
  koopa_raw_type_kind_t *ret = new koopa_raw_type_kind_t();
  ret->tag = tag;
  return (koopa_raw_type_t)ret;
}

koopa_raw_type_t pointer_type_kind(koopa_raw_type_tag_t tag) {
  koopa_raw_type_kind_t *ret = new koopa_raw_type_kind_t();
  ret->tag = KOOPA_RTT_POINTER;
  ret->data.pointer.base = type_kind(tag);
  return (koopa_raw_type_t)ret;
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