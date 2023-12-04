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

  std::vector<const void*> stmts{stmt->to_koopa()};
  ret->insts = slice(stmts, KOOPA_RSIK_VALUE);

  return ret;
}

// StmtAST
StmtAST::StmtAST(std::unique_ptr<BaseAST>& exp) : exp(std::move(exp)) {}

void* StmtAST::to_koopa() const {
  koopa_raw_value_data* ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_UNIT);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_RETURN;
  ret->kind.data.ret.value = (const koopa_raw_value_data*)exp->to_koopa();
  return ret;
}

// ExpAST
ExpAST::ExpAST(std::unique_ptr<BaseAST>& unary_exp)
    : unary_exp(std::move(unary_exp)) {}

void* ExpAST::to_koopa() const { return unary_exp->to_koopa(); }

// PrimaryExpAST
PrimaryExpAST::PrimaryExpAST(std::unique_ptr<BaseAST>& exp) : exp(std::move(exp)) {}

void* PrimaryExpAST::to_koopa() const { return exp->to_koopa(); }

// UnaryExpAST
UnaryExpAST::UnaryExpAST(std::unique_ptr<BaseAST>& exp)
    : exp(std::move(exp)) {type = Exp;}

UnaryExpAST::UnaryExpAST(const char* op, std::unique_ptr<BaseAST>& exp)
    : op(op), exp(std::move(exp)) {type = Op;}

void* UnaryExpAST::to_koopa() const {
  if (type == Exp) return exp->to_koopa();
  if (op == "+") {
    koopa_raw_value_data* ret = new koopa_raw_value_data();
    ret->ty = type_kind(KOOPA_RTT_INT32);
    ret->name = nullptr;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_INTEGER;
    ret->kind.data.integer.value = (((const koopa_raw_value_data*)exp->to_koopa())->kind.data.integer.value);
    return ret;
  }
  if (op == "-") {
    koopa_raw_value_data* ret = new koopa_raw_value_data();
    ret->ty = type_kind(KOOPA_RTT_INT32);
    ret->name = nullptr;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_INTEGER;
    ret->kind.data.integer.value = -(((const koopa_raw_value_data*)exp->to_koopa())->kind.data.integer.value);
    return ret;
  }
  if (op == "!") {
    koopa_raw_value_data* ret = new koopa_raw_value_data();
    ret->ty = type_kind(KOOPA_RTT_INT32);
    ret->name = nullptr;
    ret->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.tag = KOOPA_RVT_INTEGER;
    ret->kind.data.integer.value = !(((const koopa_raw_value_data*)exp->to_koopa())->kind.data.integer.value);
    return ret;
  }
  return nullptr;  // not implemented
}

// NumberAST
NumberAST::NumberAST(int val) : val(val) {}

void* NumberAST::to_koopa() const {
  koopa_raw_value_data* ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_INTEGER;
  ret->kind.data.integer.value = val;
  return ret;
}
