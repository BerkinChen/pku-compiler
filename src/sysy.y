%code requires {
  #include <memory>
  #include <string>
  #include "AST/AST.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <cassert>
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
  std::vector<std::unique_ptr<BaseAST>> *ast_vec;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT EQOP RELOP AND OR
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> CompUnit FuncDef Block BlockItem Stmt Decl Type If Def
%type <ast_val> Exp PrimaryExp UnaryExp AddExp MulExp RelExp EqExp LAndExp LOrExp Number LVal
%type <ast_val> ConstDecl ConstDef ConstInitVal ConstExp
%type <ast_val> VarDecl VarDef InitVal FuncFParam FuncRParam
%type <ast_vec> BlockArray ConstDefArray VarDefArray DefArray
%type <ast_vec> FuncFParamArray FuncRParamArray
%type <str_val> UNARYOP MULOP ADDOP

%%

CompUnit
  : DefArray {
    auto defs = std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>>($1);
    ast = std::unique_ptr<BaseAST>(new CompUnitAST(defs));
  }
  ;

DefArray
  : Def DefArray {
    auto vec = (std::vector<std::unique_ptr<BaseAST>>*)($2);
    auto def = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(def));
    $$ = vec;
  }
  | Def {
    //std::cout << "FuncDef" << std::endl;
    auto vec = new std::vector<std::unique_ptr<BaseAST>>();
    auto def = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(def));
    $$ = vec;
  }
  ;

Def 
  : FuncDef {
    auto funcdef = std::unique_ptr<BaseAST>($1);
    $$ = new DefAST(funcdef, DefAST::DefType::FuncDef);
  } | ConstDecl {
    auto globaconstdef = std::unique_ptr<BaseAST>($1);
    $$ = new DefAST(globaconstdef, DefAST::DefType::ConstDef);
  } | VarDecl {
    auto globavardef = std::unique_ptr<BaseAST>($1);
    $$ = new DefAST(globavardef, DefAST::DefType::VarDef);
  };

FuncDef
  : Type IDENT '(' FuncFParamArray ')' Block {
    //std::cout << "FuncDef" << std::endl;
    auto func_type = std::unique_ptr<BaseAST>($1);
    auto ident = std::unique_ptr<std::string>($2);
    auto func_fparam_array = std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>>($4);
    auto block = std::unique_ptr<BaseAST>($6);
    $$ = new FuncDefAST(func_type, ident->c_str(), func_fparam_array, block);
  }
  ;

Type
  : INT {
    $$ = new TypeAST("int");
  }
  | VOID {
    $$ = new TypeAST("void");
  }
  ;

FuncFParamArray
  : FuncFParam ',' FuncFParamArray {
    auto vec = (std::vector<std::unique_ptr<BaseAST>>*)($3);
    auto func_fparam = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(func_fparam));
    $$ = vec;
  }
  | FuncFParam {
    auto vec = new std::vector<std::unique_ptr<BaseAST>> ();
    auto func_fparam = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(func_fparam));
    $$ = vec;
  }
  | {
    $$ = new std::vector<std::unique_ptr<BaseAST>> ();
  }
  ;

FuncFParam
  : Type IDENT {
    auto type = std::unique_ptr<BaseAST>($1);
    auto ident = std::unique_ptr<std::string>($2);
    $$ = new FuncFParamAST(type, ident->c_str());
  }
  ;

Block
  : '{' BlockArray '}' {
    auto BlockArray = std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>>($2);
    $$ = new BlockAST(BlockArray);
  }
  | '{' '}' {
    $$ = new BlockAST();
  }
  ;

BlockArray
  : BlockItem BlockArray{
    auto vec = (std::vector<std::unique_ptr<BaseAST>>*)($2);
    auto blockitem = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(blockitem));
    $$ = vec;
  }
  | BlockItem {
    auto vec = new std::vector<std::unique_ptr<BaseAST>> ();
    auto blockitem = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(blockitem));
    $$ = vec;
  }
  ;

BlockItem : Stmt | Decl;

