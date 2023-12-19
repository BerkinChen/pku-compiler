#include "SymbolList.h"

void SymbolList::addSymbol(std::string symbol, Value value) {
    symbol_list_vector.back()[symbol] = value;
}

Value SymbolList::getSymbol(std::string symbol) {
    for (auto it = symbol_list_vector.rbegin(); it != symbol_list_vector.rend(); ++it) {
        if (it->find(symbol) != it->end()) {
            return it->at(symbol);
        }
    }
    return Value();
}

void SymbolList::newScope() {
    symbol_list_vector.push_back(std::map<std::string, Value>());
}

void SymbolList::delScope() {
    symbol_list_vector.pop_back();
}
