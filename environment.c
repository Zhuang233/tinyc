#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "environment.h"


//环境变量，符号表
Environment *environment_new() {
    Environment *env = malloc(sizeof(Environment));
    env->size = 0;
    env->items = NULL;

    return env;
}

void environment_set_offset(Environment *env, char *var_name, int offset) {
    env->size++;
    env->items = realloc(env->items, env->size * sizeof(VarWithOffset));

    VarWithOffset *vwo = &env->items[env->size - 1];
    vwo->var_name = var_name;
    vwo->offset = offset;
}

//获取变量相对于ebp的偏移量
int environment_get_offset(Environment *env, char *var_name) {
    VarWithOffset vwo;
    for (size_t i = 0; i < env->size; i++) {
        vwo = env->items[i];

        if (strcmp(vwo.var_name, var_name) == 0) {
            return vwo.offset;
        }
    }

    warnx("Could not find %s in environment", var_name);
    return -1;
}

void environment_free(Environment *env) {
    if (env != NULL) {
        free(env->items);
        free(env);
    }
}
