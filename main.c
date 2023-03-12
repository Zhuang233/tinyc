#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <err.h>

#include "stack.h"
#include "build/y.tab.h"
#include "syntax.h"
#include "assembly.h"

void print_help() {
    printf("Tinyc is a very basic C compiler.\n\n");
    printf("To compile a file:\n");
    printf("    $ tinyc foo.c\n");
    printf("To output the AST without compiling:\n");
    printf("    $ tinyc --dump-ast foo.c\n");
    printf("To output the preprocessed code without parsing:\n");
    printf("    $ tinyc --dump-expansion foo.c\n");
    printf("To print this message:\n");
    printf("    $ tinyc --help\n\n");
    printf("For more information, see https://github.com/Wilfred/tinyc\n");
}

extern Stack *syntax_stack;

extern int yyparse(void);
extern FILE *yyin;

typedef enum {
    MACRO_EXPAND,//宏扩展阶段
    PARSE,//语法分析阶段
    EMIT_ASM,//生成汇编阶段
} stage_t;
//阶段

int main(int argc, char *argv[]) {
    printf("argc:%d  argv[0]:%s",argc,argv[0]);
    ++argv, --argc; //跳过程序名称

    stage_t terminate_at = EMIT_ASM;
    //参数解析
    char *file_name;
    if (argc == 1 && strcmp(argv[0], "--help") == 0) {
        print_help();
        return 0;
    } else if (argc == 1) {
        file_name = argv[0];
    } else if (argc == 2 && strcmp(argv[0], "--dump-expansion") == 0) {
        terminate_at = MACRO_EXPAND;
        file_name = argv[1];
    } else if (argc == 2 && strcmp(argv[0], "--dump-ast") == 0) {
        terminate_at = PARSE;
        file_name = argv[1];
    } else {
        print_help();
        return 1;
    }

    int result;

    //预处理,宏扩展
    char command[1024] = {0};
    snprintf(command, 1024, "gcc -E %s > .expanded.c", file_name);
    result = system(command);
    if (result != 0) {
        puts("Macro expansion failed!");
        return result;
    }

    yyin = fopen(".expanded.c", "r");

    //只打印宏扩展结果
    if (terminate_at == MACRO_EXPAND) {
        int c;
        while ((c = getc(yyin)) != EOF) {
            putchar(c);
        }
        goto cleanup_file;
    }


    //宏扩展后的文件打不开
    if (yyin == NULL) {
        printf("Could not open file: '%s'\n", file_name);
        result = 2;
        goto cleanup_file;
    }

    syntax_stack = stack_new();

    result = yyparse();//yacc语法分析入口
    if (result != 0) {
        printf("\n");
        goto cleanup_syntax;
    }
    
    Syntax *complete_syntax = stack_pop(syntax_stack);//从语法分析栈中拿到剩下的最后一个节点，即为根节点
    if (syntax_stack->size > 0) {//栈非空，语法分析没通过
        warnx("Did not consume the whole syntax stack during parsing! Remaining:");

        while(syntax_stack->size > 0) {
            fprintf(stderr, "%s", syntax_type_name(stack_pop(syntax_stack)));
        }
    }

    if (terminate_at == PARSE) {
        print_syntax(complete_syntax);//打印语法分析树
    } else {
        write_assembly(complete_syntax);//翻译成汇编
        syntax_free(complete_syntax);//释放语法树的空间

        printf("Written out.s.\n");
        printf("Build it with:\n");
        printf("    $ as out.s -o out.o\n");
        printf("    $ ld -s -o out out.o\n");
    }

cleanup_syntax:
    //语法分析错误后释放之前申请的空间
    stack_free(syntax_stack);
cleanup_file:
    //关闭文件
    if (yyin != NULL) {
        fclose(yyin);
    }
    //删除文件
    unlink(".expanded.c");

    return result;
}
