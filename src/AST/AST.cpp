#include "AST/AST.h"
#include "AST.h"
#include <iostream>

// CompUnitAST
CompUnitAST::CompUnitAST(
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &def_vec)
    : def_vec(std::move(def_vec)) {}

void CompUnitAST::load_lib_func(std::vector<const void *> &lib_func_vec) const {
  koopa_raw_function_data_t *func;
  koopa_raw_type_kind_t *ty;
  std::vector<const void *> params;

  func = new koopa_raw_function_data_t();
  ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  ty->data.function.params = slice(KOOPA_RSIK_TYPE);
  ty->data.function.ret = type_kind(KOOPA_RTT_INT32);
  func->ty = ty;
  func->name = "@getint";
  func->params = slice(KOOPA_RSIK_VALUE);
  func->bbs = slice(KOOPA_RSIK_BASIC_BLOCK);
  symbol_list.addSymbol(func->name + 1, Value(ValueType::Func, func));
  lib_func_vec.push_back(func);

  func = new koopa_raw_function_data_t();
  ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  ty->data.function.params = slice(KOOPA_RSIK_TYPE);
  ty->data.function.ret = type_kind(KOOPA_RTT_INT32);
  func->ty = ty;
  func->name = "@getch";
  func->params = slice(KOOPA_RSIK_VALUE);
  func->bbs = slice(KOOPA_RSIK_BASIC_BLOCK);
  symbol_list.addSymbol(func->name + 1, Value(ValueType::Func, func));
  lib_func_vec.push_back(func);

  func = new koopa_raw_function_data_t();
  ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  params.clear();
  params.push_back(pointer_type_kind(KOOPA_RTT_INT32));
  ty->data.function.params = slice(params, KOOPA_RSIK_TYPE);
  ty->data.function.ret = type_kind(KOOPA_RTT_INT32);
  func->ty = ty;
  func->name = "@getarray";
  func->params = slice(KOOPA_RSIK_VALUE);
  func->bbs = slice(KOOPA_RSIK_BASIC_BLOCK);
  symbol_list.addSymbol(func->name + 1, Value(ValueType::Func, func));
  lib_func_vec.push_back(func);

  func = new koopa_raw_function_data_t();
  ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  params.clear();
  params.push_back(type_kind(KOOPA_RTT_INT32));
  ty->data.function.params = slice(params, KOOPA_RSIK_TYPE);
  ty->data.function.ret = type_kind(KOOPA_RTT_UNIT);
  func->ty = ty;
  func->name = "@putint";
  func->params = slice(KOOPA_RSIK_VALUE);
  func->bbs = slice(KOOPA_RSIK_BASIC_BLOCK);
  symbol_list.addSymbol(func->name + 1, Value(ValueType::Func, func));
  lib_func_vec.push_back(func);

  func = new koopa_raw_function_data_t();
  ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  params.clear();
  params.push_back(type_kind(KOOPA_RTT_INT32));
  ty->data.function.params = slice(params, KOOPA_RSIK_TYPE);
  ty->data.function.ret = type_kind(KOOPA_RTT_UNIT);
  func->ty = ty;
  func->name = "@putch";
  func->params = slice(KOOPA_RSIK_VALUE);
  func->bbs = slice(KOOPA_RSIK_BASIC_BLOCK);
  symbol_list.addSymbol(func->name + 1, Value(ValueType::Func, func));
  lib_func_vec.push_back(func);

  func = new koopa_raw_function_data_t();
  ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  params.clear();
  params.push_back(type_kind(KOOPA_RTT_INT32));
  params.push_back(pointer_type_kind(KOOPA_RTT_INT32));
  ty->data.function.params = slice(params, KOOPA_RSIK_TYPE);
  ty->data.function.ret = type_kind(KOOPA_RTT_UNIT);
  func->ty = ty;
  func->name = "@putarray";
  func->params = slice(KOOPA_RSIK_VALUE);
  func->bbs = slice(KOOPA_RSIK_BASIC_BLOCK);
  symbol_list.addSymbol(func->name + 1, Value(ValueType::Func, func));
  lib_func_vec.push_back(func);

  func = new koopa_raw_function_data_t();
  ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  ty->data.function.params = slice(KOOPA_RSIK_TYPE);
  ty->data.function.ret = type_kind(KOOPA_RTT_UNIT);
  func->ty = ty;
  func->name = "@starttime";
  func->params = slice(KOOPA_RSIK_VALUE);
  func->bbs = slice(KOOPA_RSIK_BASIC_BLOCK);
  symbol_list.addSymbol(func->name + 1, Value(ValueType::Func, func));
  lib_func_vec.push_back(func);

  func = new koopa_raw_function_data_t();
  ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  ty->data.function.params = slice(KOOPA_RSIK_TYPE);
  ty->data.function.ret = type_kind(KOOPA_RTT_UNIT);
  func->ty = ty;
  func->name = "@stoptime";
  func->params = slice(KOOPA_RSIK_VALUE);
  func->bbs = slice(KOOPA_RSIK_BASIC_BLOCK);
  symbol_list.addSymbol(func->name + 1, Value(ValueType::Func, func));
  lib_func_vec.push_back(func);
}

void *CompUnitAST::to_koopa() const {
  symbol_list.newScope();
  std::vector<const void *> funcs;
  std::vector<const void *> values;
  load_lib_func(funcs);
  for (auto def = (*def_vec).rbegin(); def != (*def_vec).rend(); def++) {
    (*def)->to_koopa(funcs, values);
  }
  symbol_list.delScope();
  koopa_raw_program_t *ret = new koopa_raw_program_t();
  ret->values = slice(values, KOOPA_RSIK_VALUE);
  ret->funcs = slice(funcs, KOOPA_RSIK_FUNCTION);

  return ret;
}

// DefAST
DefAST::DefAST(std::unique_ptr<BaseAST> &def, DefType type)
    : def(std::move(def)) {
  this->type = type;
}

void *DefAST::to_koopa(std::vector<const void *> &funcs,
                       std::vector<const void *> &values) const {
  if (type == FuncDef) {
    funcs.push_back(def->to_koopa());
  } else if (type == ConstDef) {
    def->to_koopa(values);
  } else if (type == VarDef) {
    def->to_koopa(values);
  }
  return nullptr;
}

// FuncDefAST
FuncDefAST::FuncDefAST(
    std::unique_ptr<BaseAST> &func_type, const char *ident,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &param_vec,
    std::unique_ptr<BaseAST> &block)
    : func_type(std::move(func_type)), ident(ident), block(std::move(block)),
      param_vec(std::move(param_vec)) {}

