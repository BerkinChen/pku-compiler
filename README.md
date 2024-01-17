# 编译原理实践
陈滨琪 2000013185

***注：本实践主要参考[参考文档](https://pku-minic.github.io/online-doc/#/)，基本块维护器的引入参考了[开源实现](https://github.com/CaptainHarryChen/StupidSysY2RV)***

## 一、编译器概述
### （一）基本功能
1. 能将SysY程序转换为Koopa IR
2. 能将Koopa IR转换为RISC-V汇编程序
3. 能检查一些基本的错误，如变量未定义、函数未定义、break/continue不在循环中、数组初始化列表不合法等

### （二）主要特点
完成了基本要求的功能，能通过全部测试点；能生成内存形式的Koopa IR而不是文本形式的Koopa IR（把设计如何输出的问题转化成了如何构建一个CFG的问题）；速度慢（x

## 二、编译器设计
### （一）主要模块组成
主要有三大模块，词法分析&语法分析模块、IR生成模块、目标代码生成模块，分别将SysY转换为AST，将AST转换为Koopa IR，将Koopa IR转换为RISC-V汇编程序。以及一些辅助模块，如符号表、基本块维护器、循环维护器等

### （二）主要数据结构
#### 1. AST
AST的设计参考了[参考文档](https://pku-minic.github.io/online-doc/#/)：
```c++
class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual void *to_left_value() const { return nullptr; }
  virtual void *to_koopa() const { return nullptr; }
  virtual void *to_koopa(int index) const { return nullptr; }
  virtual void *to_koopa(std::vector<const void *> &global_var) const {
    return nullptr;
  }
  virtual void *to_koopa(koopa_raw_basic_block_t end_block) const {
    return nullptr;
  }
  virtual void *to_koopa(koopa_raw_type_t type) const { return nullptr; }
  virtual void *to_koopa(std::vector<const void *> &func,
                         std::vector<const void *> &value) const {
    return nullptr;
  }
  virtual void *to_koopa(std::vector<const void *> &global_var,
                         koopa_raw_type_t type) const {
    return nullptr;
  }
  virtual void *to_koopa(std::vector<const void *> &init_list, std::vector<size_t> size_vec, int level) const {
    return nullptr;
  }
  virtual int cal_value() const { assert(false); }
};
```
其中to_koopa函数用于将AST转换为Koopa IR，to_left_value用于将AST转换为左值（主要为了处理赋值），cal_value用于计算表达式的值（处理常数表达式）

#### 2.riscv_builder
主要包含若干visit函数，用于将Koopa IR转换为RISC-V汇编程序，以及若干辅助函数，用于计算指令大小、生成一些重复的指令、为变量在栈上分配空间并记录分配的位置等

#### 3.符号表
```c++
enum ValueType { Const, Var, Func, Array, Pointer};
struct Value {
  ValueType type;
  union SymbolListValue {
    int const_value;
    koopa_raw_value_t var_value;
    koopa_raw_function_t func_value;
    koopa_raw_value_t array_value;
    koopa_raw_value_t pointer_value;
  } data;
  Value() = default;
  Value(ValueType type, int value) : type(type) { data.const_value = value; }
  Value(ValueType type, koopa_raw_value_t value) : type(type) {
    if (type == Var)
      data.var_value = value;
    else if (type == Array)
      data.array_value = value;
    else if (type == Pointer)
      data.pointer_value = value;
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
```
符号表存储了Const、Var、Func、Array、Pointer五种类型的数据，整个符号表的结构设计为一个栈结构，每次进入新的作用域时和退出当前作用域时，分别调用newScope和delScope，用来添加一个新的符号表和删除当前的符号表。当查找符号时，从当前符号表开始向上查找，直到找到为止

#### 4.基本块维护器
```c++
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
```
基本块维护器主要用来管理当前函数中生成的基本块和当前基本块中生成的指令，每次生成一个基本块时调用newBlock，每次生成一条指令时调用addInst。当函数生成完毕后，调用delUnreachableBlock，删除不可达的基本块和其中的指令，以及通过checkBlock检查当前基本块中是否有返回语句，如果缺失则根据函数有类型添加一个返回语句

#### 5.循环维护器
```c++
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
```
循环维护器主要用来管理当前函数中的循环，每次进入一个循环时调用addWhile，每次退出一个循环时调用delWhile，通过getHead和getTail获取当前循环的入口块和出口块，用于指明break和continue的跳转位置

### （三）主要设计以及算法选择
#### 1.符号表的设计考虑
采用栈结构，每次进入一个新的作用域时将新的符号表压栈，退出当前作用域时将当前符号表弹栈，这样可以很方便的实现作用域的嵌套，同时每次查找符号时从当前符号表开始向上查找，直到找到为止

#### 2.寄存器分配策略
只在运算中途使用寄存器，把变量存在栈上

#### 3.优化策略
1. 对于语法分析中{...}的形式，定义为如下形式：
```yacc
ConstDecl
  : CONST Type ConstDefArray ';' {
    auto type = std::unique_ptr<BaseAST>($2);
    auto const_defs = std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>>($3);
    $$ = new ConstDeclAST(type, const_defs);
  }
  ;

ConstDefArray
  : ConstDef ',' ConstDefArray {
    auto vec = (std::vector<std::unique_ptr<BaseAST>>*)($3);
    auto const_def = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(const_def));
    $$ = vec;
  }
  | ConstDef {
    auto vec = new std::vector<std::unique_ptr<BaseAST>> ();
    auto const_def = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(const_def));
    $$ = vec;
  }
  ;
```
不用为Array再额外设计一个AST

2. 在每个函数结束时删除不可达的基本块

## 三、编译器实现
### （一）各阶段编码细节
***注：本小节的内容全部来自于完成过程中的思考和记录***
#### 1. Lv1
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

#### 2. Lv2

##### 2.1 初步设计
在lv1将AST转化为raw program的基础上，通过raw_visit函数递归访问raw program，visit返回一个字符串，通过字符串的拼接得到riscv程序。这种实现并不优雅，是否可以采用一种数据结构像raw program那样存放riscv的函数和指令，再给指令实现dump方法从而得到riscv程序

##### 2.2 实现
设计一种数据结构存放riscv函数和指令感觉像是实现了一种新的IR，必要性不强，所以还是采用了上述的方法，通过visit函数递归访问raw program，visit返回一个字符串，通过字符串的拼接得到riscv程序

#### 3. Lv3
##### 3.1 一元表达式
添加了一元表达式的AST，在处理一元表达式的AST转换为raw program时，对于+，不生成额外的IR，对于-，！等一元运算转换为对应的二元运算进行处理

生成RISCV的部分等逻辑和比较表达式完成后一起修改

##### 3.2 算数表达式
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

##### 3.3 逻辑表达式
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

##### 3.4 目标代码生成
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
#### 4. Lv4
##### 4.1 常量
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
##### 4.2 变量与赋值
设计了新的AST，修改符号表使其支持变量的存储，对于变量符表中存储的是koopa的alloc value，方便Koopa IR的生成。对于load和store指令，更新了riscv的生成，修改了对于这两条指令的处理，同时支持在寄存器不够时随机替换一个寄存器中的变量到栈上

#### 5. Lv5
对于语句块和作用域的控制，对符号表的结构进行了修改，将多重的符号表保存为一个栈结构，每次逐层向上查询，同时由于多层语句嵌套引起的寄存器分配相当麻烦，所以改成了将变量全分配到栈上的策略（x

#### 6. Lv6
##### 6.1 if/else处理
为了解决二义性的问题，将if exp stmt else stmt的形式拆分成了if exp stmt和else stms，由于分支语句的引入导致了基本块的增多，靠传参来确定基本块很难维护，创建了一个维护基本块的变量，可以通过这个变量来确定当前的基本块，同时也可以增加新的基本块。这个基本块的维护器还可以判断当前基本块中是否有分支、跳转、返回语句，在完成一个基本块时遍历其中的指令，删除不可达的指令并整合在基本块中

生成目标代码时，由于Koopa IR直接输出时可以解决名字重复的问题，所以修改为先从Koopa IR到string，再从string到Koopa IR，这样可以解决名字重复的问题

##### 6.2 短路求值
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

#### 7. Lv7
##### 7.1 while处理
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

##### 7.2 break/continue处理
设计一个栈结构管理当前的循环，记录当前循环的开头和结尾，break和continue的处理就是插入跳转到对应的位置的指令

#### 8. Lv8
##### 8.1 函数定义和调用
创建DefArray以处理多个函数的情况，对于函数参数的处理类似于其他的array，对于函数内部使用参数，分配一个临时变量以便于IR的生成。对于SysY的库函数，再处理其他函数定义前先处理，将其加入到全局符号表中，这样在处理其他函数时就可以直接调用库函数了

目标代码生成待全局变量完成后一起修改

##### 8.2 全局变量
对于全局变量，将其存放在全局符号表中，对于全局变量的使用，直接将其值替换到使用的地方。这里由于FuncType和BType都有INT导致语法产生了reduce/reduce冲突，所以将其合并为TYPE，这样虽然让定义一个void类型的变量的语句变得合法，但是在生成IR时会检查报错，所以不会影响结果

目标代码生成待全局变量完成后一起修改

##### 8.3 目标代码生成
主要修改全局变量的处理和函数的调用，对于全局变量，先将地址读到寄存器中，再根据地址读取值。对于函数调用，将参数存放到对应的寄存器中或者栈上，再调用函数，函数返回后将返回值存放到栈上

#### 9. Lv9
##### 9.1 一维数组
对于全局数组，采用global alloc和aggregate分配并初始化，对于局部数组，采用alloc分配空间，并通过getelementptr获取对应的地址，再store进行初始化。对于数组的访问，通过getelementptr获取对应的地址，再load获取值或者store赋值

##### 9.2 多维数组
针对多维数组，修改了InitValAST，定义了process函数，通过的递归的方式将多维数组填充为一维的vector，并且中途检查对齐和用0填充，最后再将vector根据数组的维数和大小填充为aggregate value，除了最后一层，每一层的值也都是aggregate value，对于多维数组的访问，根据访问的索引，每次通过getelementptr获取对应的地址，再load获取值或者store赋值

##### 9.3 数组参数
由于数组参数的特点，新增了一种指针类型，指针类型和数组类型在传参时较为复杂，例如*[i32, 10]的类型不能给**i32类型的参数赋值，所以需要用getelementptr获取对应的地址，同样对于指针参数，不能使用getelementptr获取对应的地址，所以需要使用getptr来获取对应的地址

##### 9.4 目标代码生成
主要处理了aggregate，getelementptr，getptr三种指令，并且对于store和load指令，如果是getelementptr或者getptr的结果，需要一些额外的处理

### （二）软件工具介绍
#### 1. libkoopa
用该库的定义的IR结构体，直接将AST转为内存形式的Koopa IR。使用该库生成Koopa代码，该库生成的代码能够对变量名和基本块名进行去重，于是通过该库的生成再读取得到一个名字没有重复的Koopa程序，生成RISC-V时就不会出现label重复的问题

### （三）测试情况说明
使用往年的数据进行测试，出现了以下问题：
1. 函数没有return语句：对于这种情况，修改了基本块维护器，在函数生成完毕后，检查当前基本块中是否有返回语句，如果缺失则根据函数有类型添加一个返回语句
2. 跳转指令超出了分支跳转的跳转范围：对于这种情况，修改了RISC-V生成器，对于分支语句，将其修改为跳转到一个新的基本块，这个基本块中只有一条跳转指令，跳转到真正的目标基本块
3. Koopa IR的自动去重只能处理同一个函数中的基本块重名问题，在不同函数中仍然会出现重名导致RISC-V生成出错：对于这种情况，在创建基本块时，将名字定义为函数名+基本块名，这样就不会出现重名的问题

## 四、实习总结
### （一）收获和体会
在文档一步一步的指导下完成了编译器的实现，对于编译器的各个阶段有了更深的理解。并且通过自己~~高超~~随机乱试的调试技巧更正了实现过程中很多细节的问题，最终通过了全部的测试点，很有成就感
### （二）实践过程中的难点和建议
如何生成内存形式的Koopa IR的文档严重缺失，导致一开始只能自己写一个文本形式的IR，将其读入为内存形式后一层层拆解，来观察Koopa IR的内存形式是怎样组织的，最终才能够实现内存形式的Koopa IR的生成。建议在文档中加入如何生成内存形式的Koopa IR的内容，尤其是关于不同的类型，例如value，func，basic block的不同变量道理有何含义，以及一些具体的例子，否则感觉选择生成内存形式的Koopa IR反而是一个不必要的负担。比如写到中途才发现如果只是为了生成IR的话used_by参数完全没有必要，以及一开始对于ty进行了很随意的处理，生成全都正确，但是到了数组的部分，各种类型的报错导致大面积返工，耗费了很多不必要的时间

### （三）对老师讲解内容与方式的建议
希望在课程中能更多的涉及和实践相关的内容，尤其是给出一些可以参考并使用的例子