#include "AST/AST.h"
#include "AST.h"
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
  ret->name = name;

  ret->params = slice(KOOPA_RSIK_VALUE);

  std::vector<const void *> blocks{block->to_koopa()};
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
BlockAST::BlockAST(
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &blockitem_vec)
    : blockitem_vec(std::move(blockitem_vec)) {}

void *BlockAST::to_koopa() const {
  std::unique_ptr<SymbolList> symbol_list = std::make_unique<SymbolList>();
  symbol_list->init();
  koopa_raw_basic_block_data_t *ret = new koopa_raw_basic_block_data_t();
  ret->name = "%entry";
  ret->params = slice(KOOPA_RSIK_VALUE);
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  std::vector<const void *> items;
  for (auto blockitem = (*blockitem_vec).end() - 1;
       blockitem != (*blockitem_vec).begin() - 1; blockitem--) {
    (*blockitem)->to_koopa(items);
  }
  ret->insts = slice(items, KOOPA_RSIK_VALUE);

  return ret;
}

// StmtAST
StmtAST::StmtAST(std::unique_ptr<BaseAST> &exp) : exp(std::move(exp)) {
  type = Exp;
}

StmtAST::StmtAST(std::unique_ptr<BaseAST> &lval, std::unique_ptr<BaseAST> &exp)
    : exp(std::move(exp)), lval(std::move(lval)) {
  type = Assign;
}

void *StmtAST::to_koopa(std::vector<const void *> &inst_buf) const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_UNIT);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  if (type == Exp) {
    ret->kind.tag = KOOPA_RVT_RETURN;
    ret->kind.data.ret.value =
        (const koopa_raw_value_data *)exp->to_koopa(used_by, inst_buf);
  } else if (type == Assign) {
    ret->kind.tag = KOOPA_RVT_STORE;
    ret->kind.data.store.dest =
        (koopa_raw_value_t)lval->to_koopa();
    ret->kind.data.store.value =
        (koopa_raw_value_t)exp->to_koopa(used_by, inst_buf);
  }
  inst_buf.push_back(ret);
  return ret;
}

// ConstDeclAST
ConstDeclAST::ConstDeclAST(
    std::unique_ptr<BaseAST> &const_type,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &ConstDef_vec)
    : const_type(std::move(const_type)), ConstDef_vec(std::move(ConstDef_vec)) {
}

void *ConstDeclAST::to_koopa(std::vector<const void *> &inst_buf) const {
  koopa_raw_type_t type = (const koopa_raw_type_t)const_type->to_koopa();
  for (auto const_def = (*ConstDef_vec).end() - 1;
       const_def >= (*ConstDef_vec).begin(); const_def--) {
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
  // std::cout << "name: " << name <<" value: " <<
  // symbol_list.getSymbol(name).value << std::endl;
  return nullptr;
}

// VarDeclAST
VarDeclAST::VarDeclAST(
    std::unique_ptr<BaseAST> &var_type,
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &VarDef_vec)
    : var_type(std::move(var_type)), VarDef_vec(std::move(VarDef_vec)) {}

void *VarDeclAST::to_koopa(std::vector<const void *> &inst_buf) const {
  koopa_raw_type_t type = (const koopa_raw_type_t)var_type->to_koopa();
  for (auto var_def = (*VarDef_vec).end() - 1; var_def >= (*VarDef_vec).begin();
       var_def--) {
    (*var_def)->to_koopa(inst_buf, type);
  }
  return nullptr;
}

// VarDefAST
VarDefAST::VarDefAST(const char *ident, std::unique_ptr<BaseAST> &exp)
    : ident(ident), exp(std::move(exp)) {
  type = Exp;
}

VarDefAST::VarDefAST(const char *ident) : ident(ident) { type = Zero; }

void *VarDefAST::to_koopa(std::vector<const void *> &inst_buf,
                          koopa_raw_type_t var_type) const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = pointer_type_kind(KOOPA_RTT_INT32);
  char *name = new char[ident.length() + 1];
  ("@" + ident).copy(name, ident.length() + 1);
  ret->name = name;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_ALLOC;
  inst_buf.push_back(ret);
  Value value = Value(ValueType::Var, ret);
  symbol_list.addSymbol(ident.c_str(), value);
  if (type == Exp) {
    koopa_raw_value_data *store = new koopa_raw_value_data();
    store->name = nullptr;
    store->used_by = slice(KOOPA_RSIK_VALUE);
    store->kind.tag = KOOPA_RVT_STORE;
    store->kind.data.store.dest = (koopa_raw_value_t)ret;
    store->kind.data.store.value =
        (koopa_raw_value_t)exp->to_koopa(used_by, inst_buf);
    inst_buf.push_back(store);
  }
  // std::cout << "name: " << name <<" value: " <<
  // symbol_list.getSymbol(name).value << std::endl;
  return nullptr;
}

