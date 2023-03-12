#include "environment.h"
#include "context.h"

//只支持int类型 4字节
static const int WORD_SIZE = 4;

void new_scope(Context *ctx) {
    //每个函数都需要一组新的局部变量
    //不支持全局变量

    //新建一组局部变量换掉旧的
    environment_free(ctx->env);
    ctx->env = environment_new();

    ctx->stack_offset = -1 * WORD_SIZE;
}

Context *new_context() {
    Context *ctx = malloc(sizeof(Context));
    ctx->stack_offset = 0;
    ctx->env = NULL;
    ctx->label_count = 0;

    return ctx;
}

void context_free(Context *ctx) {
    environment_free(ctx->env);
    free(ctx);
}
