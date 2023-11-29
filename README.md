# pku-compiler

编译原理实践作业
[参考文档](https://pku-minic.github.io/online-doc/#/)

- [x] lv1
- [ ] lv2
- [ ] lv3
- [ ] lv4
- [ ] lv5
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