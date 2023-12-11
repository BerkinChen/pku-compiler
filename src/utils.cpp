#include "utils.h"
#include <assert.h>
#include <fstream>
#include <cstring>

koopa_raw_slice_t slice(koopa_raw_slice_item_kind_t kind) {
  koopa_raw_slice_t ret;
  ret.kind = kind;
  ret.buffer = nullptr;
  ret.len = 0;
  return ret;
}
koopa_raw_slice_t slice(std::vector<const void *> &vec,
                        koopa_raw_slice_item_kind_t kind) {
  koopa_raw_slice_t ret;
  ret.kind = kind;
  ret.buffer = new const void *[vec.size()];
  std::copy(vec.begin(), vec.end(), ret.buffer);
  ret.len = vec.size();
  return ret;
}

koopa_raw_slice_t slice(const void *data, koopa_raw_slice_item_kind_t kind) {
  koopa_raw_slice_t ret;
  ret.kind = kind;
  ret.buffer = new const void *[1];
  ret.buffer[0] = data;
  ret.len = 1;
  return ret;
}

koopa_raw_type_t type_kind(koopa_raw_type_tag_t tag) {
  koopa_raw_type_kind_t *ret = new koopa_raw_type_kind_t();
  ret->tag = tag;
  return (koopa_raw_type_t)ret;
}