Stmt
  : RETURN Exp ';' {
    auto exp = std::unique_ptr<BaseAST>($2);
    $$ = new StmtAST(exp, StmtAST::StmtType::Return);
  }
  | LVal '=' Exp ';' {
    auto lval = std::unique_ptr<BaseAST>($1);
    auto exp = std::unique_ptr<BaseAST>($3);
    $$ = new StmtAST(lval, exp, StmtAST::StmtType::Assign);
  }
  | Block {
    auto block = std::unique_ptr<BaseAST>($1);
    $$ = new StmtAST(block, StmtAST::StmtType::Block);
  }
  | Exp ';' {
    auto exp = std::unique_ptr<BaseAST>($1);
    $$ = new StmtAST(exp, StmtAST::StmtType::Exp);
  }
  | ';' {
    $$ = new StmtAST(StmtAST::StmtType::Empty);
  }
  | RETURN ';' {
    $$ = new StmtAST(StmtAST::StmtType::Return);
  }
  | If {
    auto exp = std::unique_ptr<BaseAST>($1);
    $$ = new StmtAST(exp, StmtAST::StmtType::If);
  }
  | If ELSE Stmt {
    auto exp = std::unique_ptr<BaseAST>($1);
    auto stmt = std::unique_ptr<BaseAST>($3);
    $$ = new StmtAST(stmt, exp, StmtAST::StmtType::If);
  }
  | WHILE '(' Exp ')' Stmt {
    auto exp = std::unique_ptr<BaseAST>($3);
    auto stmt = std::unique_ptr<BaseAST>($5);
    $$ = new StmtAST(stmt, exp, StmtAST::StmtType::While);
  }
  | BREAK ';' {
    $$ = new StmtAST(StmtAST::StmtType::Break);
  }
  | CONTINUE ';' {
    $$ = new StmtAST(StmtAST::StmtType::Continue);
  }
  ;

If
 : IF '(' Exp ')' Stmt {
    auto exp = std::unique_ptr<BaseAST>($3);
    auto stmt = std::unique_ptr<BaseAST>($5);
    $$ = new IfAST(exp, stmt);
  }
  ;

Decl : ConstDecl | VarDecl;

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

ConstDef 
  : IDENT '=' ConstInitVal {
    auto ident = std::unique_ptr<std::string>($1);
    auto const_init_val = std::unique_ptr<BaseAST>($3);
    $$ = new ConstDefAST(ident->c_str(), const_init_val);
  }
  ;

ConstInitVal : ConstExp;

ConstExp : Exp;

VarDecl
  : Type VarDefArray ';' {
    auto type = std::unique_ptr<BaseAST>($1);
    auto var_defs = std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>>($2);
    $$ = new VarDeclAST(type, var_defs);
  }
  ;

VarDefArray
  : VarDef ',' VarDefArray {
    auto vec = (std::vector<std::unique_ptr<BaseAST>>*)($3);
    auto var_def = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(var_def));
    $$ = vec;
  }
  | VarDef {
    auto vec = new std::vector<std::unique_ptr<BaseAST>> ();
    auto var_def = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(var_def));
    $$ = vec;
  }
  ;

VarDef
  : IDENT '=' InitVal {
    auto ident = std::unique_ptr<std::string>($1);
    auto init_val = std::unique_ptr<BaseAST>($3);
    $$ = new VarDefAST(ident->c_str(), init_val);
  }
  | IDENT {
    auto ident = std::unique_ptr<std::string>($1);
    $$ = new VarDefAST(ident->c_str());
  }
  ;

InitVal : Exp;

LVal
  : IDENT {
    auto ident = std::unique_ptr<std::string>($1);
    $$ = new LValAST(ident->c_str());
  }
  ;