void *FuncDefAST::to_koopa() const {
  koopa_raw_function_data_t *ret = new koopa_raw_function_data_t();
  symbol_list.addSymbol(ident.c_str(), Value(ValueType::Func, ret));
  koopa_raw_type_kind_t *ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  std::vector<const void *> params;
  for (int i = (*param_vec).size() - 1; i >= 0; i--) {
    params.push_back((*param_vec)[i]->to_koopa());
  }
  if (params.size() == 0) {
    ty->data.function.params = slice(KOOPA_RSIK_TYPE);
  } else {
    ty->data.function.params = slice(params, KOOPA_RSIK_TYPE);
  }
  ty->data.function.ret = (koopa_raw_type_t)func_type->to_koopa();
  ret->ty = ty;

  char *name = new char[ident.length() + 1];
  ("@" + ident).copy(name, ident.length() + 1);
  name[ident.length() + 1] = '\0';
  ret->name = name;
  params.clear();
  for (int i = (*param_vec).size() - 1; i >= 0; i--) {
    params.push_back((*param_vec)[i]->to_koopa(i));
  }
  if (params.size() == 0) {
    ret->params = slice(KOOPA_RSIK_VALUE);
  } else {
    ret->params = slice(params, KOOPA_RSIK_VALUE);
  }

  std::vector<const void *> blocks;
  block_manager.init(&blocks);
  koopa_raw_basic_block_data_t *entry = new koopa_raw_basic_block_data_t();
  entry->name = "%entry";
  entry->params = slice(KOOPA_RSIK_VALUE);
  entry->used_by = slice(KOOPA_RSIK_VALUE);
  symbol_list.newScope();
  block_manager.newBlock(entry);
  for (int i = 0; i < params.size(); i++) {
    koopa_raw_value_data *allo = new koopa_raw_value_data();
    koopa_raw_type_kind *ty = new koopa_raw_type_kind();
    ty->tag = KOOPA_RTT_POINTER;
    ty->data.pointer.base = ((koopa_raw_value_t)params[i])->ty;
    allo->ty = ty;
    allo->name = ((koopa_raw_value_t)params[i])->name;
    allo->used_by = slice(KOOPA_RSIK_VALUE);
    allo->kind.tag = KOOPA_RVT_ALLOC;
    if (ty->data.pointer.base->tag == KOOPA_RTT_INT32)
      symbol_list.addSymbol(allo->name + 1, Value(ValueType::Var, allo));
    else
      symbol_list.addSymbol(allo->name + 1, Value(ValueType::Pointer, allo));
    block_manager.addInst(allo);
    koopa_raw_value_data *store = new koopa_raw_value_data();
    store->ty = type_kind(KOOPA_RTT_UNIT);
    store->name = nullptr;
    store->used_by = slice(KOOPA_RSIK_VALUE);
    store->kind.tag = KOOPA_RVT_STORE;
    store->kind.data.store.dest = allo;
    store->kind.data.store.value = (koopa_raw_value_t)params[i];
    block_manager.addInst(store);
  }
  block->to_koopa();
  block_manager.addInst(
      ret_value(((koopa_raw_type_t)func_type->to_koopa())->tag));
  symbol_list.delScope();
  block_manager.delBlock();
  block_manager.delUnreachableBlock();
  for (int i = 0; i < blocks.size(); i++) {
    koopa_raw_basic_block_data_t *block =
        (koopa_raw_basic_block_data_t *)blocks[i];
    char *name = new char[ident.length() + strlen(block->name) + 2];
    ("%" + ident + "_" + (block->name + 1))
        .copy(name, ident.length() + strlen(block->name) + 2);
    name[ident.length() + strlen(block->name) + 1] = '\0';
    block->name = name;
  }
  ret->bbs = slice(blocks, KOOPA_RSIK_BASIC_BLOCK);
  return ret;
}

// FuncFParamAST
FuncFParamAST::FuncFParamAST(std::unique_ptr<BaseAST> &param_type,
                             const char *ident, FuncFParamType type)
    : param_type(std::move(param_type)), ident(ident), type(type) {}

FuncFParamAST::FuncFParamAST(
    std::unique_ptr<BaseAST> &param_type,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array,
    const char *ident, FuncFParamType type)
    : param_type(std::move(param_type)), ident(ident), type(type),
      index_array(std::move(index_array)) {}

void *FuncFParamAST::to_koopa() const {
  if (type == Var)
    return type_kind(KOOPA_RTT_INT32);
  if (type == Array) {
    koopa_raw_type_kind *ty = new koopa_raw_type_kind();
    if (index_array == nullptr) {
      ty = pointer_type_kind(KOOPA_RTT_INT32);
    } else {
      std::vector<size_t> size_vec;
      for (auto index = (*index_array).begin(); index != (*index_array).end();
           index++) {
        size_t tmp = (*index)->cal_value();
        size_vec.push_back(tmp);
      }
      ty->tag = KOOPA_RTT_POINTER;
      ty->data.pointer.base = array_type_kind(KOOPA_RTT_INT32, size_vec);
    }
    return ty;
  }
  assert(false);
}

void *FuncFParamAST::to_koopa(int index) const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = (koopa_raw_type_t)to_koopa();
  char *name = new char[ident.length() + 1];
  ("@" + ident).copy(name, ident.length() + 1);
  name[ident.length() + 1] = '\0';
  ret->name = name;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_FUNC_ARG_REF;
  ret->kind.data.func_arg_ref.index = index;
  return ret;
}

// BlockAST
BlockAST::BlockAST() { type = Empty; }

BlockAST::BlockAST(
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &blockitem_vec)
    : blockitem_vec(std::move(blockitem_vec)) {
  type = Item;
}

void *BlockAST::to_koopa() const {
  if (type == Empty)
    return nullptr;
  if (type == Item) {
    for (auto blockitem = (*blockitem_vec).rbegin();
         blockitem != (*blockitem_vec).rend(); blockitem++) {
      (*blockitem)->to_koopa();
    }
    return nullptr;
  }
  return nullptr;
}

// StmtAST
StmtAST::StmtAST(StmtType type) : type(type) {}

StmtAST::StmtAST(std::unique_ptr<BaseAST> &exp, StmtType type)
    : type(type), exp(std::move(exp)) {}

StmtAST::StmtAST(std::unique_ptr<BaseAST> &stmt, std::unique_ptr<BaseAST> &exp,
                 StmtType type)
    : type(type), exp(std::move(exp)), stmt(std::move(stmt)) {}

void *StmtAST::to_koopa() const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_UNIT);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  if (type == Return) {
    ret->kind.tag = KOOPA_RVT_RETURN;
    if (exp != nullptr)
      ret->kind.data.ret.value = (koopa_raw_value_t)exp->to_koopa();
    block_manager.addInst(ret);
  } else if (type == Assign) {
    ret->kind.tag = KOOPA_RVT_STORE;
    ret->kind.data.store.dest = (koopa_raw_value_t)stmt->to_left_value();
    ret->kind.data.store.value = (koopa_raw_value_t)exp->to_koopa();
    block_manager.addInst(ret);
  } else if (type == Exp) {
    symbol_list.newScope();
    exp->to_koopa();
    symbol_list.delScope();
  } else if (type == Block) {
    symbol_list.newScope();
    exp->to_koopa();
    symbol_list.delScope();
  } else if (type == If) {
    ret = (koopa_raw_value_data *)exp->to_koopa();
    koopa_raw_basic_block_data_t *false_block =
        new koopa_raw_basic_block_data_t();
    false_block->name = "%false";
    false_block->params = slice(KOOPA_RSIK_VALUE);
    false_block->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.data.branch.false_bb = (koopa_raw_basic_block_t)false_block;
    ret->kind.data.branch.false_args = slice(KOOPA_RSIK_VALUE);
    if (stmt != nullptr) {
      koopa_raw_basic_block_data_t *end_block =
          new koopa_raw_basic_block_data_t();
      end_block->name = "%end";
      end_block->params = slice(KOOPA_RSIK_VALUE);
      end_block->used_by = slice(KOOPA_RSIK_VALUE);
      block_manager.addInst(jump_value(end_block));
      block_manager.newBlock(false_block);
      symbol_list.newScope();
      stmt->to_koopa();
      symbol_list.delScope();
      block_manager.addInst(jump_value(end_block));
      block_manager.newBlock(end_block);
    } else {
      block_manager.addInst(jump_value(false_block));
      block_manager.newBlock(false_block);
    }
  } else if (type == While) {
    koopa_raw_basic_block_data_t *cond_block =
        new koopa_raw_basic_block_data_t();
    cond_block->name = "%while_entry";
    cond_block->params = slice(KOOPA_RSIK_VALUE);
    cond_block->used_by = slice(KOOPA_RSIK_VALUE);
    block_manager.addInst(jump_value(cond_block));
    block_manager.newBlock(cond_block);
    ret->kind.tag = KOOPA_RVT_BRANCH;
    ret->kind.data.branch.cond = (koopa_raw_value_t)exp->to_koopa();
    koopa_raw_basic_block_data_t *true_block =
        new koopa_raw_basic_block_data_t();
    true_block->name = "%while_body";
    true_block->params = slice(KOOPA_RSIK_VALUE);
    true_block->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.data.branch.true_bb = (koopa_raw_basic_block_t)true_block;
    ret->kind.data.branch.true_args = slice(KOOPA_RSIK_VALUE);
    koopa_raw_basic_block_data_t *end_block =
        new koopa_raw_basic_block_data_t();
    end_block->name = "%end";
    end_block->params = slice(KOOPA_RSIK_VALUE);
    end_block->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.data.branch.false_bb = (koopa_raw_basic_block_t)end_block;
    ret->kind.data.branch.false_args = slice(KOOPA_RSIK_VALUE);
    loop_manager.addWhile(cond_block, end_block);
    block_manager.addInst(ret);
    block_manager.newBlock(true_block);
    stmt->to_koopa();
    block_manager.addInst(jump_value(cond_block));
    block_manager.newBlock(end_block);
    loop_manager.delWhile();
  } else if (type == Break) {
    if (loop_manager.getTail() == nullptr) {
      std::cout << "break not in loop" << std::endl;
      assert(false);
    }
    block_manager.addInst(jump_value(loop_manager.getTail()));
  } else if (type == Continue) {
    if (loop_manager.getHead() == nullptr) {
      std::cout << "continue not in loop" << std::endl;
      assert(false);
    }
    block_manager.addInst(jump_value(loop_manager.getHead()));
  }
  return ret;
}

