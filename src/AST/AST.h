#ifndef AST_H
#define AST_H

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "utils.h"

static SymbolList symbol_list;
static BlockManager block_manager;
static LoopManager loop_manager;
class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void *to_left_value() const { return nullptr; }
  virtual void *to_koopa() const { return nullptr; }
  virtual void *to_koopa(int index) const { return nullptr; }
  virtual void *to_koopa(std::vector<const void *> &global_var) const {
    return nullptr;
  }
  virtual void *to_koopa(koopa_raw_basic_block_t end_block) const {
    return nullptr;
  }
  virtual void *to_koopa(koopa_raw_type_t type) const { return nullptr; }
  virtual void *to_koopa(std::vector<const void *> &func,
                         std::vector<const void *> &value) const {
    return nullptr;
  }
  virtual void *to_koopa(std::vector<const void *> &global_var,
                         koopa_raw_type_t type) const {
    return nullptr;
  }
  virtual void *to_koopa(std::vector<const void *> &init_list, std::vector<size_t> size_vec, int level) const {
    return nullptr;
  }
  virtual int cal_value() const { assert(false); }
};

class CompUnitAST : public BaseAST {
public:
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> def_vec;
  CompUnitAST(std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &def_vec);
  void load_lib_func(std::vector<const void *> &lib_func_vec) const;
  void *to_koopa() const override;
};

class DefAST : public BaseAST {
public:
  enum DefType { FuncDef, ConstDef, VarDef };
  DefType type;
  std::unique_ptr<BaseAST> def;
  DefAST(std::unique_ptr<BaseAST> &def, DefType type);
  void *to_koopa(std::vector<const void *> &func,
                 std::vector<const void *> &value) const override;
};

class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> param_vec;

  FuncDefAST(std::unique_ptr<BaseAST> &func_type, const char *ident,
             std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &param_vec,
             std::unique_ptr<BaseAST> &block);

  void *to_koopa() const override;
};

class GlobalConstDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> const_type;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> ConstDef_vec;
  GlobalConstDefAST(
      std::unique_ptr<BaseAST> &const_type,
      std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &ConstDef_vec);
  void *to_koopa() const override;
};

class GlobalVarDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> var_type;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> VarDef_vec;
  GlobalVarDefAST(
      std::unique_ptr<BaseAST> &var_type,
      std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &VarDef_vec);
  void *to_koopa(std::vector<const void *> &global_var) const override;
};

class FuncFParamAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> param_type;
  std::string ident;
  FuncFParamAST(std::unique_ptr<BaseAST> &param_type, const char *ident);
  void *to_koopa() const override;
  void *to_koopa(int index) const override;
};
class BlockAST : public BaseAST {
public:
  enum { Item, Empty } type;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> blockitem_vec;
  BlockAST();
  BlockAST(
      std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &blockitem_vec);
  void *to_koopa() const override;
};

class StmtAST : public BaseAST {
public:
  enum StmtType {
    Exp,
    Assign,
    Block,
    Return,
    Empty,
    If,
    While,
    Break,
    Continue
  };
  StmtType type;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> stmt;
  StmtAST(StmtType type);
  StmtAST(std::unique_ptr<BaseAST> &exp, StmtType type);
  StmtAST(std::unique_ptr<BaseAST> &stmt, std::unique_ptr<BaseAST> &exp,
          StmtType type);
  void *to_koopa() const override;
};

class IfAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> stmt;
  IfAST(std::unique_ptr<BaseAST> &exp, std::unique_ptr<BaseAST> &stmt);
  void *to_koopa() const override;
};

class ConstDeclAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> const_type;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> ConstDef_vec;
  ConstDeclAST(
      std::unique_ptr<BaseAST> &const_type,
      std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &ConstDef_vec);
  void *to_koopa() const override;
  void *to_koopa(std::vector<const void *> &global_var) const override;
};

class TypeAST : public BaseAST {
public:
  std::string type;
  TypeAST(const char *type);
  void *to_koopa() const override;
};

class ConstDefAST : public BaseAST {
public:
  enum { Var, Array } type;
  std::string ident;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> index_array;
  ConstDefAST(const char *ident, std::unique_ptr<BaseAST> &init_array);
  ConstDefAST(
      const char *ident,
      std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array,
      std::unique_ptr<BaseAST> &init_array);
  void *to_koopa(koopa_raw_type_t type) const override;
  void *to_koopa(std::vector<const void *> &global_var,
                 koopa_raw_type_t type) const override;
};

class VarDeclAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> var_type;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> VarDef_vec;
  VarDeclAST(
      std::unique_ptr<BaseAST> &var_type,
      std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &VarDef_vec);
  void *to_koopa() const override;
  void *to_koopa(std::vector<const void *> &global_var) const override;
};

