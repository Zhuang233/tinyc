#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "stack.h"

//新建栈
Stack *stack_new() {
    Stack *stack = malloc(sizeof(Stack));
    stack->size = 0;
    stack->content = 0;

    return stack;
}

//释放栈
void stack_free(Stack *stack) {
    if (stack->size > 0) {
        free(stack->content);
    }
    free(stack);
}

//压栈
void stack_push(Stack *stack, void *item) {
    stack->size++;

    // 扩展内存空间，并将入栈元素的指针加到后面
    stack->content =
        realloc(stack->content, stack->size * sizeof *stack->content);
    stack->content[stack->size - 1] = item;
}

//弹栈
void *stack_pop(Stack *stack) {
    //检查是否栈空
    assert(stack->size >= 1);

    //缩减内存空间
    stack->size--;
    void *item = stack->content[stack->size];
    stack->content =
        realloc(stack->content, stack->size * sizeof *stack->content);
    return item;
}

//取栈顶
void *stack_peek(Stack *stack) {
    //检查是否栈空
    assert(stack->size >= 1);

    return stack->content[stack->size - 1];
}

//检查是否栈空
bool stack_empty(Stack *stack) { return stack->size == 0; }
