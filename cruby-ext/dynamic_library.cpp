/*
 * Copyright (c) 2008-2013 Wayne Meissner
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



#include <string>

#include <sys/types.h>
#include <stdio.h>
#ifndef _MSC_VER
#  include <stdint.h>
#endif
#if (defined(_WIN32) || defined(__WIN32__)) && !defined(__CYGWIN__)
# include <winsock2.h>
# define _WINSOCKAPI_
# include <windows.h>
#else
# include <dlfcn.h>
#endif
#include <ruby.h>
#if defined(_MSC_VER) && !defined(INT8_MIN)
#  include "win32/stdint.h"
#endif

#include "dynamic_library.h"
#include "xni.h"

using namespace xni;

static VALUE library_initialize(VALUE self, VALUE libname, VALUE libflags);

static VALUE symbol_allocate(VALUE klass);

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(__CYGWIN__)
static void* dl_open(const char* name, int flags);
static void dl_error(char* buf, int size);
#define dl_sym(handle, name) GetProcAddress(handle, name)
#define dl_close(handle) FreeLibrary(handle)
enum { RTLD_LAZY=1, RTLD_NOW, RTLD_GLOBAL, RTLD_LOCAL };
#else
# define dl_open(name, flags) dlopen(name, flags != 0 ? flags : RTLD_LAZY)
# define dl_error(buf, size) do { snprintf(buf, size, "%s", dlerror()); } while(0)
# define dl_sym(handle, name) dlsym(handle, name)
# define dl_close(handle) dlclose(handle)
#ifndef RTLD_LOCAL
# define RTLD_LOCAL 8
#endif
#endif

static size_t library_memsize(const void *);
static size_t symbol_memsize(const void *);

static const rb_data_type_t library_data_type = {
    "XNI::DynamicLibrary",
    { NULL, object_free, library_memsize, }, &object_data_type,
};

static const rb_data_type_t symbol_data_type = {
    "XNI::DynamicLibrary::Symbol",
    { NULL, object_free, symbol_memsize, }, &object_data_type,
};

static VALUE cDynamicLibrary, cSymbol;

shared_ptr<DynamicLibrary> 
DynamicLibrary::from_value(VALUE v)
{
    DynamicLibrary* library;
    
    TypedData_Get_Struct(v, DynamicLibrary, &library_data_type, library);
    return lock(library);
}

static VALUE
library_allocate(VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &library_data_type, new DynamicLibrary());
}

/*
 * call-seq: DynamicLibrary.open(libname, libflags)
 * @param libname (see #initialize)
 * @param libflags (see #initialize)
 * @return [FFI::DynamicLibrary]
 * @raise {LoadError} if +libname+ cannot be opened
 * Open a library.
 */
static VALUE
library_s_open(VALUE klass, VALUE libname, VALUE libflags)
{
    VALUE args[] = { libname, libflags };
    
    return rb_class_new_instance(2, args, klass);
}

/*
 * call-seq: initialize(libname, libflags)
 * @param [String] libname name of library to open
 * @param [Fixnum] libflags flags for library to open
 * @return [FFI::DynamicLibrary]
 * @raise {LoadError} if +libname+ cannot be opened
 * A new DynamicLibrary instance.
 */
static VALUE
library_initialize(VALUE self, VALUE libname, VALUE libflags)
{
    void* handle = dlopen(StringValueCStr(libname), NUM2UINT(libflags));
    if (handle == NULL) {
        char errmsg[1024];
        dl_error(errmsg, sizeof(errmsg));
        rb_raise(rb_eLoadError, "Could not open library '%s': %s", StringValueCStr(libname), errmsg);
    }
    
    rb_iv_set(self, "@name", rb_obj_dup(libname));
    
    TRY(DynamicLibrary::from_value(self)->handle = handle);
    
    return self;
}

