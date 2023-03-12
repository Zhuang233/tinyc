#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <err.h>
#include "syntax.h"
#include "environment.h"
#include "context.h"

static const int WORD_SIZE = 4;
const int MAX_MNEMONIC_LENGTH = 7;

//写入汇编头部
void emit_header(FILE *out, char *name) { fprintf(out, "%s\n", name); }

//写入单操作数指令
void emit_instr(FILE *out, char *instr, char *operands) {
    // 4个空格作为缩进，输出指令
    fprintf(out, "    %s", instr);

    //确保参数左对齐
    int argument_offset = MAX_MNEMONIC_LENGTH - strlen(instr) + 4;
    while (argument_offset > 0) {
        fprintf(out, " ");
        argument_offset--;
    }

    fprintf(out, "%s\n", operands);
}

//写入多操作数指令
void emit_instr_format(FILE *out, char *instr, char *operands_format, ...) {
    // 4个空格作为缩进，输出指令
    fprintf(out, "    %s", instr);

    //确保参数左对齐
    int argument_offset = MAX_MNEMONIC_LENGTH - strlen(instr) + 4;
    while (argument_offset > 0) {
        fprintf(out, " ");
        argument_offset--;
    }
    
    //不定个数参数处理
    va_list argptr;
    va_start(argptr, operands_format);
    vfprintf(out, operands_format, argptr);
    va_end(argptr);

    fputs("\n", out);
}

//生成新标签
char *fresh_local_label(char *prefix, Context *ctx) {
    // 标签名总共不能超过8个字符（包括'.'和'_'）
    size_t buffer_size = strlen(prefix) + 8;
    char *buffer = malloc(buffer_size);

    snprintf(buffer, buffer_size, ".%s_%d", prefix, ctx->label_count);
    ctx->label_count++;

    return buffer;
}

//语句标号
void emit_label(FILE *out, char *label) { fprintf(out, "%s:\n", label); }

//函数标号
void emit_function_declaration(FILE *out, char *name) {
    fprintf(out, "    .global %s\n", name);
    fprintf(out, "%s:\n", name);
}

//函数体头部（建立堆栈框架）
void emit_function_prologue(FILE *out) {
    emit_instr(out, "pushl", "%ebp");
    emit_instr(out, "mov", "%esp, %ebp");
    fprintf(out, "\n");
}

//函数体尾部（撤销堆栈框架）
void emit_return(FILE *out) {
    fprintf(out, "    leave\n");
    fprintf(out, "    ret\n");
}

void emit_function_epilogue(FILE *out) {
    emit_return(out);
    fprintf(out, "\n");
}

//写入汇编头部
void write_header(FILE *out) { emit_header(out, "    .text"); }

//写入汇编尾部
void write_footer(FILE *out) {
    emit_function_declaration(out, "_start");
    emit_function_prologue(out);
    emit_instr(out, "call", "main");
    emit_instr(out, "mov", "%eax, %ebx");
    emit_instr(out, "mov", "$1, %eax");
    emit_instr(out, "int", "$0x80");
}

