// QuickJS internal structures for LunaJS code generation
//
// This header exposes internal QuickJS structures needed by the Lua code
// generator. It should only be included by LunaJS source files.

#ifndef QUICKJS_INTERNAL_H
#define QUICKJS_INTERNAL_H

#include "quickjs.h"
#include "list.h"

// GC object types (subset needed for bytecode)
typedef enum {
    JS_GC_OBJ_TYPE_JS_OBJECT,
    JS_GC_OBJ_TYPE_FUNCTION_BYTECODE,
    // ... other types not needed
} JSGCObjectTypeEnum;

struct JSGCObjectHeader {
    int ref_count;
    JSGCObjectTypeEnum gc_obj_type : 4;
    uint8_t mark : 1;
    uint8_t dummy0: 3;
    uint8_t dummy1;
    uint16_t dummy2;
    struct list_head link;
};

typedef enum {
    JS_CLOSURE_LOCAL,
    JS_CLOSURE_ARG,
    JS_CLOSURE_REF,
    JS_CLOSURE_GLOBAL_REF,
    JS_CLOSURE_GLOBAL_DECL,
    JS_CLOSURE_GLOBAL,
    JS_CLOSURE_MODULE_DECL,
    JS_CLOSURE_MODULE_IMPORT,
} JSClosureTypeEnum;

typedef enum {
    JS_VAR_NORMAL,
    JS_VAR_FUNCTION_DECL,
    JS_VAR_NEW_FUNCTION_DECL,
    JS_VAR_CATCH,
    JS_VAR_FUNCTION_NAME,
    JS_VAR_PRIVATE_FIELD,
    JS_VAR_PRIVATE_METHOD,
    JS_VAR_PRIVATE_GETTER,
    JS_VAR_PRIVATE_SETTER,
    JS_VAR_PRIVATE_GETTER_SETTER,
    JS_VAR_GLOBAL_FUNCTION_DECL,
} JSVarKindEnum;

typedef struct JSClosureVar {
    JSClosureTypeEnum closure_type : 3;
    uint8_t is_lexical : 1;
    uint8_t is_const : 1;
    uint8_t var_kind : 4;
    uint16_t var_idx;
    JSAtom var_name;
} JSClosureVar;

typedef struct JSBytecodeVarDef {
    JSAtom var_name;
    int scope_next;
    uint8_t is_const : 1;
    uint8_t is_lexical : 1;
    uint8_t is_captured : 1;
    uint8_t has_scope: 1;
    uint8_t var_kind : 4;
    uint16_t var_ref_idx;
} JSBytecodeVarDef;

typedef struct JSFunctionBytecode {
    struct JSGCObjectHeader header;
    uint8_t js_mode;
    uint8_t has_prototype : 1;
    uint8_t has_simple_parameter_list : 1;
    uint8_t is_derived_class_constructor : 1;
    uint8_t need_home_object : 1;
    uint8_t func_kind : 2;
    uint8_t new_target_allowed : 1;
    uint8_t super_call_allowed : 1;
    uint8_t super_allowed : 1;
    uint8_t arguments_allowed : 1;
    uint8_t has_debug : 1;
    uint8_t read_only_bytecode : 1;
    uint8_t is_direct_or_indirect_eval : 1;
    uint8_t *byte_code_buf;
    int byte_code_len;
    JSAtom func_name;
    JSBytecodeVarDef *vardefs;
    JSClosureVar *closure_var;
    uint16_t arg_count;
    uint16_t var_count;
    uint16_t defined_arg_count;
    uint16_t stack_size;
    uint16_t var_ref_count;
    JSContext *realm;
    JSValue *cpool;
    int cpool_count;
    int closure_var_count;
    struct {
        JSAtom filename;
        int source_len;
        int pc2line_len;
        uint8_t *pc2line_buf;
        char *source;
    } debug;
} JSFunctionBytecode;

// Enable short opcodes (must be defined before including quickjs-opcode.h)
#define SHORT_OPCODES 1

// Opcode definitions
typedef enum {
#define FMT(f)
#define DEF(id, size, n_pop, n_push, f) OP_##id,
#define def(id, size, n_pop, n_push, f)
#include "quickjs-opcode.h"
#undef def
#undef DEF
#undef FMT
    OP_COUNT,
    OP_TEMP_START = OP_nop + 1,
    OP___dummy = OP_TEMP_START - 1,
#define FMT(f)
#define DEF(id, size, n_pop, n_push, f)
#define def(id, size, n_pop, n_push, f) OP_##id,
#include "quickjs-opcode.h"
#undef def
#undef DEF
#undef FMT
    OP_TEMP_END,
} OPCodeEnum;

// Opcode format types
typedef enum {
#define FMT(f) OP_FMT_##f,
#define DEF(id, size, n_pop, n_push, f)
#include "quickjs-opcode.h"
#undef DEF
#undef FMT
} OPCodeFormat;

// Opcode info
typedef struct {
    uint8_t size;
    uint8_t n_pop;
    uint8_t n_push;
    uint8_t fmt;
} JSOpCode;

// Helper to get bytecode from a JSValue
static inline JSFunctionBytecode *JS_GetFunctionBytecode(JSValueConst func_obj)
{
    int tag = JS_VALUE_GET_TAG(func_obj);
    if (tag == JS_TAG_FUNCTION_BYTECODE) {
        return JS_VALUE_GET_PTR(func_obj);
    }
    return NULL;
}

// Check if closure var is a global
static inline int JS_IsClosureVarGlobal(JSClosureVar *cv)
{
    return cv->closure_type == JS_CLOSURE_GLOBAL ||
           cv->closure_type == JS_CLOSURE_GLOBAL_DECL ||
           cv->closure_type == JS_CLOSURE_GLOBAL_REF;
}

#endif /* QUICKJS_INTERNAL_H */
