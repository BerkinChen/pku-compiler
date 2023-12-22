#ifndef UTILS_H
#define UTILS_H

#include "koopa.h"
#include <map>
#include <string>
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

class BlockManager {
private:
  std::vector<const void *> *block_list_vector;
  std::vector<const void *> tmp_inst_list;

public:
  void init(std::vector<const void *> *block_list_vector);
  void newBlock(koopa_raw_basic_block_data_t *basic_block);
  void delBlock();
  void addInst(const void *inst);
  bool checkBlock();
};

koopa_raw_slice_t slice(koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t slice(std::vector<const void *> &vec,
                        koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t slice(const void *data,
                        koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);

koopa_raw_type_t type_kind(koopa_raw_type_tag_t tag);

koopa_raw_type_t pointer_type_kind(koopa_raw_type_tag_t tag);

koopa_raw_value_data *jump_value(koopa_raw_basic_block_t tar);


#endif // UTILS_H