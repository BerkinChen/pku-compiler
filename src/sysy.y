%code requires {
  #include <memory>
  #include <string>
  #include "AST/AST.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include "AST/AST.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个AST
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 std::unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT UNARYOP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp Number

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
CompUnit
  : FuncDef {
    auto func_def = std::unique_ptr<BaseAST>($1);
    ast = std::unique_ptr<BaseAST>(new CompUnitAST(func_def));
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto func_type = std::unique_ptr<BaseAST>($1);
    auto ident = std::unique_ptr<std::string>($2);
    auto block = std::unique_ptr<BaseAST>($5);
    $$ = new FuncDefAST(func_type, ident->c_str(), block);
  }
  ;

FuncType
  : INT {
    $$ = new FuncTypeAST("int");
  }
  ;

Block
  : '{' Stmt '}' {
    auto stmt = std::unique_ptr<BaseAST>($2);
    $$ = new BlockAST(stmt);
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto exp = std::unique_ptr<BaseAST>($2);
    $$ = new StmtAST(exp);
  }
  ;


Exp
  : UnaryExp {
    auto unaryexp = std::unique_ptr<BaseAST>($1);
    $$ = new ExpAST(unaryexp);
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto exp = std::unique_ptr<BaseAST>($2);
    $$ = new PrimaryExpAST(exp);
  } 
  | Number {
    auto number = std::unique_ptr<BaseAST>($1);
    $$ = new PrimaryExpAST(number);
  }
  ;

UnaryExp
  : PrimaryExp {
    auto primaryexp = std::unique_ptr<BaseAST>($1);
    $$ = new UnaryExpAST(primaryexp);
  }
  | UNARYOP UnaryExp {
    auto unaryop = std::unique_ptr<std::string>($1);
    auto unaryexp = std::unique_ptr<BaseAST>($2);
    $$ = new UnaryExpAST(unaryop->c_str(), unaryexp);
  }
  ;

Number
  : INT_CONST {
    $$ = new NumberAST($1);
  }
  ;


%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  extern int yylineno;
  extern char *yytext;
  int len = strlen(yytext);
  int i;
  char buf[512] = {0};
  for (i=0; i<len; ++i)
    sprintf(buf, "%s%d ", buf, yytext[i]);
  fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
}
