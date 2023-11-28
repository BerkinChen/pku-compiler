#pragma once

#include <memory>
#include <string>
#include <vector>

#include "utils.h"

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual std::string to_string() const = 0;
  virtual void* to_koopa() const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

  CompUnitAST(std::unique_ptr<BaseAST>& func_def);

  std::string to_string() const override;
  void* to_koopa() const override;
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  FuncDefAST(std::unique_ptr<BaseAST>& func_type, const char* ident,
             std::unique_ptr<BaseAST>& block);

  std::string to_string() const override;
  void* to_koopa() const override;
};

class FuncTypeAST : public BaseAST {
 public:
  std::string type;
  FuncTypeAST(const char* type);
  std::string to_string() const override;
  void* to_koopa() const override;
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;

  BlockAST(std::unique_ptr<BaseAST>& stmt);

  std::string to_string() const override;
  void* to_koopa() const override;
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> ret_num;

  StmtAST(std::unique_ptr<BaseAST>& ret_num);

  std::string to_string() const override;
  void* to_koopa() const override;
};

class NumberAST : public BaseAST {
 public:
  int val;

  NumberAST(int val);

  std::string to_string() const override;
  void* to_koopa() const override;
};