// IfAST
IfAST::IfAST(std::unique_ptr<BaseAST> &exp, std::unique_ptr<BaseAST> &stmt)
    : exp(std::move(exp)), stmt(std::move(stmt)) {}

void *IfAST::to_koopa() const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_UNIT);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_BRANCH;
  ret->kind.data.branch.cond = (koopa_raw_value_t)exp->to_koopa();
  block_manager.addInst(ret);
  koopa_raw_basic_block_data_t *true_block = new koopa_raw_basic_block_data_t();
  ret->kind.data.branch.true_bb = (koopa_raw_basic_block_t)true_block;
  ret->kind.data.branch.true_args = slice(KOOPA_RSIK_VALUE);
  true_block->name = "%true";
  true_block->params = slice(KOOPA_RSIK_VALUE);
  true_block->used_by = slice(KOOPA_RSIK_VALUE);
  block_manager.newBlock(true_block);
  stmt->to_koopa();
  return ret;
}

// ConstDeclAST
ConstDeclAST::ConstDeclAST(
    std::unique_ptr<BaseAST> &const_type,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &ConstDef_vec)
    : const_type(std::move(const_type)), ConstDef_vec(std::move(ConstDef_vec)) {
}

void *ConstDeclAST::to_koopa() const {
  koopa_raw_type_t type = (const koopa_raw_type_t)const_type->to_koopa();
  if (type->tag == KOOPA_RTT_UNIT) {
    std::cout << "number type is void" << std::endl;
    assert(false);
  }
  for (auto const_def = (*ConstDef_vec).rbegin();
       const_def != (*ConstDef_vec).rend(); const_def++) {
    (*const_def)->to_koopa(type);
  }
  return nullptr;
}

void *ConstDeclAST::to_koopa(std::vector<const void *> &global_var) const {
  koopa_raw_type_t type = (const koopa_raw_type_t)const_type->to_koopa();
  if (type->tag == KOOPA_RTT_UNIT) {
    std::cout << "number type is void" << std::endl;
    assert(false);
  }
  for (auto const_def = (*ConstDef_vec).rbegin();
       const_def != (*ConstDef_vec).rend(); const_def++) {
    (*const_def)->to_koopa(global_var, type);
  }
  return nullptr;
}

// TypeAST
TypeAST::TypeAST(const char *type) : type(type) {}

void *TypeAST::to_koopa() const {
  if (type == "int")
    return (void *)type_kind(KOOPA_RTT_INT32);
  if (type == "void")
    return (void *)type_kind(KOOPA_RTT_UNIT);
  return nullptr; // not implemented
}

// ConstDefAST
ConstDefAST::ConstDefAST(const char *ident, std::unique_ptr<BaseAST> &exp)
    : ident(ident), exp(std::move(exp)) {
  type = Var;
}

ConstDefAST::ConstDefAST(
    const char *ident,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array,
    std::unique_ptr<BaseAST> &exp)
    : ident(ident), exp(std::move(exp)), index_array(std::move(index_array)) {
  type = Array;
}

void *ConstDefAST::to_koopa(koopa_raw_type_t const_type) const {
  if (type == Var) {
    int val = exp->cal_value();
    Value value = Value(ValueType::Const, val);
    symbol_list.addSymbol(ident.c_str(), value);
    return nullptr;
  }
  if (type == Array) {
    koopa_raw_value_data *ret = new koopa_raw_value_data();
    std::vector<size_t> size_vec;
    size_t size = 1;
    for (auto index = (*index_array).begin(); index != (*index_array).end();
         index++) {
      size_t tmp = (*index)->cal_value();
      size_vec.push_back(tmp);
      size *= tmp;
    }
    koopa_raw_type_kind *ty = new koopa_raw_type_kind();
    ty->tag = KOOPA_RTT_POINTER;
    ty->data.pointer.base = array_type_kind(const_type->tag, size_vec);
    ret->ty = ty;
    char *name = new char[ident.length() + 1];
    ("@" + ident).copy(name, ident.length() + 1);
    name[ident.length() + 1] = '\0';
    ret->name = name;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_ALLOC;
    block_manager.addInst(ret);
    Value value = Value(ValueType::Array, ret);
    symbol_list.addSymbol(ident.c_str(), value);
    std::vector<const void *> init_vec;
    std::vector<int> number_vec;
    InitValAST *initval = dynamic_cast<InitValAST *>(exp.get());
    if (initval->type == InitValAST::Empty) {
      koopa_raw_value_data *store = new koopa_raw_value_data();
      store->ty = type_kind(KOOPA_RTT_UNIT);
      store->name = nullptr;
      store->used_by = slice(KOOPA_RSIK_VALUE);
      store->kind.tag = KOOPA_RVT_STORE;
      store->kind.data.store.dest = ret;
      store->kind.data.store.value =
          zero_init(array_type_kind(const_type->tag, size_vec));
    } else {
      initval->preprocess(init_vec, size_vec);
      if (init_vec.size() != size) {
        std::cout << "array size not match" << std::endl;
        assert(false);
      }
      std::vector<koopa_raw_value_data *> get_vec;
      for (int i = 0; i < size; i++) {
        int tmp = i;
        int tmp_size = size;
        for (int j = 0; j < size_vec.size(); j++) {
          tmp_size /= size_vec[j];
          int index = tmp / tmp_size;
          tmp = tmp % tmp_size;
          if (j < get_vec.size()) {
            if (index ==
                get_vec[j]
                    ->kind.data.get_elem_ptr.index->kind.data.integer.value) {
              continue;
            } else {
              while (j < get_vec.size()) {
                get_vec.pop_back();
              }
            }
          }
          koopa_raw_value_data *get = new koopa_raw_value_data();
          get->ty = type_kind(KOOPA_RTT_INT32);
          get->name = nullptr;
          get->used_by = slice(KOOPA_RSIK_VALUE);
          get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
          get->kind.data.get_elem_ptr.src =
              j == 0 ? ret : (koopa_raw_value_t)get_vec[j - 1];
          get->kind.data.get_elem_ptr.index =
              (koopa_raw_value_t)NumberAST(index).to_koopa();
          get_vec.push_back(get);
          block_manager.addInst(get);
        }
        koopa_raw_value_data *store = new koopa_raw_value_data();
        store->ty = type_kind(KOOPA_RTT_UNIT);
        store->name = nullptr;
        store->used_by = slice(KOOPA_RSIK_VALUE);
        store->kind.tag = KOOPA_RVT_STORE;
        store->kind.data.store.dest =
            (koopa_raw_value_t)get_vec[size_vec.size() - 1];
        store->kind.data.store.value = (koopa_raw_value_t)init_vec[i];
        block_manager.addInst(store);
      }
    }
    return nullptr;
  }
  return nullptr;
}

