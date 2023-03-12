#include <stdbool.h>

#ifndef Tinyc_STACK_HEADER
#define Tinyc_STACK_HEADER

typedef struct Stack {
    int size;
    void **content;//指向 指针数组（数组元素为指向栈块的指针） 的指针
} Stack; 

Stack *stack_new();

void stack_free(Stack *stack);
void stack_push(Stack *stack, void *item);
void *stack_pop(Stack *stack);
void *stack_peek(Stack *stack);
bool stack_empty(Stack *stack);

#endif