static VALUE
library_dlsym(VALUE self, VALUE name)
{
    const char* sym_name = StringValueCStr(name);
    Symbol* sym = NULL;
    TRY(
        shared_ptr<DynamicLibrary> library = DynamicLibrary::from_value(self);
        void* address = dl_sym(library->handle, sym_name);
        sym = address != NULL ? new Symbol(address, sym_name, library) : NULL;
    );
    
    return sym != NULL ? TypedData_Wrap_Struct(cSymbol, &symbol_data_type, sym) : Qnil;
}

/*
 * call-seq: last_error
 * @return [String] library's last error string
 */
static VALUE
library_dlerror(VALUE self)
{
    char errmsg[1024];
    
    dl_error(errmsg, sizeof(errmsg));
    
    return rb_tainted_str_new2(errmsg);
}

static void
library_free(void *obj)
{
    DynamicLibrary* library = reinterpret_cast<DynamicLibrary *>(obj);
    
    if (library->handle != NULL) {
        dl_close(library->handle);
    }

    delete library;
}

static size_t 
library_memsize(const void *obj)
{
    return sizeof(DynamicLibrary);
}

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(__CYGWIN__)
static void*
dl_open(const char* name, int flags)
{
    if (name == NULL) {
        return GetModuleHandle(NULL);
    } else {
        return LoadLibraryExA(name, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    }
}

static void
dl_error(char* buf, int size)
{
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
            0, buf, size, NULL);
}
#endif

static VALUE
symbol_allocate(VALUE klass)
{
    rb_raise(rb_eRuntimeError, "cannot allocate symbol");
    return Qnil;
}


static VALUE
symbol_initialize_copy(VALUE self, VALUE other)
{
    rb_raise(rb_eRuntimeError, "cannot duplicate symbol");
    return Qnil;
}

static size_t 
symbol_memsize(const void *obj)
{
    return sizeof(Symbol);
}

static VALUE
symbol_inspect(VALUE self)
{
    char buf[256];

    Symbol* sym = Symbol::from_value(self);
    snprintf(buf, sizeof(buf), "#<FFI::Library::Symbol name=%s address=%p>",
             sym->name().c_str(), sym->address());
    return rb_str_new2(buf);
}

static VALUE
symbol_address(VALUE self)
{
    return ULL2NUM((uintptr_t) Symbol::from_value(self)->address());
}

Symbol* 
Symbol::from_value(VALUE v)
{
    Symbol* sym;

    TypedData_Get_Struct(v, Symbol, &symbol_data_type, sym);
    return sym;
}

void
xni::dynamic_library_init(VALUE xniModule)
{
    
    cDynamicLibrary = rb_define_class_under(xniModule, "DynamicLibrary", rb_cObject);
    cSymbol = rb_define_class_under(cDynamicLibrary, "Symbol", rb_cObject);
    rb_global_variable(&cDynamicLibrary);
    rb_global_variable(&cSymbol);

    rb_define_alloc_func(cDynamicLibrary, library_allocate);
    rb_define_singleton_method(cDynamicLibrary, "open", RUBY_METHOD_FUNC(library_s_open), 2);
    rb_define_singleton_method(cDynamicLibrary, "last_error", RUBY_METHOD_FUNC(library_dlerror), 0);
    rb_define_method(cDynamicLibrary, "initialize", RUBY_METHOD_FUNC(library_initialize), 2);
    rb_define_method(cDynamicLibrary, "find_function", RUBY_METHOD_FUNC(library_dlsym), 1);
    rb_define_method(cDynamicLibrary, "last_error", RUBY_METHOD_FUNC(library_dlerror), 0);
    rb_define_attr(cDynamicLibrary, "name", 1, 0);

    rb_define_alloc_func(cSymbol, symbol_allocate);
    rb_undef_method(cSymbol, "new");
    rb_define_method(cSymbol, "inspect", RUBY_METHOD_FUNC(symbol_inspect), 0);
    rb_define_method(cSymbol, "address", RUBY_METHOD_FUNC(symbol_address), 0);
    

#define DEF(x) rb_define_const(cDynamicLibrary, "RTLD_" #x, UINT2NUM(RTLD_##x))
    DEF(LAZY);
    DEF(NOW);
    DEF(GLOBAL);
    DEF(LOCAL);

}