void *ConstDefAST::to_koopa(std::vector<const void *> &global_var,
                            koopa_raw_type_t const_type) const {
  if (type == Var) {
    int val = exp->cal_value();
    Value value = Value(ValueType::Const, val);
    symbol_list.addSymbol(ident.c_str(), value);
    return nullptr;
  }
  if (type == Array) {
    koopa_raw_value_data *ret = new koopa_raw_value_data();
    std::vector<size_t> size_vec;
    size_t size = 1;
    for (auto index = (*index_array).begin(); index != (*index_array).end();
         index++) {
      size_t tmp = (*index)->cal_value();
      size_vec.push_back(tmp);
      size *= tmp;
    }
    koopa_raw_type_kind *ty = new koopa_raw_type_kind();
    ty->tag = KOOPA_RTT_POINTER;
    ty->data.pointer.base = array_type_kind(const_type->tag, size_vec);
    ret->ty = ty;
    char *name = new char[ident.length() + 1];
    ("@" + ident).copy(name, ident.length() + 1);
    name[ident.length() + 1] = '\0';
    ret->name = name;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_GLOBAL_ALLOC;
    global_var.push_back(ret);
    Value value = Value(ValueType::Array, ret);
    symbol_list.addSymbol(ident.c_str(), value);
    std::vector<const void *> init_vec;
    InitValAST *initval = dynamic_cast<InitValAST *>(exp.get());
    std::vector<int> number_vec;
    if (initval->type == InitValAST::Empty) {
      ret->kind.data.global_alloc.init =
          zero_init(array_type_kind(const_type->tag, size_vec));
    } else {
      initval->preprocess(init_vec, size_vec);
      if (init_vec.size() != size) {
        std::cout << "array size not match" << std::endl;
        assert(false);
      }
      ret->kind.data.global_alloc.init =
          (koopa_raw_value_t)initval->to_koopa(init_vec, size_vec, 0);
    }
    return nullptr;
  }
  return nullptr;
}

// VarDeclAST
VarDeclAST::VarDeclAST(
    std::unique_ptr<BaseAST> &var_type,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &VarDef_vec)
    : var_type(std::move(var_type)), VarDef_vec(std::move(VarDef_vec)) {}

void *VarDeclAST::to_koopa() const {
  koopa_raw_type_t type = (const koopa_raw_type_t)var_type->to_koopa();
  if (type->tag == KOOPA_RTT_UNIT) {
    std::cout << "number type is void" << std::endl;
    assert(false);
  }
  for (auto var_def = (*VarDef_vec).rbegin(); var_def != (*VarDef_vec).rend();
       var_def++) {
    (*var_def)->to_koopa(type);
  }
  return nullptr;
}

void *VarDeclAST::to_koopa(std::vector<const void *> &global_var) const {
  koopa_raw_type_t type = (const koopa_raw_type_t)var_type->to_koopa();
  for (auto var_def = (*VarDef_vec).rbegin(); var_def != (*VarDef_vec).rend();
       var_def++) {
    (*var_def)->to_koopa(global_var, type);
  }
  return nullptr;
}

// VarDefAST
VarDefAST::VarDefAST(const char *ident, std::unique_ptr<BaseAST> &exp,
                     VarDefType type)
    : ident(ident) {
  this->type = type;
  this->exp = std::move(exp);
}

VarDefAST::VarDefAST(const char *ident, VarDefType type) : ident(ident) {
  this->type = type;
}

VarDefAST::VarDefAST(
    const char *ident,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array,
    VarDefType type)
    : ident(ident), index_array(std::move(index_array)) {
  this->type = type;
}

VarDefAST::VarDefAST(
    const char *ident,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array,
    std::unique_ptr<BaseAST> &exp, VarDefType type)
    : ident(ident), exp(std::move(exp)), index_array(std::move(index_array)) {
  this->type = type;
}

void *VarDefAST::to_koopa(koopa_raw_type_t var_type) const {
  if (type == Exp) {
    koopa_raw_value_data *ret = new koopa_raw_value_data();
    ret->ty = pointer_type_kind(KOOPA_RTT_INT32);
    char *name = new char[ident.length() + 1];
    ("@" + ident).copy(name, ident.length() + 1);
    name[ident.length() + 1] = '\0';
    ret->name = name;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_ALLOC;
    block_manager.addInst(ret);
    Value value = Value(ValueType::Var, ret);
    symbol_list.addSymbol(ident.c_str(), value);
    if (exp != nullptr) {
      koopa_raw_value_data *store = new koopa_raw_value_data();
      store->ty = type_kind(KOOPA_RTT_INT32);
      store->name = nullptr;
      store->used_by = slice(KOOPA_RSIK_VALUE);
      store->kind.tag = KOOPA_RVT_STORE;
      store->kind.data.store.dest = (koopa_raw_value_t)ret;
      store->kind.data.store.value = (koopa_raw_value_t)exp->to_koopa();
      block_manager.addInst(store);
    }
  } else if (type == Array) {
    std::vector<size_t> size_vec;
    size_t size = 1;
    for (auto index = (*index_array).begin(); index != (*index_array).end();
         index++) {
      size_t tmp = (*index)->cal_value();
      size_vec.push_back(tmp);
      size *= tmp;
    }
    koopa_raw_value_data *ret = new koopa_raw_value_data();
    koopa_raw_type_kind *ty = new koopa_raw_type_kind();
    ty->tag = KOOPA_RTT_POINTER;
    ty->data.pointer.base = array_type_kind(var_type->tag, size_vec);
    ret->ty = ty;
    char *name = new char[ident.length() + 1];
    ("@" + ident).copy(name, ident.length() + 1);
    name[ident.length() + 1] = '\0';
    ret->name = name;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_ALLOC;
    block_manager.addInst(ret);
    Value value = Value(ValueType::Array, ret);
    symbol_list.addSymbol(ident.c_str(), value);
    if (exp != nullptr) {
      std::vector<const void *> init_vec;
      InitValAST *initval = dynamic_cast<InitValAST *>(exp.get());
      if (initval->type == InitValAST::Empty) {
        koopa_raw_value_data *store = new koopa_raw_value_data();
        store->ty = type_kind(KOOPA_RTT_UNIT);
        store->name = nullptr;
        store->used_by = slice(KOOPA_RSIK_VALUE);
        store->kind.tag = KOOPA_RVT_STORE;
        store->kind.data.store.dest = ret;
        store->kind.data.store.value =
            zero_init(array_type_kind(var_type->tag, size_vec));
        block_manager.addInst(store);
      } else {
        initval->preprocess(init_vec, size_vec);
        if (init_vec.size() != size) {
          std::cout << "array size not match" << std::endl;
          assert(false);
        }
        std::vector<koopa_raw_value_data *> get_vec;
        for (int i = 0; i < size; i++) {
          int tmp = i;
          int tmp_size = size;
          for (int j = 0; j < size_vec.size(); j++) {
            tmp_size /= size_vec[j];
            int index = tmp / tmp_size;
            tmp = tmp % tmp_size;
            if (j < get_vec.size()) {
              if (index ==
                  get_vec[j]
                      ->kind.data.get_elem_ptr.index->kind.data.integer.value) {
                continue;
              } else {
                while (j < get_vec.size()) {
                  get_vec.pop_back();
                }
              }
            }
            koopa_raw_value_data *get = new koopa_raw_value_data();
            get->ty = type_kind(KOOPA_RTT_INT32);
            get->name = nullptr;
            get->used_by = slice(KOOPA_RSIK_VALUE);
            get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
            get->kind.data.get_elem_ptr.src =
                j == 0 ? ret : (koopa_raw_value_t)get_vec[j - 1];
            get->kind.data.get_elem_ptr.index =
                (koopa_raw_value_t)NumberAST(index).to_koopa();
            get_vec.push_back(get);
            block_manager.addInst(get);
          }
          koopa_raw_value_data *store = new koopa_raw_value_data();
          store->ty = type_kind(KOOPA_RTT_UNIT);
          store->name = nullptr;
          store->used_by = slice(KOOPA_RSIK_VALUE);
          store->kind.tag = KOOPA_RVT_STORE;
          store->kind.data.store.dest =
              (koopa_raw_value_t)get_vec[size_vec.size() - 1];
          store->kind.data.store.value = (koopa_raw_value_t)init_vec[i];
          block_manager.addInst(store);
        }
      }
    } else {
      koopa_raw_value_data *store = new koopa_raw_value_data();
      store->ty = type_kind(KOOPA_RTT_UNIT);
      store->name = nullptr;
      store->used_by = slice(KOOPA_RSIK_VALUE);
      store->kind.tag = KOOPA_RVT_STORE;
      store->kind.data.store.dest = ret;
      store->kind.data.store.value =
          zero_init(array_type_kind(var_type->tag, size_vec));
      block_manager.addInst(store);
    }
  }
  return nullptr;
}

