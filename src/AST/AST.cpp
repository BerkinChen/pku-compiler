#include "AST/AST.h"
#include "AST.h"
#include <cstring>
#include <iostream>

// CompUnitAST
CompUnitAST::CompUnitAST(std::unique_ptr<BaseAST> &func_def)
    : func_def(std::move(func_def)) {}

void *CompUnitAST::to_koopa() const {
  std::vector<const void *> funcs{func_def->to_koopa()};
  koopa_raw_program_t *ret = new koopa_raw_program_t();
  ret->values = slice(KOOPA_RSIK_VALUE);
  ret->funcs = slice(funcs, KOOPA_RSIK_FUNCTION);

  return ret;
}

// FuncDefAST
FuncDefAST::FuncDefAST(std::unique_ptr<BaseAST> &func_type, const char *ident,
                       std::unique_ptr<BaseAST> &block)
    : func_type(std::move(func_type)), ident(ident), block(std::move(block)) {}

void *FuncDefAST::to_koopa() const {
  koopa_raw_function_data_t *ret = new koopa_raw_function_data_t();
  koopa_raw_type_kind_t *ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  ty->data.function.params = slice(KOOPA_RSIK_TYPE);
  ty->data.function.ret =
      (const struct koopa_raw_type_kind *)func_type->to_koopa();
  ret->ty = ty;

  char *name = new char[ident.length() + 1];
  ("@" + ident).copy(name, ident.length() + 1);
  name[ident.length() + 1] = '\0';
  ret->name = name;

  ret->params = slice(KOOPA_RSIK_VALUE);

  std::vector<const void *> blocks;
  block_manager.init(&blocks);
  koopa_raw_basic_block_data_t *entry = new koopa_raw_basic_block_data_t();
  entry->name = "%entry";
  entry->params = slice(KOOPA_RSIK_VALUE);
  entry->used_by = slice(KOOPA_RSIK_VALUE);
  block_manager.newBlock(entry);
  block->to_koopa();
  block_manager.delBlock();
  ret->bbs = slice(blocks, KOOPA_RSIK_BASIC_BLOCK);
  return ret;
}

// FuncTypeAST
FuncTypeAST::FuncTypeAST(const char *type) : type(type) {}

void *FuncTypeAST::to_koopa() const {
  if (type == "int")
    return (void *)type_kind(KOOPA_RTT_INT32);
  return nullptr; // not implemented
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
    symbol_list.newScope();
    for (auto blockitem = (*blockitem_vec).rbegin();
         blockitem != (*blockitem_vec).rend(); blockitem++) {
      (*blockitem)->to_koopa();
    }
    symbol_list.delScope();
    return nullptr;
  }
  return nullptr;
}

// StmtAST
StmtAST::StmtAST(StmtType type) : type(type) {}

StmtAST::StmtAST(std::unique_ptr<BaseAST> &exp, StmtType type)
    : type(type), exp(std::move(exp)) {}

StmtAST::StmtAST(std::unique_ptr<BaseAST> &lval, std::unique_ptr<BaseAST> &exp,
                 StmtType type)
    : type(type), exp(std::move(exp)), lval(std::move(lval)) {}

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
    ret->kind.data.store.dest = (koopa_raw_value_t)lval->to_left_value();
    ret->kind.data.store.value = (koopa_raw_value_t)exp->to_koopa();
    block_manager.addInst(ret);
  } else if (type == Exp) {
    exp->to_koopa();
  } else if (type == Block) {
    exp->to_koopa();
  } else if (type == If) {
    ret = (koopa_raw_value_data *)exp->to_koopa();
    bool true_check = block_manager.checkBlock();
    koopa_raw_basic_block_data_t *false_block =
        new koopa_raw_basic_block_data_t();
    false_block->name = "%false";
    false_block->params = slice(KOOPA_RSIK_VALUE);
    false_block->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.data.branch.false_bb = (koopa_raw_basic_block_t)false_block;
    ret->kind.data.branch.false_args = slice(KOOPA_RSIK_VALUE);
    if (lval != nullptr) {
      koopa_raw_basic_block_data_t *end_block =
          new koopa_raw_basic_block_data_t();
      end_block->name = "%end";
      end_block->params = slice(KOOPA_RSIK_VALUE);
      end_block->used_by = slice(KOOPA_RSIK_VALUE);
      if (!true_check)
        block_manager.addInst(jump_value(end_block));
      block_manager.newBlock(false_block);
      lval->to_koopa();
      bool false_check = block_manager.checkBlock();
      if (!false_check)
        block_manager.addInst(jump_value(end_block));
      if (!true_check || !false_check)
        block_manager.newBlock(end_block);
    } else {
      if (!true_check)
        block_manager.addInst(jump_value(false_block));
      block_manager.newBlock(false_block);
    }
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
  for (auto const_def = (*ConstDef_vec).rbegin();
       const_def != (*ConstDef_vec).rend(); const_def++) {
    (*const_def)->to_koopa(type);
  }
  return nullptr;
}

