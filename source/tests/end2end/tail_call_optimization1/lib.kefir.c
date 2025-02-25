/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "definitions.h"

struct bf_context {
    const char *code;
    unsigned char mem[4096];
    unsigned long stack[128];

    unsigned int code_pointer;
    unsigned short mem_pointer;
    unsigned char stack_pointer;

    bf_output_t output;
};

typedef int (*op_fn_t)(struct bf_context *);
static op_fn_t ops[];

#define DUMMY_SIZE (1024 * 64)
#define RUN_NEXT(_ops, _ctx)                                  \
    ({                                                        \
        volatile char array[DUMMY_SIZE];                      \
        (void) array[0];                                      \
        (_ops)[(_ctx)->code[(_ctx)->code_pointer++]]((_ctx)); \
    })

static int op_plus(struct bf_context *ctx) {
    ctx->mem[ctx->mem_pointer]++;
    return RUN_NEXT(ops, ctx);
}

static int op_minus(struct bf_context *ctx) {
    ctx->mem[ctx->mem_pointer]--;
    return RUN_NEXT(ops, ctx);
}

static int op_next(struct bf_context *ctx) {
    ctx->mem_pointer++;
    return RUN_NEXT(ops, ctx);
}

static int op_prev(struct bf_context *ctx) {
    ctx->mem_pointer--;
    return RUN_NEXT(ops, ctx);
}

static int op_loop_begin(struct bf_context *ctx) {
    if (ctx->mem[ctx->mem_pointer] != 0) {
        ctx->stack[ctx->stack_pointer++] = ctx->code_pointer;
    } else {
        unsigned int found_brackets = 0;
        do {
            char op = ctx->code[ctx->code_pointer++];
            if (op == '[') {
                found_brackets++;
            } else if (op == ']') {
                found_brackets--;
            }
        } while (found_brackets > 0);
    }
    return RUN_NEXT(ops, ctx);
}

static int op_loop_end(struct bf_context *ctx) {
    --ctx->stack_pointer;
    if (ctx->mem[ctx->mem_pointer] != 0) {
        ctx->code_pointer = ctx->stack[ctx->stack_pointer] - 1;
    }
    return RUN_NEXT(ops, ctx);
}

static int op_print(struct bf_context *ctx) {
    ctx->output(ctx->mem[ctx->mem_pointer]);
    return RUN_NEXT(ops, ctx);
}

static int op_terminate(struct bf_context *ctx) {
    return 0;
}

static op_fn_t ops[] = {['+'] = op_plus,       ['-'] = op_minus,    ['>'] = op_next,  ['<'] = op_prev,
                        ['['] = op_loop_begin, [']'] = op_loop_end, ['.'] = op_print, ['\0'] = op_terminate};

void run_bf(const char *code, bf_output_t output) {
    struct bf_context ctx = {.code = code,
                             .mem = {0},
                             .stack = {0},
                             .code_pointer = 0,
                             .mem_pointer = 0,
                             .stack_pointer = 0,
                             .output = output};
    (void) RUN_NEXT(ops, &ctx);
}
