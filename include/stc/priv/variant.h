/* MIT License
 *
 * Copyright (c) 2024 Tyge Løvset
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/*
// https://stackoverflow.com/questions/70935435/how-to-create-variants-in-rust
#include <stdio.h>
#include "stc/cstr.h"
#include "stc/algorithm.h"

c_sumtype (Action,
    (ActionSpeak, cstr),
    (ActionQuit, _Bool),
    (ActionRunFunc, struct {
        int32 (*func)(int32, int32);
        int32 v1, v2;
    })
);

void Action_drop(Action* self) {
    c_match (self) {
        c_is(ActionSpeak, x) cstr_drop(x);
    }
}

void action(Action* action) {
    c_match (action) {
        c_is(ActionSpeak, s) {
            printf("Asked to speak: %s\n", cstr_str(s));
        }
        c_is(ActionQuit) {
            printf("Asked to quit!\n");
        }
        c_is(ActionRunFunc, r) {
            int32 res = r->func(r->v1, r->v2);
            printf("v1: %d, v2: %d, res: %d\n", r->v1, r->v2, res);
        }
        c_otherwise assert(!"no match");
    }
}

int32 add(int32 a, int32 b) {
    return a + b;
}

int main(void) {
    Action act1 = c_variant(ActionSpeak, cstr_from("Hello"));
    Action act2 = c_variant(ActionQuit, 1);
    Action act3 = c_variant(ActionRunFunc, {add, 5, 6});

    action(&act1);
    action(&act2);
    action(&act3);

    c_drop(Action, &act1, &act2, &act3);
}
*/
#ifndef STC_VARIANT_H_INCLUDED
#define STC_VARIANT_H_INCLUDED

#include "stc/common.h"

#define _c_EMPTY()
#define _c_CALL(f, ...) f(__VA_ARGS__)
#define _c_LOOP_INDIRECTION() c_LOOP
#define _c_LOOP_END_1 ,_c_LOOP1
#define _c_LOOP0(f,T,x,...) _c_CALL(f,T,c_EXPAND x) _c_LOOP_INDIRECTION _c_EMPTY()()(f,T,__VA_ARGS__)
#define _c_LOOP1(...)
#define _c_TUPLE_AT_1(x,y,...) y
#define _c_MACRO2(a1,a2,x,...) x
#define _c_CHECK(x,...) _c_TUPLE_AT_1(__VA_ARGS__,x,)
#define _c_E0(...) __VA_ARGS__
#define _c_E1(...) _c_E0(_c_E0(_c_E0(_c_E0(_c_E0(__VA_ARGS__)))))
#define _c_E2(...) _c_E1(_c_E1(_c_E1(_c_E1(_c_E1(__VA_ARGS__)))))
#define c_EVAL(...) _c_E2(_c_E2(_c_E2(_c_E2(__VA_ARGS__))))
#define c_LOOP(f,T,x,...) _c_CHECK(_c_LOOP0, c_JOIN(_c_LOOP_END_, c_NUMARGS x))(f,T,x,__VA_ARGS__)


#define _c_vartuple_tag(T, Tag, ...) Tag,
#define _c_vartuple_type(T, Tag, ...) typedef __VA_ARGS__ Tag##_type; typedef T Tag##_sumtype;
#define _c_vartuple_var(T, Tag, ...) struct { uint8_t tag; Tag##_type var; } Tag;

#define c_sumtype(T, ...) \
    typedef union T T; \
    c_EVAL(c_LOOP(_c_vartuple_type, T,  __VA_ARGS__, (0),)) \
    enum enum_##T { null_##T, c_EVAL(c_LOOP(_c_vartuple_tag, T, __VA_ARGS__, (0),)) }; \
    union T { \
        struct { uint8_t tag; } _any_; \
        c_EVAL(c_LOOP(_c_vartuple_var, T, __VA_ARGS__, (0),)) \
    }

#if defined __GNUC__ || defined __clang__ || defined __TINYC__ || _MSC_VER >= 1939
    #define c_match(var) \
        for (__typeof__(var) _match = (var); _match; _match = NULL) \
        switch (_match->_any_.tag)

    #define c_is_2(Tag, x) \
        break; case Tag: \
        for (__typeof__(_match->Tag.var)* x = &_match->Tag.var; x; x = NULL)

    #define c_if_is(v, Tag, x) \
        for (__typeof__(v) _var = (v); _var; _var = NULL) \
            if (c_holds(_var, Tag)) \
                for (__typeof__(_var->Tag.var) *x = &_var->Tag.var; x; x = NULL)
#else
    typedef union { struct { uint8_t tag; } _any_; } _c_any_variant;
    #define c_match(var) \
        for (_c_any_variant* _match = (_c_any_variant *)(var) + 0*sizeof((var)->_any_.tag); \
             _match; _match = NULL) \
            switch (_match->_any_.tag)

    #define c_is_2(Tag, x) \
        break; case Tag: \
        for (Tag##_type *x = &((Tag##_sumtype *)_match)->Tag.var; x; x = NULL)

    #define c_if_is(v, Tag, x) \
        for (Tag##_sumtype* _var = c_const_cast(Tag##_sumtype*, v); _var; _var = NULL) \
            if (c_holds(_var, Tag)) \
                for (Tag##_type *x = &_var->Tag.var; x; x = NULL)
#endif

#define c_is(...) _c_MACRO2(__VA_ARGS__,c_is_2,c_is_1)(__VA_ARGS__)
#define c_is_1(Tag) \
    break; case Tag:

#define c_or_is(Tag) \
    ; case Tag:

#define c_otherwise \
    break; default:

#define c_variant(Tag, ...) \
    (c_literal(Tag##_sumtype){.Tag={.tag=Tag, .var=__VA_ARGS__}})

#define c_tag_index(var) \
    ((var)->_any_.tag)

#define c_holds(var, Tag) \
    (c_tag_index(var) == Tag)

#endif // STC_VARIANT_H_INCLUDED
