typedef struct Context {
    int stack_offset;
    Environment *env; //
    int label_count; //标签序号（生成新标签时用）
} Context;

void new_scope(Context *ctx);

Context *new_context();

void context_free(Context *ctx);
