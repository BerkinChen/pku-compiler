# pku-compiler

编译原理实践作业
[参考文档](https://pku-minic.github.io/online-doc/#/)

- [x] lv1
- [x] lv2
- [x] lv3
- [x] lv4
- [x] lv5
- [x] lv6
- [x] lv7
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

生成目标代码时，由于Koopa IR直接输出时可以解决名字重复的问题，所以修改为先从Koopa IR到string，再从string到Koopa IR，这样可以解决名字重复的问题

### 短路求值
将LOr和LAnd改为短路求值，改为如下的形式
```c++
// LOr
int result = 1;
if (lhs == 1) {
  // do nothing
} else {
  result = rhs != 0;
}
// LAnd
int result = 0;
if (lhs == 1) {
  result = rhs != 0;
} else {
  // do nothing
}
```

## Lv7
### while处理
while定义三个基本块，分别是入口块（条件块），循环主体（循环块）和结束块，从当前block默认跳转到入口块，入口块根据条件跳转到循环块或者结束块，循环块执行完毕后跳转到入口块，结束块也就是后续的block
```c++
koopa_raw_basic_block_data_t *cond_block =
        new koopa_raw_basic_block_data_t();
    cond_block->name = "%while_entry";
    cond_block->params = slice(KOOPA_RSIK_VALUE);
    cond_block->used_by = slice(KOOPA_RSIK_VALUE);
    block_manager.addInst(jump_value(cond_block));
    block_manager.newBlock(cond_block);
    ret->kind.tag = KOOPA_RVT_BRANCH;
    ret->kind.data.branch.cond = (koopa_raw_value_t)exp->to_koopa();
    koopa_raw_basic_block_data_t *true_block =
        new koopa_raw_basic_block_data_t();
    true_block->name = "%while_body";
    true_block->params = slice(KOOPA_RSIK_VALUE);
    true_block->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.data.branch.true_bb = (koopa_raw_basic_block_t)true_block;
    ret->kind.data.branch.true_args = slice(KOOPA_RSIK_VALUE);
    koopa_raw_basic_block_data_t *end_block =
        new koopa_raw_basic_block_data_t();
    end_block->name = "%end";
    end_block->params = slice(KOOPA_RSIK_VALUE);
    end_block->used_by = slice(KOOPA_RSIK_VALUE);
    ret->kind.data.branch.false_bb = (koopa_raw_basic_block_t)end_block;
    ret->kind.data.branch.false_args = slice(KOOPA_RSIK_VALUE);
    loop_manager.addWhile(cond_block, end_block);
    block_manager.addInst(ret);
    block_manager.newBlock(true_block);
    stmt->to_koopa();
    block_manager.addInst(jump_value(cond_block));
    block_manager.newBlock(end_block);
    loop_manager.delWhile();
```

### break/continue处理
设计一个栈结构管理当前的循环，记录当前循环的开头和结尾，break和continue的处理就是插入跳转到对应的位置的指令

## Lv8
### 函数定义和调用
创建DefArray以处理多个函数的情况，对于函数参数的处理类似于其他的array，对于函数内部使用参数，分配一个临时变量以便于IR的生成。对于SysY的库函数，再处理其他函数定义前先处理，将其加入到全局符号表中，这样在处理其他函数时就可以直接调用库函数了

目标代码生成待全局变量完成后一起修改

### 全局变量
对于全局变量，将其存放在全局符号表中，对于全局变量的使用，直接将其值替换到使用的地方。这里由于FuncType和BType都有INT导致语法产生了reduce/reduce冲突，所以将其合并为TYPE，这样虽然让定义一个void类型的变量的语句变得合法，但是在生成IR时会检查报错，所以不会影响结果

目标代码生成待全局变量完成后一起修改

### 目标代码生成
主要修改全局变量的处理和函数的调用，对于全局变量，先将地址读到寄存器中，再根据地址读取值。对于函数调用，将参数存放到对应的寄存器中或者栈上，再调用函数，函数返回后将返回值存放到栈上

## Lv9
### 一维数组
对于全局数组，采用global alloc和aggregate分配并初始化，对于局部数组，采用alloc分配空间，并通过getelementptr获取对应的地址，再store进行初始化。对于数组的访问，通过getelementptr获取对应的地址，再load获取值或者store赋值

### 多维数组
针对多维数组，修改了InitValAST，定义了process函数，通过的递归的方式将多维数组填充为一维的vector，并且中途检查对齐和用0填充，最后再将vector根据数组的维数和大小填充为aggregate value，除了最后一层，每一层的值也都是aggregate value，对于多维数组的访问，根据访问的索引，每次通过getelementptr获取对应的地址，再load获取值或者store赋值

### 数组参数
由于数组参数的特点，新增了一种指针类型，指针类型和数组类型在传参时较为复杂，例如*[i32, 10]的类型不能给**i32类型的参数赋值，所以需要用getelementptr获取对应的地址，同样对于指针参数，不能使用getelementptr获取对应的地址，所以需要使用getptr来获取对应的地址

### 目标代码生成