void *VarDefAST::to_koopa(std::vector<const void *> &global_var,
                          koopa_raw_type_t var_type) const {
  if (type == Exp) {
    koopa_raw_value_data *ret = new koopa_raw_value_data();
    ret->ty = pointer_type_kind(var_type->tag);
    char *name = new char[ident.length() + 1];
    ("@" + ident).copy(name, ident.length() + 1);
    name[ident.length() + 1] = '\0';
    ret->name = name;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_GLOBAL_ALLOC;
    global_var.push_back(ret);
    Value value = Value(ValueType::Var, ret);
    symbol_list.addSymbol(ident.c_str(), value);
    if (exp != nullptr) {
      ret->kind.data.global_alloc.init = (koopa_raw_value_t)exp->to_koopa();
    } else {
      ret->kind.data.global_alloc.init = zero_init(type_kind(var_type->tag));
    }
  } else if (type == Array) {
    std::vector<size_t> size_vec;
    size_t size = 1;
    for (auto index = (*index_array).begin(); index != (*index_array).end();
         index++) {
      size_t tmp = (*index)->cal_value();
      size_vec.push_back(tmp);
      size *= tmp;
    }
    koopa_raw_value_data *ret = new koopa_raw_value_data();
    koopa_raw_type_kind *ty = new koopa_raw_type_kind();
    ty->tag = KOOPA_RTT_POINTER;
    ty->data.pointer.base = array_type_kind(var_type->tag, size_vec);
    ret->ty = ty;
    char *name = new char[ident.length() + 1];
    ("@" + ident).copy(name, ident.length() + 1);
    name[ident.length() + 1] = '\0';
    ret->name = name;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_GLOBAL_ALLOC;
    global_var.push_back(ret);
    Value value = Value(ValueType::Array, ret);
    symbol_list.addSymbol(ident.c_str(), value);
    if (exp != nullptr) {
      std::vector<const void *> init_vec;
      InitValAST *initval = dynamic_cast<InitValAST *>(exp.get());
      std::vector<int> number_vec;
      if (initval->type == InitValAST::Empty) {
        ret->kind.data.global_alloc.init =
            zero_init(array_type_kind(var_type->tag, size_vec));
      } else {
        initval->preprocess(init_vec, size_vec);
        if (init_vec.size() != size) {
          std::cout << "array size not match " << init_vec.size() << " " << size
                    << std::endl;
          assert(false);
        }
        ret->kind.data.global_alloc.init =
            (koopa_raw_value_t)initval->to_koopa(init_vec, size_vec, 0);
      }
    } else {
      ret->kind.data.global_alloc.init =
          zero_init(array_type_kind(var_type->tag, size_vec));
    }
  }
  return nullptr;
}

// InitValAST
InitValAST::InitValAST() { type = Empty; }

InitValAST::InitValAST(std::unique_ptr<BaseAST> &exp) : exp(std::move(exp)) {
  type = Exp;
}

InitValAST::InitValAST(
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &initlist_vec)
    : initlist_vec(std::move(initlist_vec)) {
  type = InitList;
}

void *InitValAST::to_koopa() const {
  if (type != Exp)
    assert(false);
  return exp->to_koopa();
}

void *InitValAST::to_koopa(std::vector<const void *> &init_vec,
                           std::vector<size_t> size_vec, int level) const {
  std::vector<const void *> *init_val = new std::vector<const void *>();
  if (level == size_vec.size() - 1) {
    for (int i = 0; i < size_vec[level]; i++) {
      init_val->push_back((*init_vec.begin()));
      init_vec.erase(init_vec.begin());
    }
  } else {
    for (int i = 0; i < size_vec[level]; i++) {
      init_val->push_back(to_koopa(init_vec, size_vec, level + 1));
    }
  }
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  std::vector<size_t> sub_vec;
  for (int i = level; i < size_vec.size(); i++) {
    sub_vec.push_back(size_vec[i]);
  }
  ret->ty = array_type_kind(KOOPA_RTT_INT32, sub_vec);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_AGGREGATE;
  ret->kind.data.aggregate.elems = slice(*init_val, KOOPA_RSIK_VALUE);
  return (void *)ret;
}

void InitValAST::preprocess(std::vector<const void *> &init_vec,
                            std::vector<size_t> size_vec) {
  if (type != Empty) {
    size_t len_n = size_vec.back();
    for (auto init = (*initlist_vec).rbegin(); init != (*initlist_vec).rend();
         init++) {
      InitValAST *initval = dynamic_cast<InitValAST *>((*init).get());
      if (initval->type == Exp) {
        init_vec.push_back(NumberAST(initval->exp->cal_value()).to_koopa());
      } else if (initval->type == InitList || initval->type == Empty) {
        int curr_size = init_vec.size();
        std::vector<size_t> sub_vec;
        if (curr_size % len_n != 0) {
          std::cout << "init list error" << std::endl;
          assert(false);
        }
        int align_size = curr_size / len_n;
        int level = 1;
        for (int i = size_vec.size() - 2; i >= 0; i--) {
          if (align_size % size_vec[i] != 0) {
            break;
          }
          align_size /= size_vec[i];
          level++;
        }
        for (int i = std::max(int(size_vec.size() - level), 1);
             i < size_vec.size(); i++) {
          sub_vec.push_back(size_vec[i]);
        }
        initval->preprocess(init_vec, sub_vec);
      }
    }
  } else {
    int size = 1;
    for (int i = 0; i < size_vec.size(); i++) {
      size *= size_vec[i];
    }
    for (int i = 0; i < size; i++) {
      init_vec.push_back(NumberAST(0).to_koopa());
    }
  }
  int size = 1;
  for (int i = 0; i < size_vec.size(); i++) {
    size *= size_vec[i];
  }
  while (init_vec.size() % size != 0) {
    init_vec.push_back(NumberAST(0).to_koopa());
  }
}

int InitValAST::cal_value() const {
  if (type != Exp)
    assert(false);
  return exp->cal_value();
}

// LValAST
LValAST::LValAST(const char *ident) : ident(ident) {}

LValAST::LValAST(
    const char *ident,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array)
    : ident(ident), index_array(std::move(index_array)) {}

void *LValAST::to_left_value() const {
  Value value = symbol_list.getSymbol(ident);
  if (value.type == ValueType::Var) {
    return (void *)value.data.var_value;
  }
  if (value.type == ValueType::Array) {
    std::vector<koopa_raw_value_data *> get_vec;

    for (int i = 0; i < index_array->size(); i++) {
      koopa_raw_value_data *get = new koopa_raw_value_data();
      get->name = nullptr;
      get->used_by = slice(KOOPA_RSIK_VALUE);
      get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
      get->kind.data.get_elem_ptr.index =
          (koopa_raw_value_t)(*index_array)[i]->to_koopa();
      koopa_raw_type_kind *ty = new koopa_raw_type_kind();
      ty->tag = KOOPA_RTT_POINTER;
      if (i == 0) {
        ty->data.pointer.base =
            value.data.array_value->ty->data.pointer.base->data.array.base;
        get->ty = ty;
        get->kind.data.get_elem_ptr.src =
            (koopa_raw_value_t)value.data.array_value;
      } else {
        ty->data.pointer.base =
            get_vec[i - 1]->ty->data.pointer.base->data.array.base;
        get->ty = ty;
        get->kind.data.get_elem_ptr.src = (koopa_raw_value_t)get_vec[i - 1];
      }
      get_vec.push_back(get);
      block_manager.addInst(get);
    }
    return (void *)get_vec.back();
  }
  if (value.type == ValueType::Const) {
    std::cout << "const is not left value" << std::endl;
    assert(false);
  }
  if (value.type == ValueType::Func) {
    std::cout << "func is not left value" << std::endl;
    assert(false);
  }
  if (value.type == ValueType::Pointer) {
    koopa_raw_value_data *load = new koopa_raw_value_data();
    load->ty = value.data.pointer_value->ty->data.pointer.base;
    load->name = nullptr;
    load->used_by = slice(KOOPA_RSIK_VALUE);
    load->kind.tag = KOOPA_RVT_LOAD;
    load->kind.data.load.src = (koopa_raw_value_t)value.data.pointer_value;
    block_manager.addInst(load);
    std::vector<koopa_raw_value_data *> get_vec;
    for (int i = 0; i < index_array->size(); i++) {
      koopa_raw_value_data *get = new koopa_raw_value_data();
      get->name = nullptr;
      get->used_by = slice(KOOPA_RSIK_VALUE);
      if (i == 0) {
        get->ty = load->ty;
        get->kind.tag = KOOPA_RVT_GET_PTR;
        get->kind.data.get_ptr.index =
            (koopa_raw_value_t)(*index_array)[i]->to_koopa();
        get->kind.data.get_ptr.src = load;
      } else {
        koopa_raw_type_kind *ty = new koopa_raw_type_kind();
        ty->tag = KOOPA_RTT_POINTER;
        ty->data.pointer.base =
            get_vec[i - 1]->ty->data.pointer.base->data.array.base;
        get->ty = ty;
        get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
        get->kind.data.get_elem_ptr.index =
            (koopa_raw_value_t)(*index_array)[i]->to_koopa();
        get->kind.data.get_elem_ptr.src = (koopa_raw_value_t)get_vec[i - 1];
      }
      get_vec.push_back(get);
      block_manager.addInst(get);
    }
    return (void *)get_vec.back();
  }
  assert(false);
}

