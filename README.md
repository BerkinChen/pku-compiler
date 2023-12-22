# pku-compiler

编译原理实践作业
[参考文档](https://pku-minic.github.io/online-doc/#/)

- [x] lv1
- [x] lv2
- [x] lv3
- [x] lv4
- [x] lv5
- [ ] lv6
- [ ] lv7
- [ ] lv8
- [ ] lv9

## Lv1
根据参考文档补全sysy.l,sysy.y
其中sysy.l中补全对于块注释的识别
```flex
BlockComment  "/*"([^*]|(\*+[^*/]))*\**"*/"
```
sysy.y中调整对于AST的构建，主要是把对于AST成员的赋值改为在构造函数中完成，并且修改了yyerror函数，使其能输出更加具体的错误信息
```yacc
%%
// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
CompUnit
  : FuncDef {
    auto func_def = unique_ptr<BaseAST>($1);
    ast = std::unique_ptr<BaseAST>(new CompUnitAST(func_def));
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto func_type = unique_ptr<BaseAST>($1);
    auto ident = unique_ptr<std::string>($2);
    auto block = unique_ptr<BaseAST>($5);
    $$ = new FuncDefAST(func_type, ident->c_str(), block);
  }
  ;
  // 略
%%
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  extern int yylineno;
  extern char *yytext;
  int len = strlen(yytext);
  int i;
  char buf[512] = {0};
  for (i=0; i<len; ++i)
    sprintf(buf, "%s%d ", buf, yytext[i]);
  fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
}
```
在定义AST时实现了to_string和to_koopa函数，完成了AST到字符串和AST到koopa raw program的转换
```c++
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual std::string to_string() const = 0;
  virtual void* to_koopa() const = 0;
};
```
实现了utils文件，包含了一些对于raw program的处理函数
```c++
koopa_raw_slice_t slice(koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t slice(std::vector<const void *> &vec,
                        koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);

koopa_raw_type_t type_kind(koopa_raw_type_tag_t tag);
```

## Lv2

### 初步设计
在lv1将AST转化为raw program的基础上，通过raw_visit函数递归访问raw program，visit返回一个字符串，通过字符串的拼接得到riscv程序。这种实现并不优雅，是否可以采用一种数据结构像raw program那样存放riscv的函数和指令，再给指令实现dump方法从而得到riscv程序

### 实现
设计一种数据结构存放riscv函数和指令感觉像是实现了一种新的IR，必要性不强，所以还是采用了上述的方法，通过visit函数递归访问raw program，visit返回一个字符串，通过字符串的拼接得到riscv程序

## Lv3
### 一元表达式
添加了一元表达式的AST，在处理一元表达式的AST转换为raw program时，对于+，不生成额外的IR，对于-，！等一元运算转换为对应的二元运算进行处理

生成RISCV的部分等逻辑和比较表达式完成后一起修改

### 算数表达式
根据语法添加对应的AST，修改生成IR的函数，由于要用到used_by，所以需要对函数添加parent参数，又由于现在每一个基本块生成的指令不止一条，所以需要增加一个存放指令的vector，随着函数传递下去。对BaseAST做出以下修改
```c++
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void* to_koopa() const { return nullptr; }
  virtual void* to_koopa(std::vector<const void*>& inst_buf) const {
    return nullptr;
  }
  virtual void* to_koopa(koopa_raw_slice_t parent) const { return nullptr; }
  virtual void* to_koopa(koopa_raw_slice_t parent,
                         std::vector<const void*>& inst_buf) const {
    return nullptr;
  }
};
```
其余的派生类根据不同的需要实现对应的多态函数

生成RISCV的部分等逻辑和比较表达式完成后一起修改

### 逻辑表达式
AST和算数表达式类似，修改sysy.l的文件，添加对于逻辑运算符的识别
```flex
RelOP         [<>]=?
EqOP          [!=]=
"&&"            { yylval.str_val = new string(yytext); return AND; }
"||"            { yylval.str_val = new string(yytext); return OR; }

{RelOP}         { yylval.str_val = new string(yytext); return RELOP; }
{EqOP}          { yylval.str_val = new string(yytext); return EQOP; }
```
除此之外，由于Koopa IR不支持逻辑运算，LAnd和LOr在运算时会对两边的表达式进行ne 0的操作，将其转换为逻辑运算

### 目标代码生成
考虑到后续寄存器分配的问题，设计了一个RISCV_Builder类，并于其中维护一个env类，保存和更新寄存器的使用情况
```c++
class RISCV_Builder {
  class Env {
    enum Register_State { UNUSED, USED };
    std::map<koopa_raw_value_t, std::string>
        register_alloc_map;
    std::map<std::string, Register_State> register_state_map;
   public:
    void state_init();
    void state_free(std::string reg);
    std::string register_check(koopa_raw_value_t value);
    std::string register_alloc(koopa_raw_value_t value);
  };
  Env env;
  std::string load_register(koopa_raw_value_t value, std::string reg);
  std::string raw_visit(const koopa_raw_program_t &raw);
  std::string raw_visit(const koopa_raw_slice_t &slice);
  std::string raw_visit(const koopa_raw_function_t &func);
  std::string raw_visit(const koopa_raw_basic_block_t &bb);
  std::string raw_visit(const koopa_raw_value_t &value);
  std::string raw_visit(const koopa_raw_return_t &return_value);
  //std::string raw_visit(const koopa_raw_integer_t &integer_value);
  std::string raw_visit(const koopa_raw_binary_t &binary_value);
 public:
  RISCV_Builder() = default;
  void build(koopa_raw_program_t raw, const char *path);
};
```
## Lv4
### 常量
根据语法设计对应的ast，对const关键字做对应处理，对于const变量在处理时直接计算出其值，对于const变量的使用，直接将其值替换到使用的地方。为此给表达式相关的ast添加了一个新函数用于计算表达式的值。设计了一个符号表存储有关常量的信息。
```c++
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
```
### 变量与赋值
设计了新的AST，修改符号表使其支持变量的存储，对于变量符表中存储的是koopa的alloc value，方便Koopa IR的生成。对于load和store指令，更新了riscv的生成，修改了对于这两条指令的处理，同时支持在寄存器不够时随机替换一个寄存器中的变量到栈上

## Lv5
对于语句块和作用域的控制，对符号表的结构进行了修改，将多重的符号表保存为一个栈结构，每次逐层向上查询，同时由于多层语句嵌套引起的寄存器分配相当麻烦，所以改成了将变量全分配到栈上的策略（x

## Lv6
### if/else处理
为了解决二义性的问题，将if exp stmt else stmt的形式拆分成了if exp stmt和else stms，由于分支语句的引入导致了基本块的增多，靠传参来确定基本块很难维护，创建了一个维护基本块的变量，可以通过这个变量来确定当前的基本块，同时也可以增加新的基本块。这个基本块的维护器还可以判断当前基本块中是否有分支、跳转、返回语句，在完成一个基本块时遍历其中的指令，删除不可达的指令并整合在基本块中