Exp
  : LOrExp {
    auto lorexp = std::unique_ptr<BaseAST>($1);
    $$ = new ExpAST(lorexp);
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
  | LVal {
    auto lval = std::unique_ptr<BaseAST>($1);
    $$ = new PrimaryExpAST(lval);
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
  | IDENT '(' FuncRParamArray ')' {
    auto ident = std::unique_ptr<std::string>($1);
    auto func_rparam_array = std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>>($3);
    $$ = new UnaryExpAST(ident->c_str(), func_rparam_array);
  }
  ;

FuncRParamArray
  : FuncRParam ',' FuncRParamArray {
    auto vec = (std::vector<std::unique_ptr<BaseAST>>*)($3);
    auto func_rparam = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(func_rparam));
    $$ = vec;
  }
  | FuncRParam {
    auto vec = new std::vector<std::unique_ptr<BaseAST>> ();
    auto func_rparam = std::unique_ptr<BaseAST>($1);
    vec->push_back(std::move(func_rparam));
    $$ = vec;
  }
  | {
    $$ = new std::vector<std::unique_ptr<BaseAST>> ();
  }
  ;

FuncRParam : Exp;

UNARYOP :
  '-' {
    $$ = new std::string("-");
  }| '!' {
    $$ = new std::string("!");
  }| '+' {
    $$ = new std::string("+");
  }
  ;

MulExp
  : UnaryExp {
    auto unaryexp = std::unique_ptr<BaseAST>($1);
    $$ = new MulExpAST(unaryexp);
  }
  | MulExp MULOP UnaryExp {
    auto mulexp = std::unique_ptr<BaseAST>($1);
    auto mulop = std::unique_ptr<std::string>($2);
    auto unaryexp = std::unique_ptr<BaseAST>($3);
    $$ = new MulExpAST(mulop->c_str(), mulexp, unaryexp);
  }
  ;

MULOP : 
  '*' {
    $$ = new std::string("*");
  }| '/' {
    $$ = new std::string("/");
  }| '%' {
    $$ = new std::string("%");
  }
  ;

AddExp
  : MulExp {
    auto mulexp = std::unique_ptr<BaseAST>($1);
    $$ = new AddExpAST(mulexp);
  }
  | AddExp ADDOP MulExp {
    auto addexp = std::unique_ptr<BaseAST>($1);
    auto addop = std::unique_ptr<std::string>($2);
    auto mulexp = std::unique_ptr<BaseAST>($3);
    $$ = new AddExpAST(addop->c_str(), addexp, mulexp);
  }
  ;

ADDOP :
  '+' {
    $$ = new std::string("+");
  }| '-' {
    $$ = new std::string("-");
  }
  ;

RelExp :
  AddExp {
    auto addexp = std::unique_ptr<BaseAST>($1);
    $$ = new RelExpAST(addexp);
  }
  | RelExp RELOP AddExp {
    auto relexp = std::unique_ptr<BaseAST>($1);
    auto relop = std::unique_ptr<std::string>($2);
    auto addexp = std::unique_ptr<BaseAST>($3);
    $$ = new RelExpAST(relop->c_str(), relexp, addexp);
  }
  ;

EqExp :
  RelExp {
    auto relexp = std::unique_ptr<BaseAST>($1);
    $$ = new EqExpAST(relexp);
  }
  | EqExp EQOP RelExp {
    auto eqexp = std::unique_ptr<BaseAST>($1);
    auto eqop = std::unique_ptr<std::string>($2);
    auto relexp = std::unique_ptr<BaseAST>($3);
    $$ = new EqExpAST(eqop->c_str(), eqexp, relexp);
  }
  ;

LAndExp :
  EqExp {
    auto eqexp = std::unique_ptr<BaseAST>($1);
    $$ = new LAndExpAST(eqexp);
  }
  | LAndExp AND EqExp {
    auto landexp = std::unique_ptr<BaseAST>($1);
    auto andop = std::unique_ptr<std::string>($2);
    auto eqexp = std::unique_ptr<BaseAST>($3);
    $$ = new LAndExpAST(andop->c_str(), landexp, eqexp);
  }
  ;

LOrExp :
  LAndExp {
    auto landexp = std::unique_ptr<BaseAST>($1);
    $$ = new LOrExpAST(landexp);
  }
  | LOrExp OR LAndExp {
    auto lorexp = std::unique_ptr<BaseAST>($1);
    auto orop = std::unique_ptr<std::string>($2);
    auto landexp = std::unique_ptr<BaseAST>($3);
    $$ = new LOrExpAST(orop->c_str(), lorexp, landexp);
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