void *LValAST::to_koopa() const {
  Value value = symbol_list.getSymbol(ident);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  if (value.type == ValueType::Var) {
    ret->kind.tag = KOOPA_RVT_LOAD;
    ret->kind.data.load.src = (koopa_raw_value_t)value.data.var_value;
    block_manager.addInst(ret);
  } else if (value.type == ValueType::Const) {
    ret->kind.tag = KOOPA_RVT_INTEGER;
    ret->kind.data.integer.value = value.data.const_value;
  } else if (value.type == ValueType::Array) {
    std::vector<koopa_raw_value_data *> get_vec;
    if (index_array != nullptr) {
      for (int i = 0; i < index_array->size(); i++) {
        koopa_raw_value_data *get = new koopa_raw_value_data();
        get->name = nullptr;
        get->used_by = slice(KOOPA_RSIK_VALUE);
        get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
        get->kind.data.get_elem_ptr.index =
            (koopa_raw_value_t)(*index_array)[i]->to_koopa();
        koopa_raw_type_kind *ty = new koopa_raw_type_kind();
        ty->tag = KOOPA_RTT_POINTER;
        if (i == 0) {

          ty->data.pointer.base =
              value.data.array_value->ty->data.pointer.base->data.array.base;
          get->ty = ty;
          get->kind.data.get_elem_ptr.src =
              (koopa_raw_value_t)value.data.array_value;
        } else {
          ty->data.pointer.base =
              get_vec[i - 1]->ty->data.pointer.base->data.array.base;
          get->ty = ty;
          get->kind.data.get_elem_ptr.src = (koopa_raw_value_t)get_vec[i - 1];
        }
        get_vec.push_back(get);
        block_manager.addInst(get);
      }
    }
    if (index_array == nullptr) {
      koopa_raw_value_data *get = new koopa_raw_value_data();
      koopa_raw_type_kind *ty = new koopa_raw_type_kind();
      ty->tag = KOOPA_RTT_POINTER;
      ty->data.pointer.base =
          value.data.array_value->ty->data.pointer.base->data.array.base;
      get->ty = ty;
      get->name = nullptr;
      get->used_by = slice(KOOPA_RSIK_VALUE);
      get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
      get->kind.data.get_elem_ptr.src = value.data.array_value;
      get->kind.data.get_elem_ptr.index =
          (koopa_raw_value_t)NumberAST(0).to_koopa();
      block_manager.addInst(get);
      ret = get;
    } else if (get_vec.back()->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY) {
      koopa_raw_value_data *get = new koopa_raw_value_data();
      koopa_raw_type_kind *ty = new koopa_raw_type_kind();
      ty->tag = KOOPA_RTT_POINTER;
      ty->data.pointer.base =
          get_vec.back()->ty->data.pointer.base->data.array.base;
      get->ty = ty;
      get->name = nullptr;
      get->used_by = slice(KOOPA_RSIK_VALUE);
      get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
      get->kind.data.get_elem_ptr.src = (koopa_raw_value_t)get_vec.back();
      get->kind.data.get_elem_ptr.index =
          (koopa_raw_value_t)NumberAST(0).to_koopa();
      block_manager.addInst(get);
      ret = get;
    } else {
      ret->kind.tag = KOOPA_RVT_LOAD;
      ret->kind.data.load.src = (koopa_raw_value_t)get_vec.back();
      block_manager.addInst(ret);
    }
  }
  if (value.type == ValueType::Func) {
    std::cout << "func is not a value" << std::endl;
    assert(false);
  }
  if (value.type == ValueType::Pointer) {
    koopa_raw_value_data *load = new koopa_raw_value_data();
    load->ty = value.data.pointer_value->ty->data.pointer.base;
    load->name = nullptr;
    load->used_by = slice(KOOPA_RSIK_VALUE);
    load->kind.tag = KOOPA_RVT_LOAD;
    load->kind.data.load.src = value.data.pointer_value;
    block_manager.addInst(load);
    std::vector<koopa_raw_value_data *> get_vec;
    if (index_array != nullptr) {
      for (int i = 0; i < index_array->size(); i++) {
        koopa_raw_value_data *get = new koopa_raw_value_data();
        get->name = nullptr;
        get->used_by = slice(KOOPA_RSIK_VALUE);
        if (i == 0) {
          get->kind.tag = KOOPA_RVT_GET_PTR;
          get->kind.data.get_ptr.index =
              (koopa_raw_value_t)(*index_array)[i]->to_koopa();
          get->kind.data.get_ptr.src = load;
          get->ty = load->ty;
        } else {
          get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
          get->kind.data.get_elem_ptr.index =
              (koopa_raw_value_t)(*index_array)[i]->to_koopa();
          get->kind.data.get_elem_ptr.src = (koopa_raw_value_t)get_vec[i - 1];
          koopa_raw_type_kind *ty = new koopa_raw_type_kind();
          ty->tag = KOOPA_RTT_POINTER;
          ty->data.pointer.base =
              get_vec[i - 1]->ty->data.pointer.base->data.array.base;
          get->ty = ty;
        }
        get_vec.push_back(get);
        block_manager.addInst(get);
      }
    }
    if (index_array == nullptr) {
      koopa_raw_value_data *get = new koopa_raw_value_data();
      get->ty = load->ty;
      get->name = nullptr;
      get->used_by = slice(KOOPA_RSIK_VALUE);
      get->kind.tag = KOOPA_RVT_GET_PTR;
      get->kind.data.get_ptr.src = load;
      get->kind.data.get_ptr.index = (koopa_raw_value_t)NumberAST(0).to_koopa();
      block_manager.addInst(get);
      ret = get;
    } else if (get_vec.back()->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY) {
      koopa_raw_value_data *get = new koopa_raw_value_data();
      koopa_raw_type_kind *ty = new koopa_raw_type_kind();
      ty->tag = KOOPA_RTT_POINTER;
      ty->data.pointer.base =
          get_vec.back()->ty->data.pointer.base->data.array.base;
      get->ty = ty;
      get->name = nullptr;
      get->used_by = slice(KOOPA_RSIK_VALUE);
      get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
      get->kind.data.get_elem_ptr.src = (koopa_raw_value_t)get_vec.back();
      get->kind.data.get_elem_ptr.index =
          (koopa_raw_value_t)NumberAST(0).to_koopa();
      block_manager.addInst(get);
      ret = get;
    } else {
      ret->kind.tag = KOOPA_RVT_LOAD;
      ret->kind.data.load.src = (koopa_raw_value_t)get_vec.back();
      block_manager.addInst(ret);
    }
  }
  return ret;
}

int LValAST::cal_value() const {
  return symbol_list.getSymbol(ident).data.const_value;
}

// ExpAST
ExpAST::ExpAST(std::unique_ptr<BaseAST> &add_exp)
    : add_exp(std::move(add_exp)) {}

void *ExpAST::to_koopa() const { return add_exp->to_koopa(); }

int ExpAST::cal_value() const { return add_exp->cal_value(); }

// PrimaryExpAST
PrimaryExpAST::PrimaryExpAST(std::unique_ptr<BaseAST> &exp)
    : exp(std::move(exp)) {}

void *PrimaryExpAST::to_koopa() const { return exp->to_koopa(); }

