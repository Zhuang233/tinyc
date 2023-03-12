#include "list.h"

#ifndef Tinyc_SYNTAX_HEADER
#define Tinyc_SYNTAX_HEADER


//节点类型
typedef enum {
    IMMEDIATE,
    VARIABLE,
    UNARY_OPERATOR,
    BINARY_OPERATOR,
    BLOCK,
    IF_STATEMENT,
    RETURN_STATEMENT,
    DEFINE_VAR,
    FUNCTION,
    FUNCTION_CALL,
    FUNCTION_ARGUMENTS,
    ASSIGNMENT,
    WHILE_SYNTAX,
    TOP_LEVEL
} SyntaxType;


//一元运算表达式类型
typedef enum { 
    BITWISE_NEGATION, 
    LOGICAL_NEGATION 
} UnaryExpressionType;


//二元运算表达式类型
typedef enum {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
} BinaryExpressionType;

struct Syntax;
typedef struct Syntax Syntax;

//立即数
typedef struct Immediate { int value; } Immediate;

//变量
typedef struct Variable {
    char *var_name;
} Variable;

//一元表达式
typedef struct UnaryExpression {
    UnaryExpressionType unary_type;
    Syntax *expression;
} UnaryExpression;

//二元表达式
typedef struct BinaryExpression {
    BinaryExpressionType binary_type;
    Syntax *left;
    Syntax *right;
} BinaryExpression;

//参数列表
typedef struct FunctionArguments { List *arguments; } FunctionArguments;

//函数调用
typedef struct FunctionCall {
    char *function_name;
    Syntax *function_arguments;
} FunctionCall;

//赋值语句
typedef struct Assignment {
    char *var_name;
    Syntax *expression;
} Assignment;

//if语句
typedef struct IfStatement {
    Syntax *condition;
    Syntax *then;
} IfStatement;

//变量声明
typedef struct DefineVarStatement {
    char *var_name;
    Syntax *init_value;
} DefineVarStatement;

//while语句
typedef struct WhileStatement {
    Syntax *condition;
    Syntax *body;
} WhileStatement;

//函数返回
typedef struct ReturnStatement { Syntax *expression; } ReturnStatement;

//语句块
typedef struct Block { List *statements; } Block;

//函数
typedef struct Function {
    char *name;
    List *parameters;
    Syntax *root_block;
} Function;

//形参
typedef struct Parameter {
    char *name;
} Parameter;

//根
typedef struct TopLevel { List *declarations; } TopLevel;

// 语法树节点结构体
struct Syntax {
    SyntaxType type; //类型
    union {
        Immediate *immediate; //立即数

        Variable *variable; //变量

        UnaryExpression *unary_expression;//一元运算符表达式

        BinaryExpression *binary_expression;//二元运算符表达式

        Assignment *assignment;//赋值

        ReturnStatement *return_statement;//函数返回

        FunctionArguments *function_arguments;//函数参数

        FunctionCall *function_call;//函数调用

        IfStatement *if_statement;//条件语句

        DefineVarStatement *define_var_statement;//变量声明

        WhileStatement *while_statement;//循环语句

        Block *block;//语句块

        Function *function;//函数

        TopLevel *top_level;//根
    };
};

Syntax *immediate_new(int value);

Syntax *variable_new(char *var_name);

Syntax *bitwise_negation_new(Syntax *expression);

Syntax *logical_negation_new(Syntax *expression);

Syntax *addition_new(Syntax *left, Syntax *right);

Syntax *subtraction_new(Syntax *left, Syntax *right);

Syntax *multiplication_new(Syntax *left, Syntax *right);

Syntax *less_than_new(Syntax *left, Syntax *right);

Syntax *less_or_equal_new(Syntax *left, Syntax *right);

Syntax *function_call_new(char *function_name, Syntax *func_args);

Syntax *function_arguments_new();

Syntax *assignment_new(char *var_name, Syntax *expression);

Syntax *return_statement_new(Syntax *expression);

Syntax *block_new(List *statements);

Syntax *if_new(Syntax *condition, Syntax *then);

Syntax *define_var_new(char *var_name, Syntax *init_value);

Syntax *while_new(Syntax *condition, Syntax *body);

Syntax *function_new(char *name, Syntax *root_block);

Syntax *top_level_new();

void syntax_free(Syntax *syntax);

char *syntax_type_name(Syntax *syntax);

void print_syntax(Syntax *syntax);

#endif
