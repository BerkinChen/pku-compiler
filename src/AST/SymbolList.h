#ifndef SYMBOL_LIST_H
#define SYMBOL_LIST_H
#include "koopa.h"
#include <map>
#include <string>
#include <variant>
#include <vector>
enum ValueType { Const, Var };
struct Value {
  ValueType type;
  union SymbolListValue {
    int const_value;
    koopa_raw_value_t var_value;
  } data;
  Value() = default;
  Value(ValueType type, int value) : type(type) { data.const_value = value; }
  Value(ValueType type, koopa_raw_value_t value) : type(type) {
    data.var_value = value;
  }
};

class SymbolList {
private:
  std::vector<std::map<std::string, Value>> symbol_list_vector;

public:
  ~SymbolList() = default;
  void addSymbol(std::string symbol, Value value);
  Value getSymbol(std::string symbol);
  void newScope();
  void delScope();
};

#endif // SYMBOL_LIST_H