int PrimaryExpAST::cal_value() const { return exp->cal_value(); }

// UnaryExpAST
UnaryExpAST::UnaryExpAST(std::unique_ptr<BaseAST> &exp) : exp(std::move(exp)) {
  type = Exp;
}

UnaryExpAST::UnaryExpAST(const char *op, std::unique_ptr<BaseAST> &exp)
    : op(op), exp(std::move(exp)) {
  type = Op;
}

UnaryExpAST::UnaryExpAST(
    const char *indet,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &args)
    : op(indet), args(std::move(args)) {
  type = Call;
}

void *UnaryExpAST::to_koopa() const {
  if (type == Exp || op == "+")
    return exp->to_koopa();
  if (type == Op) {
    koopa_raw_value_data *ret = new koopa_raw_value_data();
    ret->ty = type_kind(KOOPA_RTT_INT32);
    ret->name = nullptr;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_BINARY;
    auto &binary = ret->kind.data.binary;
    if (op == "-") {
      binary.op = KOOPA_RBO_SUB;
    }
    if (op == "!") {
      binary.op = KOOPA_RBO_EQ;
    }
    NumberAST zero(0);
    binary.lhs = (koopa_raw_value_t)zero.to_koopa();
    binary.rhs = (koopa_raw_value_t)exp->to_koopa();
    block_manager.addInst(ret);
    return ret;
  } else if (type == Call) {
    koopa_raw_value_data *ret = new koopa_raw_value_data();
    koopa_raw_function_t func = symbol_list.getSymbol(op).data.func_value;
    ret->ty = func->ty->data.function.ret;
    ret->name = nullptr;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_CALL;
    ret->kind.data.call.callee = func;
    std::vector<const void *> arg_vec;
    for (int i = args->size() - 1; i >= 0; i--) {
      koopa_raw_value_t arg = (koopa_raw_value_t)(*args)[i]->to_koopa();
      arg_vec.push_back(arg);
    }
    if (arg_vec.size() == 0) {
      ret->kind.data.call.args = slice(KOOPA_RSIK_VALUE);
    } else {
      ret->kind.data.call.args = slice(arg_vec, KOOPA_RSIK_VALUE);
    }
    block_manager.addInst(ret);
    return ret;
  }
  return nullptr;
}

int UnaryExpAST::cal_value() const {
  if (type == Exp || op == "+")
    return exp->cal_value();
  if (op == "-")
    return -exp->cal_value();
  if (op == "!")
    return !exp->cal_value();
  return 0;
}

// AddExpAST
AddExpAST::AddExpAST(std::unique_ptr<BaseAST> &mul_exp)
    : mul_exp(std::move(mul_exp)) {
  type = Exp;
}

AddExpAST::AddExpAST(const char *op, std::unique_ptr<BaseAST> &add_exp,
                     std::unique_ptr<BaseAST> &mul_exp)
    : op(op), add_exp(std::move(add_exp)), mul_exp(std::move(mul_exp)) {
  type = Op;
}

void *AddExpAST::to_koopa() const {
  if (type == Exp)
    return mul_exp->to_koopa();
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "+") {
    binary.op = KOOPA_RBO_ADD;
  }
  if (op == "-") {
    binary.op = KOOPA_RBO_SUB;
  }
  binary.lhs = (koopa_raw_value_t)add_exp->to_koopa();
  binary.rhs = (koopa_raw_value_t)mul_exp->to_koopa();
  block_manager.addInst(ret);
  return ret;
}

int AddExpAST::cal_value() const {
  if (type == Exp)
    return mul_exp->cal_value();
  if (op == "+")
    return add_exp->cal_value() + mul_exp->cal_value();
  if (op == "-")
    return add_exp->cal_value() - mul_exp->cal_value();
  return 0;
}

// MulExpAST
MulExpAST::MulExpAST(std::unique_ptr<BaseAST> &unary_exp)
    : unary_exp(std::move(unary_exp)) {
  type = Exp;
}

MulExpAST::MulExpAST(const char *op, std::unique_ptr<BaseAST> &mul_exp,
                     std::unique_ptr<BaseAST> &unary_exp)
    : op(op), mul_exp(std::move(mul_exp)), unary_exp(std::move(unary_exp)) {
  type = Op;
}

void *MulExpAST::to_koopa() const {
  if (type == Exp)
    return unary_exp->to_koopa();
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "*") {
    binary.op = KOOPA_RBO_MUL;
  }
  if (op == "/") {
    binary.op = KOOPA_RBO_DIV;
  }
  if (op == "%") {
    binary.op = KOOPA_RBO_MOD;
  }
  binary.lhs = (koopa_raw_value_t)mul_exp->to_koopa();
  binary.rhs = (koopa_raw_value_t)unary_exp->to_koopa();
  block_manager.addInst(ret);
  return ret;
}

int MulExpAST::cal_value() const {
  if (type == Exp)
    return unary_exp->cal_value();
  if (op == "*")
    return mul_exp->cal_value() * unary_exp->cal_value();
  if (op == "/")
    return mul_exp->cal_value() / unary_exp->cal_value();
  if (op == "%")
    return mul_exp->cal_value() % unary_exp->cal_value();
  return 0;
}

// RelExpAST
RelExpAST::RelExpAST(std::unique_ptr<BaseAST> &add_exp)
    : add_exp(std::move(add_exp)) {
  type = Exp;
}

RelExpAST::RelExpAST(const char *op, std::unique_ptr<BaseAST> &rel_exp,
                     std::unique_ptr<BaseAST> &add_exp)
    : op(op), rel_exp(std::move(rel_exp)), add_exp(std::move(add_exp)) {
  type = Op;
}

void *RelExpAST::to_koopa() const {
  if (type == Exp)
    return add_exp->to_koopa();
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "<") {
    binary.op = KOOPA_RBO_LT;
  }
  if (op == ">") {
    binary.op = KOOPA_RBO_GT;
  }
  if (op == "<=") {
    binary.op = KOOPA_RBO_LE;
  }
  if (op == ">=") {
    binary.op = KOOPA_RBO_GE;
  }
  binary.lhs = (koopa_raw_value_t)rel_exp->to_koopa();
  binary.rhs = (koopa_raw_value_t)add_exp->to_koopa();
  block_manager.addInst(ret);
  return ret;
}

int RelExpAST::cal_value() const {
  if (type == Exp)
    return add_exp->cal_value();
  if (op == "<")
    return rel_exp->cal_value() < add_exp->cal_value();
  if (op == ">")
    return rel_exp->cal_value() > add_exp->cal_value();
  if (op == "<=")
    return rel_exp->cal_value() <= add_exp->cal_value();
  if (op == ">=")
    return rel_exp->cal_value() >= add_exp->cal_value();
  return 0;
}

// EqExpAST
EqExpAST::EqExpAST(std::unique_ptr<BaseAST> &rel_exp)
    : rel_exp(std::move(rel_exp)) {
  type = Exp;
}

EqExpAST::EqExpAST(const char *op, std::unique_ptr<BaseAST> &eq_exp,
                   std::unique_ptr<BaseAST> &rel_exp)
    : op(op), eq_exp(std::move(eq_exp)), rel_exp(std::move(rel_exp)) {
  type = Op;
}

void *EqExpAST::to_koopa() const {
  if (type == Exp)
    return rel_exp->to_koopa();
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "==") {
    binary.op = KOOPA_RBO_EQ;
  }
  if (op == "!=") {
    binary.op = KOOPA_RBO_NOT_EQ;
  }
  binary.lhs = (koopa_raw_value_t)eq_exp->to_koopa();
  binary.rhs = (koopa_raw_value_t)rel_exp->to_koopa();
  block_manager.addInst(ret);
  return ret;
}

int EqExpAST::cal_value() const {
  if (type == Exp)
    return rel_exp->cal_value();
  if (op == "==")
    return eq_exp->cal_value() == rel_exp->cal_value();
  if (op == "!=")
    return eq_exp->cal_value() != rel_exp->cal_value();
  return 0;
}

// LAndExpAST
LAndExpAST::LAndExpAST(std::unique_ptr<BaseAST> &eq_exp)
    : eq_exp(std::move(eq_exp)) {
  type = Exp;
}

