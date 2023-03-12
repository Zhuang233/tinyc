%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../syntax.h"
#include "../stack.h"

#define YYSTYPE char*

int yyparse(void);
int yylex(); // lex自动产生的词法分析器

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
	return 1;
}

extern FILE *yyin;

Stack *syntax_stack;

%}

// 定义终结符
%token INCLUDE HEADER_NAME
%token TYPE IDENTIFIER RETURN NUMBER
%token OPEN_BRACE CLOSE_BRACE
%token IF WHILE
%token LESS_OR_EQUAL


 // 左结合终结符
%left '='
%left '<'
%left '+'
%left '-'
%left '*'
// 不可结合终结符
%nonassoc '!'
%nonassoc '~'

%%

// 文法规则
program:
        function program
        {
            Syntax *top_level_syntax;
            if (stack_empty(syntax_stack)) {
                top_level_syntax = top_level_new();
            } else if (((Syntax *)stack_peek(syntax_stack))->type != TOP_LEVEL) {
                top_level_syntax = top_level_new();
            } else {
                top_level_syntax = stack_pop(syntax_stack);
            }

            list_push(top_level_syntax->top_level->declarations,
                      stack_pop(syntax_stack));
            stack_push(syntax_stack, top_level_syntax);
        }
        |
        ;

// 函数-> 类型 标识符 (形参列表){语句块}
function:
	TYPE IDENTIFIER '(' parameter_list ')' OPEN_BRACE block CLOSE_BRACE
        {
            Syntax *current_syntax = stack_pop(syntax_stack);
            stack_push(syntax_stack, function_new((char*)$2, current_syntax));
        }
        ;

// 形参列表-> 非空形参列表|空
parameter_list:
        nonempty_parameter_list
        |
        ;

// 形参列表-> 类型 标识符,形参列表|类型 标识符
nonempty_parameter_list:
        TYPE IDENTIFIER ',' parameter_list
        |
        TYPE IDENTIFIER
        ;

// 语句块-> 语句 语句块|空
block:
        statement block
        {
            Syntax *block_syntax;
            if (stack_empty(syntax_stack)) {
                block_syntax = block_new(list_new());
            } else if (((Syntax *)stack_peek(syntax_stack))->type != BLOCK) {
                block_syntax = block_new(list_new());
            } else {
                block_syntax = stack_pop(syntax_stack);
            }

            list_push(block_syntax->block->statements, stack_pop(syntax_stack));
            stack_push(syntax_stack, block_syntax);
        }
        |
        ;

// 实参列表-> 非空实参列表|空
argument_list:
        nonempty_argument_list
        |
        {
            // Empty argument list.
            stack_push(syntax_stack, function_arguments_new());
        }
        ;

// 非空实参列表-> 表达式,非空实参列表|表达式
nonempty_argument_list:
        expression ',' nonempty_argument_list
        {
            Syntax *arguments_syntax;
            if (stack_empty(syntax_stack)) {
                // This should be impossible, we shouldn't be able to
                // parse this on its own.
                assert(false);
            } else if (((Syntax *)stack_peek(syntax_stack))->type != FUNCTION_ARGUMENTS) {
                arguments_syntax = function_arguments_new();
            } else {
                arguments_syntax = stack_pop(syntax_stack);
            }

            list_push(arguments_syntax->function_arguments->arguments, stack_pop(syntax_stack));
            stack_push(syntax_stack, arguments_syntax);
        }
        |
        expression
        {
            if (stack_empty(syntax_stack)) {
                // This should be impossible, we shouldn't be able to
                // parse this on its own.
                assert(false);
            }

            Syntax *arguments_syntax = function_arguments_new();
            list_push(arguments_syntax->function_arguments->arguments, stack_pop(syntax_stack));

            stack_push(syntax_stack, arguments_syntax);
        }
        ;

// 语句-> return 表达式;
//      | if(表达式){语句块}
//      | while(表达式){语句块}
//      | 类型 标识符 = 表达式
//      | 表达式
statement:
        RETURN expression ';'
        {
            Syntax *current_syntax = stack_pop(syntax_stack);
            stack_push(syntax_stack, return_statement_new(current_syntax));
        }
        |
        IF '(' expression ')' OPEN_BRACE block CLOSE_BRACE
        {
            Syntax *then = stack_pop(syntax_stack);
            Syntax *condition = stack_pop(syntax_stack);
            stack_push(syntax_stack, if_new(condition, then));
        }
        |
        WHILE '(' expression ')' OPEN_BRACE block CLOSE_BRACE
        {
            Syntax *body = stack_pop(syntax_stack);
            Syntax *condition = stack_pop(syntax_stack);
            stack_push(syntax_stack, while_new(condition, body));
        }
        |
        TYPE IDENTIFIER '=' expression ';'
        {
            Syntax *init_value = stack_pop(syntax_stack);
            stack_push(syntax_stack, define_var_new((char*)$2, init_value));
        }
        |
        expression ';'
        {
            // 节点已存在
        }
        ;

// 表达式-> 数|标识符|标识符=表达式|~表达式|!表达式
//           |表达式+表达式|表达式-表达式|表达式*表达式|表达式<表达式|表达式 小等 表达式
//           |标识符(参数列表)
expression:
	NUMBER
        {
            stack_push(syntax_stack, immediate_new(atoi((char*)$1)));
            free($1);
        }
        |
	IDENTIFIER
        {
            stack_push(syntax_stack, variable_new((char*)$1));
        }
        |
	IDENTIFIER '=' expression
        {
            Syntax *expression = stack_pop(syntax_stack);
            stack_push(syntax_stack, assignment_new((char*)$1, expression));
        }
        |
        '~' expression
        {
            Syntax *current_syntax = stack_pop(syntax_stack);
            stack_push(syntax_stack, bitwise_negation_new(current_syntax));
        }
        |
        '!' expression
        {
            Syntax *current_syntax = stack_pop(syntax_stack);
            stack_push(syntax_stack, logical_negation_new(current_syntax));
        }
        |
        expression '+' expression
        {
            Syntax *right = stack_pop(syntax_stack);
            Syntax *left = stack_pop(syntax_stack);
            stack_push(syntax_stack, addition_new(left, right));
        }
        |
        expression '-' expression
        {
            Syntax *right = stack_pop(syntax_stack);
            Syntax *left = stack_pop(syntax_stack);
            stack_push(syntax_stack, subtraction_new(left, right));
        }
        |
        expression '*' expression
        {
            Syntax *right = stack_pop(syntax_stack);
            Syntax *left = stack_pop(syntax_stack);
            stack_push(syntax_stack, multiplication_new(left, right));
        }
        |
        expression '<' expression
        {
            Syntax *right = stack_pop(syntax_stack);
            Syntax *left = stack_pop(syntax_stack);
            stack_push(syntax_stack, less_than_new(left, right));
        }
        |
        expression LESS_OR_EQUAL expression
        {
            Syntax *right = stack_pop(syntax_stack);
            Syntax *left = stack_pop(syntax_stack);
            stack_push(syntax_stack, less_or_equal_new(left, right));
        }
        |
        IDENTIFIER '(' argument_list ')'
        {
            Syntax *arguments = stack_pop(syntax_stack);
            stack_push(syntax_stack, function_call_new((char*)$1, arguments));
        }
        ;
