#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

#include "utils.h"

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void* to_koopa() const = 0;
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
  void* to_koopa() const override;
};

class ExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> unary_exp;
  ExpAST(std::unique_ptr<BaseAST>& unary_exp);
  void* to_koopa() const override;
};

class PrimaryExpAST : public BaseAST {
  public:
  std::unique_ptr<BaseAST> exp;
  PrimaryExpAST(std::unique_ptr<BaseAST>& exp);
  void* to_koopa() const override;
};

class UnaryExpAST : public BaseAST {
 public:
 enum {
    Exp,
    Op
  }type;
  std::string op;
  std::unique_ptr<BaseAST> exp;
  UnaryExpAST(std::unique_ptr<BaseAST>& exp);
  UnaryExpAST(const char* op, std::unique_ptr<BaseAST>& exp);
  void* to_koopa() const override;
};

class NumberAST : public BaseAST {
 public:
  int val;
  NumberAST(int val);
  void* to_koopa() const override;
};

#endif  // AST_H