//写程序主体汇编
void write_syntax(FILE *out, Syntax *syntax, Context *ctx) {
    //翻译一元运算符
    if (syntax->type == UNARY_OPERATOR) {
        UnaryExpression *unary_syntax = syntax->unary_expression;
        //递归求完表达式，传到eax
        write_syntax(out, unary_syntax->expression, ctx);
        //取反或取非
        if (unary_syntax->unary_type == BITWISE_NEGATION) {
            emit_instr(out, "not", "%eax");
        } else {
            //应实现效果 只有eax为0时al为1
            emit_instr(out, "test", "$0xFFFFFFFF, %eax");
            emit_instr(out, "setz", "%al");
        }


    //处理立即数
    } else if (syntax->type == IMMEDIATE) {
        emit_instr_format(out, "mov", "$%d, %%eax", syntax->immediate->value);


    //处理变量
    } else if (syntax->type == VARIABLE) {
        //局部变量放到栈帧对应的位置
        emit_instr_format(
            out, "mov", "%d(%%ebp), %%eax",
            environment_get_offset(ctx->env, syntax->variable->var_name));


    // 处理二元运算符
    } else if (syntax->type == BINARY_OPERATOR) {
        BinaryExpression *binary_syntax = syntax->binary_expression;
        int stack_offset = ctx->stack_offset;//记录当前栈顶
        //留左边表达式的空间
        ctx->stack_offset -= WORD_SIZE;
        emit_instr(out, "sub", "$4, %esp");

        //计算左表达式值，存入栈中
        write_syntax(out, binary_syntax->left, ctx);
        emit_instr_format(out, "mov", "%%eax, %d(%%ebp)", stack_offset);

        //计算右表达式值
        write_syntax(out, binary_syntax->right, ctx);

        //写入二元表达式具体执行指令
        if (binary_syntax->binary_type == MULTIPLICATION) {
            emit_instr_format(out, "mull", "%d(%%ebp)", stack_offset);

        } else if (binary_syntax->binary_type == ADDITION) {
            emit_instr_format(out, "add", "%d(%%ebp), %%eax", stack_offset);

        } else if (binary_syntax->binary_type == SUBTRACTION) {
            emit_instr_format(out, "sub", "%%eax, %d(%%ebp)", stack_offset);
            emit_instr_format(out, "mov", "%d(%%ebp), %%eax", stack_offset);

        } else if (binary_syntax->binary_type == LESS_THAN) {
            emit_instr_format(out, "cmp", "%%eax, %d(%%ebp)", stack_offset);
            emit_instr(out, "setl", "%al");
            emit_instr(out, "movzbl", "%al, %eax");
        } else if (binary_syntax->binary_type == LESS_THAN_OR_EQUAL) {
            emit_instr_format(out, "cmp", "%%eax, %d(%%ebp)", stack_offset);
            emit_instr(out, "setle", "%al");
            emit_instr(out, "movzbl", "%al, %eax");
        }

    //翻译赋值语句
    } else if (syntax->type == ASSIGNMENT) {
        //计算赋值号右边表达式的值
        write_syntax(out, syntax->assignment->expression, ctx);

        //存入栈中
        emit_instr_format(
            out, "mov", "%%eax, %d(%%ebp)",
            environment_get_offset(ctx->env, syntax->variable->var_name));

    } else if (syntax->type == RETURN_STATEMENT) {
        ReturnStatement *return_statement = syntax->return_statement;
        //计算return后的表达式的值
        write_syntax(out, return_statement->expression, ctx);
        //撤销堆栈框架
        emit_return(out);

    //翻译函数调用
    } else if (syntax->type == FUNCTION_CALL) {
        emit_instr_format(out, "call", syntax->function_call->function_name);

    //处理if语句
    } else if (syntax->type == IF_STATEMENT) {
        IfStatement *if_statement = syntax->if_statement;

        //计算条件表达式
        write_syntax(out, if_statement->condition, ctx);

        //生成新标签
        char *label = fresh_local_label("if_end", ctx);

        //判断，跳转
        emit_instr(out, "test", "%eax, %eax");
        emit_instr_format(out, "jz", "%s", label);

        //写入then后面的语句
        write_syntax(out, if_statement->then, ctx);

        //写入.if_end_x
        emit_label(out, label);

    //翻译while语句
    } else if (syntax->type == WHILE_SYNTAX) {
        WhileStatement *while_statement = syntax->while_statement;

        //新建循环开始和结束便签
        char *start_label = fresh_local_label("while_start", ctx);
        char *end_label = fresh_local_label("while_end", ctx);

        //写循环开始标签
        emit_label(out, start_label);

        //计算条件
        write_syntax(out, while_statement->condition, ctx);

        //判断，跳转
        emit_instr(out, "test", "%eax, %eax");
        emit_instr_format(out, "jz", "%s", end_label);

        //翻译循环体
        write_syntax(out, while_statement->body, ctx);

        //无条件跳转
        emit_instr_format(out, "jmp", "%s", start_label);

        //写循环结束标签
        emit_label(out, end_label);

    //翻译变量定义语句
    } else if (syntax->type == DEFINE_VAR) {
        //记录变量在栈中的偏移量，即当前的栈顶
        DefineVarStatement *define_var_statement = syntax->define_var_statement;
        int stack_offset = ctx->stack_offset;
        environment_set_offset(ctx->env, define_var_statement->var_name,
                               stack_offset);
        //esp向下移4字节（int）
        emit_instr(out, "sub", "$4, %esp");
        ctx->stack_offset -= WORD_SIZE;

        //计算变量初始化
        write_syntax(out, define_var_statement->init_value, ctx);

        //将变量初始值写入栈中
        emit_instr_format(out, "mov", "%%eax, %d(%%ebp)\n", stack_offset);

    } else if (syntax->type == BLOCK) {
        //遍历语句列表，翻译各个语句
        List *statements = syntax->block->statements;
        for (int i = 0; i < list_length(statements); i++) {
            write_syntax(out, list_get(statements, i), ctx);
        }
    

    //翻译函数定义
    } else if (syntax->type == FUNCTION) {
        //新建函数的局部变量信息记录结构体
        new_scope(ctx);

        //写入函数标号
        emit_function_declaration(out, syntax->function->name);

        //建立新的堆栈框架
        emit_function_prologue(out);

        //写入函数的主体语句
        write_syntax(out, syntax->function->root_block, ctx);

        //撤销堆栈框架
        emit_function_epilogue(out);


    //处理根节点
    } else if (syntax->type == TOP_LEVEL) {
        //遍历函数节点列表，翻译各个函数
        List *declarations = syntax->top_level->declarations;
        for (int i = 0; i < list_length(declarations); i++) {
            write_syntax(out, list_get(declarations, i), ctx);
        }

    //如果是未知树节点，报错
    } else {
        warnx("Unknown syntax %s", syntax_type_name(syntax));
        assert(false);
    }
}

//主框架
void write_assembly(Syntax *syntax) {
    FILE *out = fopen("out.s", "wb");

    //写入汇编头部
    write_header(out);

    //写入主体部分
    Context *ctx = new_context();
    write_syntax(out, syntax, ctx);
    
    //写入汇编尾部
    write_footer(out);

    //释放空间
    context_free(ctx);
    fclose(out);
}
