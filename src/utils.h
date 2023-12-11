#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include "koopa.h"

koopa_raw_slice_t slice(koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t slice(std::vector<const void *> &vec,
                        koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t slice(const void *data,
                        koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);

koopa_raw_type_t type_kind(koopa_raw_type_tag_t tag);

#endif // UTILS_H