LAndExpAST::LAndExpAST(const char *op, std::unique_ptr<BaseAST> &and_exp,
                       std::unique_ptr<BaseAST> &eq_exp)
    : op(op), and_exp(std::move(and_exp)), eq_exp(std::move(eq_exp)) {
  type = Op;
}

void *LAndExpAST::make_bool(const std::unique_ptr<BaseAST> &exp) const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  binary.op = KOOPA_RBO_NOT_EQ;
  binary.lhs = (koopa_raw_value_t)exp->to_koopa();
  NumberAST zero(0);
  binary.rhs = (koopa_raw_value_t)zero.to_koopa();
  block_manager.addInst(ret);
  return ret;
}

void *LAndExpAST::to_koopa() const {
  if (type == Exp)
    return eq_exp->to_koopa();
  koopa_raw_value_data *temp = new koopa_raw_value_data();
  temp->ty = pointer_type_kind(KOOPA_RTT_INT32);
  temp->name = "@temp";
  temp->used_by = slice(KOOPA_RSIK_VALUE);
  temp->kind.tag = KOOPA_RVT_ALLOC;
  block_manager.addInst(temp);
  koopa_raw_value_data *temp_store = new koopa_raw_value_data();
  temp_store->ty = type_kind(KOOPA_RTT_UNIT);
  temp_store->name = nullptr;
  temp_store->used_by = slice(KOOPA_RSIK_VALUE);
  temp_store->kind.tag = KOOPA_RVT_STORE;
  temp_store->kind.data.store.dest = temp;
  temp_store->kind.data.store.value =
      (koopa_raw_value_t)NumberAST(0).to_koopa();
  block_manager.addInst(temp_store);
  koopa_raw_value_data *branch = new koopa_raw_value_data();
  branch->ty = type_kind(KOOPA_RTT_UNIT);
  branch->name = nullptr;
  branch->used_by = slice(KOOPA_RSIK_VALUE);
  branch->kind.tag = KOOPA_RVT_BRANCH;
  branch->kind.data.branch.cond = (koopa_raw_value_t)make_bool(and_exp);
  koopa_raw_basic_block_data_t *true_block = new koopa_raw_basic_block_data_t();
  true_block->name = "%true";
  true_block->params = slice(KOOPA_RSIK_VALUE);
  true_block->used_by = slice(KOOPA_RSIK_VALUE);
  branch->kind.data.branch.true_bb = (koopa_raw_basic_block_t)true_block;
  branch->kind.data.branch.true_args = slice(KOOPA_RSIK_VALUE);
  koopa_raw_basic_block_data_t *false_block =
      new koopa_raw_basic_block_data_t();
  false_block->name = "%end";
  false_block->params = slice(KOOPA_RSIK_VALUE);
  false_block->used_by = slice(KOOPA_RSIK_VALUE);
  branch->kind.data.branch.false_bb = (koopa_raw_basic_block_t)false_block;
  branch->kind.data.branch.false_args = slice(KOOPA_RSIK_VALUE);
  block_manager.addInst(branch);
  block_manager.newBlock(true_block);
  koopa_raw_value_data *true_store = new koopa_raw_value_data();
  true_store->ty = type_kind(KOOPA_RTT_UNIT);
  true_store->name = nullptr;
  true_store->used_by = slice(KOOPA_RSIK_VALUE);
  true_store->kind.tag = KOOPA_RVT_STORE;
  true_store->kind.data.store.dest = temp;
  true_store->kind.data.store.value = (koopa_raw_value_t)make_bool(eq_exp);
  block_manager.addInst(true_store);
  block_manager.addInst(jump_value(false_block));
  block_manager.newBlock(false_block);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_LOAD;
  ret->kind.data.load.src = temp;
  block_manager.addInst(ret);
  return ret;
}

int LAndExpAST::cal_value() const {
  if (type == Exp)
    return eq_exp->cal_value();
  return and_exp->cal_value() && eq_exp->cal_value();
}

// LOrExpAST
LOrExpAST::LOrExpAST(std::unique_ptr<BaseAST> &and_exp)
    : and_exp(std::move(and_exp)) {
  type = Exp;
}

LOrExpAST::LOrExpAST(const char *op, std::unique_ptr<BaseAST> &or_exp,
                     std::unique_ptr<BaseAST> &and_exp)
    : op(op), or_exp(std::move(or_exp)), and_exp(std::move(and_exp)) {
  type = Op;
}

void *LOrExpAST::make_bool(const std::unique_ptr<BaseAST> &exp) const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  binary.op = KOOPA_RBO_NOT_EQ;
  binary.lhs = (koopa_raw_value_t)exp->to_koopa();
  NumberAST zero(0);
  binary.rhs = (koopa_raw_value_t)zero.to_koopa();
  block_manager.addInst(ret);
  return ret;
}

void *LOrExpAST::to_koopa() const {
  if (type == Exp)
    return and_exp->to_koopa();
  koopa_raw_value_data *temp = new koopa_raw_value_data();
  temp->ty = pointer_type_kind(KOOPA_RTT_INT32);
  temp->name = "@temp";
  temp->used_by = slice(KOOPA_RSIK_VALUE);
  temp->kind.tag = KOOPA_RVT_ALLOC;
  block_manager.addInst(temp);
  koopa_raw_value_data *temp_store = new koopa_raw_value_data();
  temp_store->ty = type_kind(KOOPA_RTT_UNIT);
  temp_store->name = nullptr;
  temp_store->used_by = slice(KOOPA_RSIK_VALUE);
  temp_store->kind.tag = KOOPA_RVT_STORE;
  temp_store->kind.data.store.dest = temp;
  temp_store->kind.data.store.value =
      (koopa_raw_value_t)NumberAST(1).to_koopa();
  block_manager.addInst(temp_store);
  koopa_raw_value_data *branch = new koopa_raw_value_data();
  branch->ty = type_kind(KOOPA_RTT_UNIT);
  branch->name = nullptr;
  branch->used_by = slice(KOOPA_RSIK_VALUE);
  branch->kind.tag = KOOPA_RVT_BRANCH;
  branch->kind.data.branch.cond = (koopa_raw_value_t)make_bool(or_exp);
  koopa_raw_basic_block_data_t *true_block = new koopa_raw_basic_block_data_t();
  true_block->name = "%end";
  true_block->params = slice(KOOPA_RSIK_VALUE);
  true_block->used_by = slice(KOOPA_RSIK_VALUE);
  branch->kind.data.branch.true_bb = (koopa_raw_basic_block_t)true_block;
  branch->kind.data.branch.true_args = slice(KOOPA_RSIK_VALUE);
  koopa_raw_basic_block_data_t *false_block =
      new koopa_raw_basic_block_data_t();
  false_block->name = "%false";
  false_block->params = slice(KOOPA_RSIK_VALUE);
  false_block->used_by = slice(KOOPA_RSIK_VALUE);
  branch->kind.data.branch.false_bb = (koopa_raw_basic_block_t)false_block;
  branch->kind.data.branch.false_args = slice(KOOPA_RSIK_VALUE);
  block_manager.addInst(branch);
  block_manager.newBlock(false_block);
  koopa_raw_value_data *false_store = new koopa_raw_value_data();
  false_store->ty = type_kind(KOOPA_RTT_UNIT);
  false_store->name = nullptr;
  false_store->used_by = slice(KOOPA_RSIK_VALUE);
  false_store->kind.tag = KOOPA_RVT_STORE;
  false_store->kind.data.store.dest = temp;
  false_store->kind.data.store.value = (koopa_raw_value_t)make_bool(and_exp);
  block_manager.addInst(false_store);
  block_manager.addInst(jump_value(true_block));
  block_manager.newBlock(true_block);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_LOAD;
  ret->kind.data.load.src = temp;
  block_manager.addInst(ret);
  return ret;
}

int LOrExpAST::cal_value() const {
  if (type == Exp)
    return and_exp->cal_value();
  return or_exp->cal_value() || and_exp->cal_value();
}

// NumberAST
NumberAST::NumberAST(int val) : val(val) {}

void *NumberAST::to_koopa() const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_INTEGER;
  ret->kind.data.integer.value = val;
  return ret;
}

int NumberAST::cal_value() const { return val; }
