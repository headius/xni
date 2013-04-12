/*
 * Copyright (c) 2009, 2013 Wayne Meissner
 *
 * This file is part of the XNI project.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/param.h>
#include <sys/types.h>
#ifndef _WIN32
#  include <sys/mman.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#ifndef _WIN32
#  include <unistd.h>
#endif
#include <errno.h>
#include <ruby.h>

#include <ffi.h>
#include <stdexcept>

#include "closure_pool.h"
#include "method_stub.h"
#include "xni.h"

using namespace xni;

#ifndef roundup
#  define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))
#endif
#ifdef _WIN32
  typedef char* caddr_t;
#endif

static bool prep_trampoline(void* ctx, void* code, Closure* closure, char* errmsg, size_t errmsgsize);
static long trampoline_size(void);

#if defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
# define CUSTOM_TRAMPOLINE 1
#endif


#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif


class MethodStub0 : public xni::MethodStub {
public:
    MethodStub0(Closure* closure, shared_ptr<Method> method): MethodStub(closure, method) {}
    void invoke(void** args, void* retval);
};

class MethodStub1 : public xni::MethodStub {
public:
    MethodStub1(Closure* closure, shared_ptr<Method> method): MethodStub(closure, method) {}
    void invoke(void** args, void* retval);
};

class MethodStub2 : public xni::MethodStub {
public:
    MethodStub2(Closure* closure, shared_ptr<Method> method): MethodStub(closure, method) {}
    void invoke(void** args, void* retval);
};

class MethodStub3 : public xni::MethodStub {
public:
    MethodStub3(Closure* closure, shared_ptr<Method> method): MethodStub(closure, method) {}
    void invoke(void** args, void* retval);
};

class DefaultMethodStub : public xni::MethodStub {
public:
    DefaultMethodStub(Closure* closure, shared_ptr<Method> method): MethodStub(closure, method) {}
    void invoke(void** args, void* retval);
};

static ClosurePool* stub_closure_pool;


shared_ptr<MethodStub>
xni::MethodStub::allocate(shared_ptr<Method>& method, int arity)
{
    Closure* closure = stub_closure_pool->allocate();
    if (closure == NULL) {
        throw std::runtime_error("failed to allocate closure from pool");
    }

    MethodStub* stub;
    switch (arity) {
        case 0:
            stub = new MethodStub0(closure, method);
            break;
        
        case 1:
            stub = new MethodStub1(closure, method);
            break;
            
        case 2:
            stub = new MethodStub2(closure, method);
            break;
            
        case 3:
            stub = new MethodStub3(closure, method);
            break;
        
        case -1:
            stub = new DefaultMethodStub(closure, method);
            break;
        
        default:
            throw std::runtime_error("unsupported method arity");
    }
    
    closure->info = stub;
    closure->function = NULL;

    return shared_ptr<MethodStub>(stub);
}

void 
DefaultMethodStub::invoke(void** parameters, void* retval)
{
    *(VALUE *) retval = m_method->invoke(*(int *)parameters[0], *(VALUE **)parameters[1], *(VALUE *)parameters[2]);
}

void 
MethodStub0::invoke(void** parameters, void* retval)
{
    VALUE argv[] = {};
    *(VALUE *) retval = m_method->invoke(0, argv, *(VALUE *)parameters[0]);
}

void 
MethodStub1::invoke(void** parameters, void* retval)
{
    VALUE argv[] = { *(VALUE *) parameters[1] };
    *(VALUE *) retval = m_method->invoke(1, argv, *(VALUE *)parameters[0]);
}

void 
MethodStub2::invoke(void** parameters, void* retval)
{
    VALUE argv[] = { *(VALUE *) parameters[1], *(VALUE *) parameters[2] };
    *(VALUE *) retval = m_method->invoke(2, argv, *(VALUE *)parameters[0]);
}

void 
MethodStub3::invoke(void** parameters, void* retval)
{
    VALUE argv[] = { *(VALUE *) parameters[1], *(VALUE *) parameters[2], *(VALUE *) parameters[3] };
    *(VALUE *) retval = m_method->invoke(3, argv, *(VALUE *)parameters[0]);
}


#ifndef CUSTOM_TRAMPOLINE
static void attached_method_invoke(ffi_cif* cif, void* retval, void** parameters, void* user_data);

static ffi_type* stub_parameter_types[] = {
    &ffi_type_sint,
    &ffi_type_pointer,
    &ffi_type_ulong,
};

static ffi_cif stub_cif;

static bool
prep_trampoline(void* ctx, void* code, Closure* closure, char* errmsg, size_t errmsgsize)
{
    ffi_status status;

    status = ffi_prep_closure(code, &stub_cif, attached_method_invoke, closure);
    if (status != FFI_OK) {
        snprintf(errmsg, errmsgsize, "ffi_prep_closure failed.  status=%#x", status);
        return false;
    }

    return true;
}


static long
trampoline_size(void)
{
    return sizeof(ffi_closure);
}

static void
attached_method_invoke(ffi_cif* cif, void* mretval, void** parameters, void* user_data)
{
    ((MethodStub *) ((Closure *) user_data)->info)->invoke(parameters, mretval);
}

#endif



#if defined(CUSTOM_TRAMPOLINE)

static VALUE custom_trampoline(long a1, long a2, long a3, Closure*);

#define TRAMPOLINE_CTX_MAGIC (0xfee1deadcafebabe)
#define TRAMPOLINE_FUN_MAGIC (0xfeedfacebeeff00d)

/*
 * This is a hand-coded trampoline to speedup entry from ruby to the FFI translation
 * layer for x86_64 arches.
 *
 * Since a ruby function has exactly 3 arguments, and the first 6 arguments are
 * passed in registers for x86_64, we can tack on a context pointer by simply
 * putting a value in %rcx, then jumping to the C trampoline code.
 *
 * This results in approx a 30% speedup for x86_64 FFI dispatch
 */