class VarDefAST : public BaseAST {
public:
  enum VarDefType { Exp, Array };
  VarDefType type;
  std::string ident;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> index_array;
  VarDefAST(const char *ident, std::unique_ptr<BaseAST> &exp, VarDefType type);
  VarDefAST(const char *ident,
            std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array,
            VarDefType type);
  VarDefAST(const char *ident, VarDefType type);
  VarDefAST(const char *ident,
            std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array,
            std::unique_ptr<BaseAST> &exp, VarDefType type);
  void *to_koopa(koopa_raw_type_t type) const override;
  void *to_koopa(std::vector<const void *> &global_var,
                 koopa_raw_type_t type) const override;
};

class InitValAST : public BaseAST {
public:
  enum { Exp, InitList, Empty } type;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> initlist_vec;
  InitValAST();
  InitValAST(std::unique_ptr<BaseAST> &exp);
  InitValAST(
      std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &initlist_vec);
  void *to_koopa(std::vector<const void *> &init_vec, std::vector<size_t> size_vec, int level) const override;
  void *to_koopa() const override;
  int cal_value() const override;
  void preprocess(std::vector<const void *> &init_vec,
                  std::vector<size_t> size_vec);
};

class LValAST : public BaseAST {
public:
  std::string ident;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> index_array;
  LValAST(const char *ident);
  LValAST(const char *ident,
          std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &index_array);
  void *to_left_value() const override;
  void *to_koopa() const override;
  int cal_value() const override;
};

class ExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> add_exp;
  ExpAST(std::unique_ptr<BaseAST> &add_exp);
  void *to_koopa() const override;
  int cal_value() const override;
};

class PrimaryExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> exp;
  PrimaryExpAST(std::unique_ptr<BaseAST> &exp);
  void *to_koopa() const override;
  int cal_value() const override;
};

class UnaryExpAST : public BaseAST {
public:
  enum { Exp, Op, Call } type;
  std::string op;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> args;
  UnaryExpAST(std::unique_ptr<BaseAST> &exp);
  UnaryExpAST(const char *op, std::unique_ptr<BaseAST> &exp);
  UnaryExpAST(const char *op,
              std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> &args);
  void *to_koopa() const override;
  int cal_value() const override;
};

class AddExpAST : public BaseAST {
public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> add_exp;
  std::unique_ptr<BaseAST> mul_exp;
  AddExpAST(std::unique_ptr<BaseAST> &add_exp);
  AddExpAST(const char *op, std::unique_ptr<BaseAST> &add_exp,
            std::unique_ptr<BaseAST> &mul_exp);
  void *to_koopa() const override;
  int cal_value() const override;
};

class MulExpAST : public BaseAST {
public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> mul_exp;
  std::unique_ptr<BaseAST> unary_exp;
  MulExpAST(std::unique_ptr<BaseAST> &unary_exp);
  MulExpAST(const char *op, std::unique_ptr<BaseAST> &mul_exp,
            std::unique_ptr<BaseAST> &unary_exp);
  void *to_koopa() const override;
  int cal_value() const override;
};

class RelExpAST : public BaseAST {
public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> rel_exp;
  std::unique_ptr<BaseAST> add_exp;
  RelExpAST(std::unique_ptr<BaseAST> &add_exp);
  RelExpAST(const char *op, std::unique_ptr<BaseAST> &rel_exp,
            std::unique_ptr<BaseAST> &add_exp);
  void *to_koopa() const override;
  int cal_value() const override;
};

class EqExpAST : public BaseAST {
public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> eq_exp;
  std::unique_ptr<BaseAST> rel_exp;
  EqExpAST(std::unique_ptr<BaseAST> &rel_exp);
  EqExpAST(const char *op, std::unique_ptr<BaseAST> &eq_exp,
           std::unique_ptr<BaseAST> &rel_exp);
  void *to_koopa() const override;
  int cal_value() const override;
};

class LAndExpAST : public BaseAST {
public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> and_exp;
  std::unique_ptr<BaseAST> eq_exp;
  LAndExpAST(std::unique_ptr<BaseAST> &eq_exp);
  LAndExpAST(const char *op, std::unique_ptr<BaseAST> &and_exp,
             std::unique_ptr<BaseAST> &eq_exp);
  void *make_bool(const std::unique_ptr<BaseAST> &exp) const;
  void *to_koopa() const override;
  int cal_value() const override;
};

class LOrExpAST : public BaseAST {
public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> or_exp;
  std::unique_ptr<BaseAST> and_exp;
  LOrExpAST(std::unique_ptr<BaseAST> &and_exp);
  LOrExpAST(const char *op, std::unique_ptr<BaseAST> &or_exp,
            std::unique_ptr<BaseAST> &and_exp);
  void *make_bool(const std::unique_ptr<BaseAST> &exp) const;
  void *to_koopa() const override;
  int cal_value() const override;
};
class NumberAST : public BaseAST {
public:
  int val;
  NumberAST(int val);
  void *to_koopa() const override;
  int cal_value() const override;
};

#endif // AST_H