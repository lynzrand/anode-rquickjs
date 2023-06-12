#include "anode-ext.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "core/builtins/js-big-num.h"
#include "core/builtins/js-operator.h"
#include "core/quickjs-internals.h"
#include "quickjs/quickjs.h"

JSFunctionBytecode* anode_get_function_bytecode(JSValue function) {
  int tag = JS_VALUE_GET_TAG(function);
  if (tag != JS_TAG_OBJECT) {
    fprintf(stderr, "anode_get_function_bytecode: expected object, got tag %d\n", tag);
    abort();
  }

  JSObject* obj = JS_VALUE_GET_OBJ(function);
  if (obj->class_id != JS_CLASS_BYTECODE_FUNCTION) {
    fprintf(stderr, "anode_get_function_bytecode: expected bytecode function, got class id %d\n", obj->class_id);
    abort();
  }

  return obj->u.func.function_bytecode;
}

int32_t anode_js_to_bool(JSContext* ctx, JSValue op1) {
  int32_t res;
  if ((uint32_t)JS_VALUE_GET_TAG(op1) <= JS_TAG_UNDEFINED) {
    res = JS_VALUE_GET_INT(op1);
  } else {
    res = JS_ToBoolFree(ctx, op1);
  }
  return res;
}

JSValue anode_js_add_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    int32_t sum = JS_VALUE_GET_INT(x) + JS_VALUE_GET_INT(y);
    // check for overflow
    if ((sum ^ JS_VALUE_GET_INT(x)) >= 0) {
      return JS_NewInt32(ctx, sum);
    } else {
      return JS_NewFloat64(ctx, (double)JS_VALUE_GET_INT(x) + (double)JS_VALUE_GET_INT(y));
    }
  } else if (JS_VALUE_IS_BOTH_FLOAT(x, y)) {
    return JS_NewFloat64(ctx, JS_VALUE_GET_FLOAT64(x) + JS_VALUE_GET_FLOAT64(y));
  } else {
    JSValue args[] = {x, y};
    // js_xxx_slow functions expect the input sp at the end of the args array
    if (js_add_slow(ctx, args + 2)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_sub_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    int32_t diff = JS_VALUE_GET_INT(x) - JS_VALUE_GET_INT(y);
    // check for overflow
    if ((diff ^ JS_VALUE_GET_INT(x)) >= 0) {
      return JS_NewInt32(ctx, diff);
    } else {
      return JS_NewFloat64(ctx, (double)JS_VALUE_GET_INT(x) - (double)JS_VALUE_GET_INT(y));
    }
  } else if (JS_VALUE_IS_BOTH_FLOAT(x, y)) {
    return JS_NewFloat64(ctx, JS_VALUE_GET_FLOAT64(x) - JS_VALUE_GET_FLOAT64(y));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_arith_slow(ctx, args + 2, OP_sub)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_mul_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    int64_t prod = (int64_t)JS_VALUE_GET_INT(x) * (int64_t)JS_VALUE_GET_INT(y);
    // check for overflow
    if ((int32_t)prod == prod) {
      return JS_NewInt32(ctx, (int32_t)prod);
    } else {
      return JS_NewFloat64(ctx, (double)JS_VALUE_GET_INT(x) * (double)JS_VALUE_GET_INT(y));
    }
  } else if (JS_VALUE_IS_BOTH_FLOAT(x, y)) {
    return JS_NewFloat64(ctx, JS_VALUE_GET_FLOAT64(x) * JS_VALUE_GET_FLOAT64(y));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_arith_slow(ctx, args + 2, OP_mul)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_div_any(JSContext* ctx, JSValue x, JSValue y) {
  // division is always a float
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewFloat64(ctx, (double)JS_VALUE_GET_INT(x) / (double)JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_arith_slow(ctx, args + 2, OP_div)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_mod_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    int32_t x_int = JS_VALUE_GET_INT(x);
    int32_t y_int = JS_VALUE_GET_INT(y);
    if (y_int == 0) {
      return JS_EXCEPTION;
    }
    return JS_NewInt32(ctx, x_int % y_int);
  } else if (JS_VALUE_IS_BOTH_FLOAT(x, y)) {
    return JS_NewFloat64(ctx, fmod(JS_VALUE_GET_FLOAT64(x), JS_VALUE_GET_FLOAT64(y)));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_arith_slow(ctx, args + 2, OP_mod)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_pow_any(JSContext* ctx, JSValue x, JSValue y) {
  JSValue args[] = {x, y};
  if (js_binary_arith_slow(ctx, args + 2, OP_pow)) {
    return JS_EXCEPTION;
  }
  return args[0];
}

JSValue anode_js_bit_and_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewInt32(ctx, JS_VALUE_GET_INT(x) & JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_logic_slow(ctx, args + 2, OP_and)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_bit_or_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewInt32(ctx, JS_VALUE_GET_INT(x) | JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_logic_slow(ctx, args + 2, OP_or)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_bit_xor_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewInt32(ctx, JS_VALUE_GET_INT(x) ^ JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_logic_slow(ctx, args + 2, OP_xor)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_bit_not_any(JSContext* ctx, JSValue x) {
  if (JS_VALUE_GET_TAG(x) == JS_TAG_INT) {
    return JS_NewInt32(ctx, ~JS_VALUE_GET_INT(x));
  } else {
    JSValue args[] = {x};
    if (js_unary_arith_slow(ctx, args + 1, OP_not)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_shift_left_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewInt32(ctx, JS_VALUE_GET_INT(x) << (JS_VALUE_GET_INT(y) & 0x1f));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_logic_slow(ctx, args + 2, OP_shl)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_shift_right_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewInt32(ctx, JS_VALUE_GET_INT(x) >> (JS_VALUE_GET_INT(y) & 0x1f));
  } else {
    JSValue args[] = {x, y};
    if (js_shr_slow(ctx, args + 2)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_shift_right_arith_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewInt32(ctx, (int32_t)JS_VALUE_GET_INT(x) >> (JS_VALUE_GET_INT(y) & 0x1f));
  } else {
    JSValue args[] = {x, y};
    if (js_binary_logic_slow(ctx, args + 2, OP_sar)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_neg_any(JSContext* ctx, JSValue x) {
  if (JS_VALUE_GET_TAG(x) == JS_TAG_INT) {
    return JS_NewInt32(ctx, -JS_VALUE_GET_INT(x));
  } else if (JS_VALUE_GET_NORM_TAG(x) == JS_TAG_FLOAT64) {
    return JS_NewFloat64(ctx, -JS_VALUE_GET_FLOAT64(x));
  } else {
    JSValue args[] = {x};
    if (js_unary_arith_slow(ctx, args + 2, OP_neg)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_plus_any(JSContext* ctx, JSValue x) {
  if (JS_VALUE_GET_TAG(x) == JS_TAG_INT) {
    return x;
  } else if (JS_VALUE_GET_NORM_TAG(x) == JS_TAG_FLOAT64) {
    return x;
  } else {
    JSValue args[] = {x};
    if (js_unary_arith_slow(ctx, args + 2, OP_plus)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_not_any(JSContext* ctx, JSValue x) {
  if (JS_VALUE_GET_TAG(x) == JS_TAG_BOOL) {
    return JS_NewBool(ctx, !JS_VALUE_GET_BOOL(x));
  } else {
    JSValue args[] = {x};
    if (js_unary_arith_slow(ctx, args + 2, OP_not)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_lnot_any(JSContext* ctx, JSValue x) {
  int res;
  if ((uint32_t)JS_VALUE_GET_TAG(x) <= JS_TAG_UNDEFINED) {
    res = JS_VALUE_GET_INT(x) != 0;
  } else {
    res = JS_ToBoolFree(ctx, x);
  }
  return JS_NewBool(ctx, !res);
}

JSValue anode_js_typeof_any(JSContext* ctx, JSValue op1) {
  JSAtom atom = js_operator_typeof(ctx, op1);
  JS_FreeValue(ctx, op1);
  return JS_AtomToString(ctx, atom);
}
/*
#define OP_CMP(opcode, binary_op, slow_call)                                           \
  CASE(opcode) : {                                                                     \
    JSValue op1, op2;                                                                  \
    op1 = sp[-2];                                                                      \
    op2 = sp[-1];                                                                      \
    if (likely(JS_VALUE_IS_BOTH_INT(op1, op2))) {                                      \
      sp[-2] = JS_NewBool(ctx, JS_VALUE_GET_INT(op1) binary_op JS_VALUE_GET_INT(op2)); \
      sp--;                                                                            \
    } else {                                                                           \
      if (slow_call)                                                                   \
        goto exception;                                                                \
      sp--;                                                                            \
    }                                                                                  \
  }                                                                                    \
  BREAK

      OP_CMP(OP_lt, <, js_relational_slow(ctx, sp, opcode));
      OP_CMP(OP_lte, <=, js_relational_slow(ctx, sp, opcode));
      OP_CMP(OP_gt, >, js_relational_slow(ctx, sp, opcode));
      OP_CMP(OP_gte, >=, js_relational_slow(ctx, sp, opcode));
      OP_CMP(OP_eq, ==, js_eq_slow(ctx, sp, 0));
      OP_CMP(OP_neq, !=, js_eq_slow(ctx, sp, 1));
      OP_CMP(OP_strict_eq, ==, js_strict_eq_slow(ctx, sp, 0));
      OP_CMP(OP_strict_neq, !=, js_strict_eq_slow(ctx, sp, 1));

*/

JSValue anode_js_eq_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewBool(ctx, JS_VALUE_GET_INT(x) == JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_eq_slow(ctx, args + 2, 0)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_gt_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewBool(ctx, JS_VALUE_GET_INT(x) > JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_relational_slow(ctx, args + 2, OP_gt)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_ge_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewBool(ctx, JS_VALUE_GET_INT(x) >= JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_relational_slow(ctx, args + 2, OP_gte)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_lt_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewBool(ctx, JS_VALUE_GET_INT(x) < JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_relational_slow(ctx, args + 2, OP_lt)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_le_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewBool(ctx, JS_VALUE_GET_INT(x) <= JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_relational_slow(ctx, args + 2, OP_lte)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_ne_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewBool(ctx, JS_VALUE_GET_INT(x) != JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_eq_slow(ctx, args + 2, 1)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_strict_eq_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewBool(ctx, JS_VALUE_GET_INT(x) == JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_strict_eq_slow(ctx, args + 2, 0)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_strict_ne_any(JSContext* ctx, JSValue x, JSValue y) {
  if (JS_VALUE_IS_BOTH_INT(x, y)) {
    return JS_NewBool(ctx, JS_VALUE_GET_INT(x) != JS_VALUE_GET_INT(y));
  } else {
    JSValue args[] = {x, y};
    if (js_strict_eq_slow(ctx, args + 2, 1)) {
      return JS_EXCEPTION;
    }
    return args[0];
  }
}

JSValue anode_js_instance_of_any(JSContext* ctx, JSValue x, JSValue y) {
  JSValue args[] = {x, y};
  if (js_operator_instanceof(ctx, args + 2)) {
    return JS_EXCEPTION;
  }
  return args[0];
}

JSValue anode_js_in_any(JSContext* ctx, JSValue x, JSValue y) {
  JSValue args[] = {x, y};
  if (js_operator_in(ctx, args + 2)) {
    return JS_EXCEPTION;
  }
  return args[0];
}

JSValue anode_js_is_truthy_any(JSContext* ctx, JSValue x) {
  int res;
  if ((uint32_t)JS_VALUE_GET_TAG(x) <= JS_TAG_UNDEFINED) {
    res = JS_VALUE_GET_INT(x) != 0;
  } else {
    res = JS_ToBoolFree(ctx, x);
  }
  return JS_NewBool(ctx, res);
}
