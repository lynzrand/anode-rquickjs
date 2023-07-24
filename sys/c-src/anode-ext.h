#ifndef ANODE_EXT_H
#define ANODE_EXT_H
#include <string.h>

#include "core/quickjs-internals.h"
#include "core/types.h"
#include "quickjs/list.h"
#include "quickjs/quickjs.h"

// This file is used to generate LLVM IR for the API of Anode.

#pragma region JSValueManipulations
// This section defines utility functions that acts the same as QuickJS's
// macros.
__attribute((always_inline)) static inline int32_t
anode_js_value_get_tag(JSValueConst val) {
  return JS_VALUE_GET_TAG(val);
}

__attribute((always_inline)) static inline int32_t
anode_js_value_get_norm_tag(JSValueConst val) {
  return JS_VALUE_GET_NORM_TAG(val);
}

__attribute((always_inline)) static inline int32_t
anode_js_value_get_int(JSValueConst val) {
  return JS_VALUE_GET_INT(val);
}

__attribute((always_inline)) static inline int32_t
anode_js_value_get_bool(JSValueConst val) {
  return JS_VALUE_GET_BOOL(val);
}

__attribute((always_inline)) static inline int32_t
anode_js_value_get_float64(JSValueConst val) {
  return JS_VALUE_GET_FLOAT64(val);
}

__attribute((always_inline)) static inline void*
anode_js_value_get_ptr(JSValueConst val) {
  return JS_VALUE_GET_PTR(val);
}

__attribute((always_inline)) static inline JSValue
anode_js_new_int32(int32_t tag, int32_t val) {
  return JS_MKVAL(tag, val);
}

__attribute((always_inline)) static inline JSValue
anode_js_new_ptr(int32_t tag, void* ptr) {
  return JS_MKPTR(tag, ptr);
}

__attribute((always_inline)) static inline JSValue
anode_js_new_float64(JSContext* ctx, double d) {
  return JS_NewFloat64(ctx, d);
}

#pragma endregion

#pragma region Function Bytecode

/// Does the same bookkeeping stuff as the start of `JS_CallInternal`
static inline void anode_stackframe_init(
  JSContext* ctx,
  JSStackFrame* sf,
  JSValueConst func,
  int32_t argc,
  JSValue* argv) {
  memset(sf, 0, sizeof(*sf));
  sf->prev_frame = ctx->rt->current_stack_frame;
  sf->cur_func = func;
  sf->arg_buf = argv;
  sf->arg_count = argc;
  init_list_head(&sf->var_ref_list);
  ctx->rt->current_stack_frame = sf;
}

static inline void
anode_stackframe_add_locals(JSStackFrame* sf, JSValue* locv) {
  sf->var_buf = locv;
}

static inline void anode_stackframe_set_pc(JSStackFrame* sf, int pc) {
  sf->cur_pc = ((JSObject*)JS_VALUE_GET_PTR(sf->cur_func))
                 ->u.func.function_bytecode->byte_code_buf
    + pc;
}

/// Does the same bookkeeping stuff as the end of `JS_CallInternal`
static inline void anode_stackframe_exit(JSContext* ctx, JSStackFrame* sf) {
  ctx->rt->current_stack_frame = sf->prev_frame;
}

JSFunctionBytecode* anode_get_function_bytecode(JSValueConst function);

__attribute((always_inline)) static inline JSValue
anode_function_get_cpool_unchecked(JSFunctionBytecode* bc, int32_t ix) {
  return bc->cpool[ix];
}

JSValue anode_get_function_var_ref(
  JSContext* ctx,
  JSValueConst function,
  int32_t var_idx);

int32_t anode_js_to_bool(JSContext* ctx, JSValueConst val);

JSValue anode_js_add_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_sub_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_mul_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_div_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_mod_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_pow_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_bit_and_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_bit_or_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_bit_xor_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_not_any(JSContext* ctx, JSValue x);
JSValue anode_js_shift_left_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_shift_right_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_shift_right_arith_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_neg_any(JSContext* ctx, JSValue x);
JSValue anode_js_plus_any(JSContext* ctx, JSValue x);
JSValue anode_js_lnot_any(JSContext* ctx, JSValue x);
JSValue anode_js_typeof_any(JSContext* ctx, JSValue x);
JSValue anode_js_eq_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_gt_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_ge_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_lt_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_le_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_ne_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_strict_eq_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_strict_ne_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_instance_of_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_in_any(JSContext* ctx, JSValue x, JSValue y);
JSValue anode_js_is_truthy_any(JSContext* ctx, JSValue x);
JSValue anode_js_inc_any(JSContext* ctx, JSValue x);
JSValue anode_js_dec_any(JSContext* ctx, JSValue x);

JSValue anode_run_eval(
  JSContext* ctx,
  int scope_idx,
  JSValueConst eval_obj,
  int call_argc,
  JSValueConst* call_argv);

#endif
