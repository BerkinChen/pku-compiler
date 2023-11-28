#include "AST/AST.h"

// CompUnitAST
CompUnitAST::CompUnitAST(std::unique_ptr<BaseAST>& func_def)
    : func_def(std::move(func_def)) {}

std::string CompUnitAST::to_string() const {
  return "CompUnitAST { " + func_def->to_string() + " }";
}

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

std::string FuncDefAST::to_string() const {
  return "FuncDefAST { " + func_type->to_string() + ", " + ident + ", " +
         block->to_string() + " }";
}

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

std::string FuncTypeAST::to_string() const {
  return "FuncTypeAST { " + type + " }";
}

void* FuncTypeAST::to_koopa() const {
  if (type == "int") return type_kind(KOOPA_RTT_INT32);
  return nullptr;  // not implemented
}

// BlockAST
BlockAST::BlockAST(std::unique_ptr<BaseAST>& stmt) : stmt(std::move(stmt)) {}

std::string BlockAST::to_string() const {
  return "BlockAST { " + stmt->to_string() + " }";
}

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
StmtAST::StmtAST(std::unique_ptr<BaseAST>& ret_num)
    : ret_num(std::move(ret_num)) {}

std::string StmtAST::to_string() const {
  return "StmtAST { return, " + ret_num->to_string() + " }";
}

void* StmtAST::to_koopa() const {
  koopa_raw_value_data* ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_UNIT);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_RETURN;
  ret->kind.data.ret.value = (const koopa_raw_value_data*)ret_num->to_koopa();
  return ret;
}

// NumberAST
NumberAST::NumberAST(int val) : val(val) {}

std::string NumberAST::to_string() const {
  return "NumberAST { int " + std::to_string(val) + " }";
}

void* NumberAST::to_koopa() const {
  koopa_raw_value_data* ret = new koopa_raw_value_data();
  ret->ty = type_kind(KOOPA_RTT_INT32);
  ret->name = nullptr;
  ret->used_by = slice(KOOPA_RSIK_VALUE);
  ret->kind.tag = KOOPA_RVT_INTEGER;
  ret->kind.data.integer.value = val;
  return ret;
}
