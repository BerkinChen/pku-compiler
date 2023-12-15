#include "SymbolList.h"

void SymbolList::addSymbol(std::string symbol, Value value) {
    symbol_list[symbol] = value;
}

Value SymbolList::getSymbol(std::string symbol) {
    return symbol_list[symbol];
}

void SymbolList::init() {
    symbol_list.clear();
}
