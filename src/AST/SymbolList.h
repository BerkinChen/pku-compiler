#ifndef SYMBOL_LIST_H
#define SYMBOL_LIST_H
#include <map>
#include <string>
#include <variant>
#include <vector>
enum ValueType { Const, Var };
struct Value {
  ValueType type;
  int value;
  Value() = default;
  Value(ValueType type, int value): type(type), value(value) {};
};

class SymbolList {
private:
  std::map<std::string, Value> symbol_list;

public:
  ~SymbolList() = default;
  void addSymbol(std::string symbol, Value value);
  Value getSymbol(std::string symbol);
  void init();
};

#endif // SYMBOL_LIST_H