// LValAST
LValAST::LValAST(const char *ident) : ident(ident) {}

void *LValAST::to_koopa() const {
  return (void *)symbol_list.getSymbol(ident).data.var_value;
}

void *LValAST::to_koopa(koopa_raw_slice_t parent,
                        std::vector<const void *> &inst_buf) const {
  Value value = symbol_list.getSymbol(ident);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  //std::cout << "value: " << value.type << std::endl;
  if (value.type == ValueType::Var) {
    ret->kind.tag = KOOPA_RVT_LOAD;
    ret->kind.data.load.src = (koopa_raw_value_t)value.data.var_value;
    inst_buf.push_back(ret);
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

void *ExpAST::to_koopa(koopa_raw_slice_t parent,
                       std::vector<const void *> &inst_buf) const {
  return add_exp->to_koopa(parent, inst_buf);
}

int ExpAST::cal_value() const { return add_exp->cal_value(); }

// PrimaryExpAST
PrimaryExpAST::PrimaryExpAST(std::unique_ptr<BaseAST> &exp)
    : exp(std::move(exp)) {}

void *PrimaryExpAST::to_koopa(koopa_raw_slice_t parent,
                              std::vector<const void *> &inst_buf) const {
  return exp->to_koopa(parent, inst_buf);
}

int PrimaryExpAST::cal_value() const { return exp->cal_value(); }

// UnaryExpAST
UnaryExpAST::UnaryExpAST(std::unique_ptr<BaseAST> &exp) : exp(std::move(exp)) {
  type = Exp;
}

UnaryExpAST::UnaryExpAST(const char *op, std::unique_ptr<BaseAST> &exp)
    : op(op), exp(std::move(exp)) {
  type = Op;
}

void *UnaryExpAST::to_koopa(koopa_raw_slice_t parent,
                            std::vector<const void *> &inst_buf) const {
  if (type == Exp || op == "+")
    return exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "-") {
    binary.op = KOOPA_RBO_SUB;
  }
  if (op == "!") {
    binary.op = KOOPA_RBO_EQ;
  }
  NumberAST zero(0);
  binary.lhs = (koopa_raw_value_t)zero.to_koopa(used_by, inst_buf);
  binary.rhs = (koopa_raw_value_t)exp->to_koopa(used_by, inst_buf);
  inst_buf.push_back(ret);
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

void *AddExpAST::to_koopa(koopa_raw_slice_t parent,
                          std::vector<const void *> &inst_buf) const {
  if (type == Exp)
    return mul_exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "+") {
    binary.op = KOOPA_RBO_ADD;
  }
  if (op == "-") {
    binary.op = KOOPA_RBO_SUB;
  }
  binary.lhs = (koopa_raw_value_t)add_exp->to_koopa(used_by, inst_buf);
  binary.rhs = (koopa_raw_value_t)mul_exp->to_koopa(used_by, inst_buf);
  inst_buf.push_back(ret);
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

void *MulExpAST::to_koopa(koopa_raw_slice_t parent,
                          std::vector<const void *> &inst_buf) const {
  if (type == Exp)
    return unary_exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
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
  binary.lhs = (koopa_raw_value_t)mul_exp->to_koopa(used_by, inst_buf);
  binary.rhs = (koopa_raw_value_t)unary_exp->to_koopa(used_by, inst_buf);
  inst_buf.push_back(ret);
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

void *RelExpAST::to_koopa(koopa_raw_slice_t parent,
                          std::vector<const void *> &inst_buf) const {
  if (type == Exp)
    return add_exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
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
  binary.lhs = (koopa_raw_value_t)rel_exp->to_koopa(used_by, inst_buf);
  binary.rhs = (koopa_raw_value_t)add_exp->to_koopa(used_by, inst_buf);
  inst_buf.push_back(ret);
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

void *EqExpAST::to_koopa(koopa_raw_slice_t parent,
                         std::vector<const void *> &inst_buf) const {
  if (type == Exp)
    return rel_exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "==") {
    binary.op = KOOPA_RBO_EQ;
  }
  if (op == "!=") {
    binary.op = KOOPA_RBO_NOT_EQ;
  }
  binary.lhs = (koopa_raw_value_t)eq_exp->to_koopa(used_by, inst_buf);
  binary.rhs = (koopa_raw_value_t)rel_exp->to_koopa(used_by, inst_buf);
  inst_buf.push_back(ret);
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

void *LAndExpAST::make_bool(koopa_raw_slice_t parent,
                            std::vector<const void *> &inst_buf,
                            const std::unique_ptr<BaseAST> &exp) const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  binary.op = KOOPA_RBO_NOT_EQ;
  binary.lhs = (koopa_raw_value_t)exp->to_koopa(used_by, inst_buf);
  NumberAST zero(0);
  binary.rhs = (koopa_raw_value_t)zero.to_koopa(used_by, inst_buf);
  inst_buf.push_back(ret);
  return ret;
}

void *LAndExpAST::to_koopa(koopa_raw_slice_t parent,
                           std::vector<const void *> &inst_buf) const {
  if (type == Exp)
    return eq_exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "&&") {
    binary.op = KOOPA_RBO_AND;
  }
  binary.lhs = (koopa_raw_value_t)make_bool(used_by, inst_buf, and_exp);
  binary.rhs = (koopa_raw_value_t)make_bool(used_by, inst_buf, eq_exp);
  inst_buf.push_back(ret);
  return ret;
}

int LAndExpAST::cal_value() const {
  if (type == Exp)
    return eq_exp->cal_value();
  if (op == "&&")
    return and_exp->cal_value() && eq_exp->cal_value();
  return 0;
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

void *LOrExpAST::make_bool(koopa_raw_slice_t parent,
                           std::vector<const void *> &inst_buf,
                           const std::unique_ptr<BaseAST> &exp) const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  binary.op = KOOPA_RBO_NOT_EQ;
  binary.lhs = (koopa_raw_value_t)exp->to_koopa(used_by, inst_buf);
  NumberAST zero(0);
  binary.rhs = (koopa_raw_value_t)zero.to_koopa(used_by, inst_buf);
  inst_buf.push_back(ret);
  return ret;
}

void *LOrExpAST::to_koopa(koopa_raw_slice_t parent,
                          std::vector<const void *> &inst_buf) const {
  if (type == Exp)
    return and_exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_BINARY;
  auto &binary = ret->kind.data.binary;
  if (op == "||") {
    binary.op = KOOPA_RBO_OR;
  }
  binary.lhs = (koopa_raw_value_t)make_bool(used_by, inst_buf, or_exp);
  binary.rhs = (koopa_raw_value_t)make_bool(used_by, inst_buf, and_exp);
  inst_buf.push_back(ret);
  return ret;
}

int LOrExpAST::cal_value() const {
  if (type == Exp)
    return and_exp->cal_value();
  if (op == "||")
    return or_exp->cal_value() || and_exp->cal_value();
  return 0;
}

// NumberAST
NumberAST::NumberAST(int val) : val(val) {}

void *NumberAST::to_koopa(koopa_raw_slice_t parent,
                          std::vector<const void *> &inst_buf) const {
  koopa_raw_value_data *ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_INTEGER;
  ret->kind.data.integer.value = val % 256;
  return ret;
}

int NumberAST::cal_value() const { return val % 256; }
