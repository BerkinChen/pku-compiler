#ifndef UTILS_H
#define UTILS_H

#include "koopa.h"
#include <map>
#include <string>
#include <vector>

enum ValueType { Const, Var, Func, Array };
struct Value {
  ValueType type;
  union SymbolListValue {
    int const_value;
    koopa_raw_value_t var_value;
    koopa_raw_function_t func_value;
    koopa_raw_value_t array_value;
  } data;
  Value() = default;
  Value(ValueType type, int value) : type(type) { data.const_value = value; }
  Value(ValueType type, koopa_raw_value_t value) : type(type) {
    if (type == Var)
      data.var_value = value;
    else if (type == Array)
      data.array_value = value;
  }
  Value(ValueType type, koopa_raw_function_t value) : type(type) {
    data.func_value = value;
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

class BlockManager {
private:
  std::vector<const void *> *block_list_vector;
  std::vector<const void *> tmp_inst_list;

public:
  void init(std::vector<const void *> *block_list_vector);
  void newBlock(koopa_raw_basic_block_data_t *basic_block);
  void delBlock();
  void addInst(const void *inst);
  void delUnreachableBlock();
  bool checkBlock();
};

class LoopManager {
private:
  struct While {
    koopa_raw_basic_block_t head;
    koopa_raw_basic_block_t tail;
    While(koopa_raw_basic_block_t head, koopa_raw_basic_block_t tail)
        : head(head), tail(tail) {}
  };
  std::vector<While> while_list;

public:
  void addWhile(koopa_raw_basic_block_t head, koopa_raw_basic_block_t tail);
  void delWhile();
  koopa_raw_basic_block_t getHead();
  koopa_raw_basic_block_t getTail();
};

koopa_raw_slice_t slice(koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t slice(std::vector<const void *> &vec,
                        koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t slice(const void *data,
                        koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);

koopa_raw_type_kind *type_kind(koopa_raw_type_tag_t tag);

koopa_raw_type_kind *pointer_type_kind(koopa_raw_type_tag_t tag);

koopa_raw_type_kind *array_type_kind(koopa_raw_type_tag_t tag, std::vector<size_t> size_vec);

koopa_raw_value_data *jump_value(koopa_raw_basic_block_t tar);

koopa_raw_value_data *ret_value(koopa_raw_type_tag_t tag);

koopa_raw_value_data *zero_init(koopa_raw_type_kind *type);

#endif // UTILS_H