__asm__(
    ".text\n\t"
    ".globl xni_trampoline\n\t"
    ".globl _xni_trampoline\n\t"
    "xni_trampoline:\n\t"
    "_xni_trampoline:\n\t"
    "movabsq $0xfee1deadcafebabe, %rcx\n\t"
    "movabsq $0xfeedfacebeeff00d, %r11\n\t"
    "jmpq *%r11\n\t"
    ".globl xni_trampoline_end\n\t"
    "xni_trampoline_end:\n\t"
    ".globl _xni_trampoline_end\n\t"
    "_xni_trampoline_end:\n\t"
);

static VALUE
custom_trampoline(long a1, long a2, long a3, Closure* closure)
{
    VALUE retval;
    void* parameters[] = { &a1, &a2, &a3 };
    ((MethodStub *) closure->info)->invoke(parameters, (void *) &retval);
    
    return retval;
}


extern "C" void xni_trampoline(int argc, VALUE* argv, VALUE self);
extern "C" void xni_trampoline_end(void);
static int trampoline_offsets(long *, long *);

static long trampoline_ctx_offset, trampoline_func_offset;

static long
trampoline_offset(int off, const long value)
{
    caddr_t ptr;
    for (ptr = (caddr_t) &xni_trampoline + off; ptr < (caddr_t) &xni_trampoline_end; ++ptr) {
        if (*(long *) ptr == value) {
            return ptr - (caddr_t) &xni_trampoline;
        }
    }

    return -1;
}

static int
trampoline_offsets(long* ctxOffset, long* fnOffset)
{
    *ctxOffset = trampoline_offset(0, TRAMPOLINE_CTX_MAGIC);
    if (*ctxOffset == -1) {
        return -1;
    }

    *fnOffset = trampoline_offset(0, TRAMPOLINE_FUN_MAGIC);
    if (*fnOffset == -1) {
        return -1;
    }

    return 0;
}

static bool
prep_trampoline(void* ctx, void* code, Closure* closure, char* errmsg, size_t errmsgsize)
{
    caddr_t ptr = (caddr_t) code;

    memcpy(ptr, (void *) &xni_trampoline, trampoline_size());
    /* Patch the context and function addresses into the stub code */
    *(intptr_t *)(ptr + trampoline_ctx_offset) = (intptr_t) closure;
    *(intptr_t *)(ptr + trampoline_func_offset) = (intptr_t) custom_trampoline;

    return true;
}

static long
trampoline_size(void)
{
    return (caddr_t) &xni_trampoline_end - (caddr_t) &xni_trampoline;
}

#endif /* CUSTOM_TRAMPOLINE */


void
xni::method_stub_init(VALUE module)
{
#ifndef CUSTOM_TRAMPOLINE
    ffi_status status;
#endif

    stub_closure_pool = new ClosurePool((int) trampoline_size(), prep_trampoline, NULL);

#if defined(CUSTOM_TRAMPOLINE)
    if (trampoline_offsets(&trampoline_ctx_offset, &trampoline_func_offset) != 0) {
        rb_raise(rb_eFatal, "Could not locate offsets in trampoline code");
    }
#else
    status = ffi_prep_cif(&mh_cif, FFI_DEFAULT_ABI, 3, &ffi_type_ulong,
            stub_parameter_types);
    if (status != FFI_OK) {
        rb_raise(rb_eFatal, "ffi_prep_cif failed.  status=%#x", status);
    }

#endif
}

#ifdef __GNUC__
# pragma GCC visibility pop
#endif