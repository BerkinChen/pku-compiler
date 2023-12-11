#include "AST/AST.h"

// CompUnitAST
CompUnitAST::CompUnitAST(std::unique_ptr<BaseAST>& func_def)
    : func_def(std::move(func_def)) {}

void* CompUnitAST::to_koopa() const {
  std::vector<const void*> funcs{func_def->to_koopa()};

  koopa_raw_program_t* ret = new koopa_raw_program_t();
  ret->values = slice(KOOPA_RSIK_VALUE);
  ret->funcs = slice(funcs, KOOPA_RSIK_FUNCTION);

  return ret;
}

// FuncDefAST
FuncDefAST::FuncDefAST(std::unique_ptr<BaseAST>& func_type, const char* ident,
                       std::unique_ptr<BaseAST>& block)
    : func_type(std::move(func_type)), ident(ident), block(std::move(block)) {}

void* FuncDefAST::to_koopa() const {
  koopa_raw_function_data_t* ret = new koopa_raw_function_data_t();

  koopa_raw_type_kind_t* ty = new koopa_raw_type_kind_t();
  ty->tag = KOOPA_RTT_FUNCTION;
  ty->data.function.params = slice(KOOPA_RSIK_TYPE);
  ty->data.function.ret =
      (const struct koopa_raw_type_kind*)func_type->to_koopa();
  ret->ty = ty;

  char* name = new char[ident.length() + 1];
  ("@" + ident).copy(name, ident.length() + 1);
  ret->name = name;

  ret->params = slice(KOOPA_RSIK_VALUE);

  std::vector<const void*> blocks{block->to_koopa()};
  ret->bbs = slice(blocks, KOOPA_RSIK_BASIC_BLOCK);

  return ret;
}

// FuncTypeAST
FuncTypeAST::FuncTypeAST(const char* type) : type(type) {}

void* FuncTypeAST::to_koopa() const {
  if (type == "int") return (void*)type_kind(KOOPA_RTT_INT32);
  return nullptr;  // not implemented
}

// BlockAST
BlockAST::BlockAST(std::unique_ptr<BaseAST>& stmt) : stmt(std::move(stmt)) {}

void* BlockAST::to_koopa() const {
  koopa_raw_basic_block_data_t* ret = new koopa_raw_basic_block_data_t();
  ret->name = "%entry";
  ret->params = slice(KOOPA_RSIK_VALUE);
  ret->used_by = slice(KOOPA_RSIK_VALUE);

  std::vector<const void*> stmts;
  stmt->to_koopa(stmts);
  ret->insts = slice(stmts, KOOPA_RSIK_VALUE);

  return ret;
}

// StmtAST
StmtAST::StmtAST(std::unique_ptr<BaseAST>& exp) : exp(std::move(exp)) {}

void* StmtAST::to_koopa(std::vector<const void*>& inst_buf) const {
  koopa_raw_value_data* ret = new koopa_raw_value_data();
  koopa_raw_slice_t used_by = slice(ret, KOOPA_RSIK_VALUE);
  ret->ty = type_kind(KOOPA_RTT_UNIT);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_RETURN;
  ret->kind.data.ret.value =
      (const koopa_raw_value_data*)exp->to_koopa(used_by, inst_buf);
  inst_buf.push_back(ret);
  return ret;
}

// ExpAST
ExpAST::ExpAST(std::unique_ptr<BaseAST>& add_exp)
    : add_exp(std::move(add_exp)) {}

void* ExpAST::to_koopa(koopa_raw_slice_t parent,
                       std::vector<const void*>& inst_buf) const {
  return add_exp->to_koopa(parent, inst_buf);
}

// PrimaryExpAST
PrimaryExpAST::PrimaryExpAST(std::unique_ptr<BaseAST>& exp)
    : exp(std::move(exp)) {}

void* PrimaryExpAST::to_koopa(koopa_raw_slice_t parent,
                              std::vector<const void*>& inst_buf) const {
  return exp->to_koopa(parent, inst_buf);
}

// UnaryExpAST
UnaryExpAST::UnaryExpAST(std::unique_ptr<BaseAST>& exp) : exp(std::move(exp)) {
  type = Exp;
}

UnaryExpAST::UnaryExpAST(const char* op, std::unique_ptr<BaseAST>& exp)
    : op(op), exp(std::move(exp)) {
  type = Op;
}

void* UnaryExpAST::to_koopa(koopa_raw_slice_t parent,
                            std::vector<const void*>& inst_buf) const {
  if (type == Exp || op == "+") return exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data* ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_INTEGER;
  if (op == "-") {
    ret->kind.data.integer.value =
        -(((const koopa_raw_value_data*)exp->to_koopa(parent, inst_buf))
              ->kind.data.integer.value);
  }
  if (op == "!") {
    ret->kind.data.integer.value =
        !(((const koopa_raw_value_data*)exp->to_koopa(parent, inst_buf))
              ->kind.data.integer.value);
  }
  return ret;
}

// AddExpAST
AddExpAST::AddExpAST(std::unique_ptr<BaseAST>& mul_exp)
    : mul_exp(std::move(mul_exp)) {
  type = Exp;
}

AddExpAST::AddExpAST(const char* op, std::unique_ptr<BaseAST>& add_exp,
                     std::unique_ptr<BaseAST>& mul_exp)
    : op(op), add_exp(std::move(add_exp)), mul_exp(std::move(mul_exp)) {
  type = Op;
}

void* AddExpAST::to_koopa(koopa_raw_slice_t parent,
                          std::vector<const void*>& inst_buf) const {
  if (type == Exp) return mul_exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data* ret = new koopa_raw_value_data();
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

// MulExpAST
MulExpAST::MulExpAST(std::unique_ptr<BaseAST>& unary_exp)
    : unary_exp(std::move(unary_exp)) {
  type = Exp;
}

MulExpAST::MulExpAST(const char* op, std::unique_ptr<BaseAST>& mul_exp,
                     std::unique_ptr<BaseAST>& unary_exp)
    : op(op), mul_exp(std::move(mul_exp)), unary_exp(std::move(unary_exp)) {
  type = Op;
}

void* MulExpAST::to_koopa(koopa_raw_slice_t parent,
                          std::vector<const void*>& inst_buf) const {
  if (type == Exp) return unary_exp->to_koopa(parent, inst_buf);
  koopa_raw_value_data* ret = new koopa_raw_value_data();
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

// NumberAST
NumberAST::NumberAST(int val) : val(val) {}

void* NumberAST::to_koopa(koopa_raw_slice_t parent,
                          std::vector<const void*>& inst_buf) const {
  koopa_raw_value_data* ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = parent;
  ret->kind.tag = KOOPA_RVT_INTEGER;
  ret->kind.data.integer.value = val;
  return ret;
}