// BTypeAST
BTypeAST::BTypeAST(const char *type) : type(type) {}

void *BTypeAST::to_koopa() const {
  if (type == "int")
    return (void *)type_kind(KOOPA_RTT_INT32);
  return nullptr; // not implemented
}

// ConstDefAST
ConstDefAST::ConstDefAST(const char *ident, std::unique_ptr<BaseAST> &exp)
    : ident(ident), exp(std::move(exp)) {}

void *ConstDefAST::to_koopa(koopa_raw_type_t const_type) const {
  int val = exp->cal_value();
  Value value = Value(ValueType::Const, val);
  symbol_list.addSymbol(ident.c_str(), value);
  return nullptr;
}

// VarDeclAST
VarDeclAST::VarDeclAST(
    std::unique_ptr<BaseAST> &var_type,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &VarDef_vec)
    : var_type(std::move(var_type)), VarDef_vec(std::move(VarDef_vec)) {}

void *VarDeclAST::to_koopa() const {
  koopa_raw_type_t type = (const koopa_raw_type_t)var_type->to_koopa();
  for (auto var_def = (*VarDef_vec).rbegin(); var_def != (*VarDef_vec).rend();
       var_def++) {
    (*var_def)->to_koopa(type);
  }
  return nullptr;
}

// VarDefAST
VarDefAST::VarDefAST(const char *ident, std::unique_ptr<BaseAST> &exp)
    : ident(ident), exp(std::move(exp)) {
  type = Exp;
}

VarDefAST::VarDefAST(const char *ident) : ident(ident) { type = Zero; }

void *VarDefAST::to_koopa(koopa_raw_type_t var_type) const {
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
  if (type == Exp) {
    koopa_raw_value_data *store = new koopa_raw_value_data();
    store->ty = type_kind(KOOPA_RTT_INT32);
    store->name = nullptr;
    store->used_by = slice(KOOPA_RSIK_VALUE);
    store->kind.tag = KOOPA_RVT_STORE;
    store->kind.data.store.dest = (koopa_raw_value_t)ret;
    store->kind.data.store.value = (koopa_raw_value_t)exp->to_koopa();
    block_manager.addInst(store);
  }
  return nullptr;
}

// LValAST
LValAST::LValAST(const char *ident) : ident(ident) {}

void *LValAST::to_left_value() const {
  return (void *)symbol_list.getSymbol(ident).data.var_value;
}

void *LValAST::to_koopa() const {
  Value value = symbol_list.getSymbol(ident);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  // std::cout << "value: " << value.type << std::endl;
  if (value.type == ValueType::Var) {
    ret->kind.tag = KOOPA_RVT_LOAD;
    ret->kind.data.load.src = (koopa_raw_value_t)value.data.var_value;
    block_manager.addInst(ret);
  } else if (value.type == ValueType::Const) {
    ret->kind.tag = KOOPA_RVT_INTEGER;
    ret->kind.data.integer.value = value.data.const_value;
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

void *UnaryExpAST::to_koopa() const {
  if (type == Exp || op == "+")
    return exp->to_koopa();
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
  false_block->name = "%ebd";
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
  ret->kind.data.integer.value = val % 256;
  return ret;
}

int NumberAST::cal_value() const { return val % 256; }
