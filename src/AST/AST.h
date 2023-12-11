#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

#include "utils.h"

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void* to_koopa() const { return nullptr; }
  virtual void* to_koopa(std::vector<const void*>& inst_buf) const {
    return nullptr;
  }
  virtual void* to_koopa(koopa_raw_slice_t parent) const { return nullptr; }
  virtual void* to_koopa(koopa_raw_slice_t parent,
                         std::vector<const void*>& inst_buf) const {
    return nullptr;
  }
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

  CompUnitAST(std::unique_ptr<BaseAST>& func_def);

  void* to_koopa() const override;
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  FuncDefAST(std::unique_ptr<BaseAST>& func_type, const char* ident,
             std::unique_ptr<BaseAST>& block);

  void* to_koopa() const override;
};

class FuncTypeAST : public BaseAST {
 public:
  std::string type;
  FuncTypeAST(const char* type);
  void* to_koopa() const override;
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;
  BlockAST(std::unique_ptr<BaseAST>& stmt);
  void* to_koopa() const override;
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  StmtAST(std::unique_ptr<BaseAST>& exp);
  void* to_koopa(std::vector<const void*>& inst_buf) const override;
};

class ExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> add_exp;
  ExpAST(std::unique_ptr<BaseAST>& add_exp);
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

class PrimaryExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  PrimaryExpAST(std::unique_ptr<BaseAST>& exp);
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

class UnaryExpAST : public BaseAST {
 public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> exp;
  UnaryExpAST(std::unique_ptr<BaseAST>& exp);
  UnaryExpAST(const char* op, std::unique_ptr<BaseAST>& exp);
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

class AddExpAST : public BaseAST {
 public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> add_exp;
  std::unique_ptr<BaseAST> mul_exp;
  AddExpAST(std::unique_ptr<BaseAST>& add_exp);
  AddExpAST(const char* op, std::unique_ptr<BaseAST>& add_exp,
            std::unique_ptr<BaseAST>& mul_exp);
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

class MulExpAST : public BaseAST {
 public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> mul_exp;
  std::unique_ptr<BaseAST> unary_exp;
  MulExpAST(std::unique_ptr<BaseAST>& unary_exp);
  MulExpAST(const char* op, std::unique_ptr<BaseAST>& mul_exp,
            std::unique_ptr<BaseAST>& unary_exp);
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

class RelExpAST : public BaseAST {
 public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> rel_exp;
  std::unique_ptr<BaseAST> add_exp;
  RelExpAST(std::unique_ptr<BaseAST>& add_exp);
  RelExpAST(const char* op, std::unique_ptr<BaseAST>& rel_exp,
            std::unique_ptr<BaseAST>& add_exp);
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

class EqExpAST : public BaseAST {
 public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> eq_exp;
  std::unique_ptr<BaseAST> rel_exp;
  EqExpAST(std::unique_ptr<BaseAST>& rel_exp);
  EqExpAST(const char* op, std::unique_ptr<BaseAST>& eq_exp,
           std::unique_ptr<BaseAST>& rel_exp);
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

class LAndExpAST : public BaseAST {
 public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> and_exp;
  std::unique_ptr<BaseAST> eq_exp;
  LAndExpAST(std::unique_ptr<BaseAST>& eq_exp);
  LAndExpAST(const char* op, std::unique_ptr<BaseAST>& and_exp,
             std::unique_ptr<BaseAST>& eq_exp);
  void* make_bool(koopa_raw_slice_t parent,
                  std::vector<const void*>& inst_buf, koopa_raw_value_t exp) const;
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

class LOrExpAST : public BaseAST {
 public:
  enum { Exp, Op } type;
  std::string op;
  std::unique_ptr<BaseAST> or_exp;
  std::unique_ptr<BaseAST> and_exp;
  LOrExpAST(std::unique_ptr<BaseAST>& and_exp);
  LOrExpAST(const char* op, std::unique_ptr<BaseAST>& or_exp,
            std::unique_ptr<BaseAST>& and_exp);
  void* make_bool(koopa_raw_slice_t parent, std::vector<const void*>& inst_buf,
                  koopa_raw_value_t exp) const;
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};
class NumberAST : public BaseAST {
 public:
  int val;
  NumberAST(int val);
  void* to_koopa(koopa_raw_slice_t parent,
                 std::vector<const void*>& inst_buf) const override;
};

#endif  // AST_H