/*
 * Copyright (C) 2013 Wayne Meissner
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
#include <ruby.h>
#include <ffi.h>

#include "xni.h"
#include "carray.h"
#include "native_type.h"
#include "type.h"

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

using namespace xni;
using std::tr1::dynamic_pointer_cast;

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

VALUE xni_cType, xni_cCArrayType;

static size_t type_memsize(const void *);
static size_t builtin_type_memsize(const void *);
static size_t carray_type_memsize(const void *);

static const rb_data_type_t type_data_type = {
    "XNI::Type",
    { NULL, object_free, type_memsize, }, &object_data_type,
};

static const rb_data_type_t builtin_type_data_type = {
    "builtin_type",
    { NULL, object_free, builtin_type_memsize, }, &type_data_type,
};

static const rb_data_type_t carray_type_data_type = {
    "carray",
    { NULL, object_free, carray_type_memsize, }, &type_data_type,
};


Type::~Type()
{
}

shared_ptr<Type> 
Type::from_value(VALUE v) 
{
    return shared_ptr<Type>(lock(object_from_value<Type>(v, &type_data_type)));
}

static VALUE
type_allocate(VALUE klass)
{
    rb_raise(rb_eRuntimeError, "cannot allocate");
    return Qnil;
}


/*
 * call-seq: type.size
 * @return [Fixnum]
 * Return type's size, in bytes.
 */
static VALUE
type_size(VALUE self)
{
    TRY(return INT2FIX(Type::from_value(self)->size()));
}

/*
 * call-seq: type.alignment
 * @return [Fixnum]
 * Get Type alignment.
 */
static VALUE
type_alignment(VALUE self)
{
    TRY(return INT2FIX(Type::from_value(self)->alignment()));
}


/*
 * call-seq: type.inspect
 * @return [String]
 * Inspect {Type} object.
 */
static VALUE
type_inspect(VALUE self)
{
    char buf[100];

    TRY(
        shared_ptr<Type> type = Type::from_value(self);
    
        snprintf(buf, sizeof(buf), "#<%s:%p size=%d alignment=%d>",
                rb_obj_classname(self), type.get(), (int) type->size(), (int) type->alignment());
    );

    return rb_str_new2(buf);
}

static VALUE
builtin_type_new(VALUE klass, int nativeType, ffi_type* ffiType, const char* name)
{
    return TypedData_Wrap_Struct(klass, &builtin_type_data_type, new BuiltinType(nativeType, ffiType, name));
}

static size_t 
type_memsize(const void *obj)
{
    return obj != NULL ? sizeof(Type) : 0;
}

static size_t 
builtin_type_memsize(const void *obj)
{
    return obj != NULL ? sizeof(BuiltinType) : 0;
}

static size_t 
carray_type_memsize(const void *obj)
{
    return obj != NULL ? sizeof(CArrayType) : 0;
}

static VALUE
carray_type_allocate(VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &carray_type_data_type, new CArrayType());
}

static VALUE
carray_type_initialize(VALUE self, VALUE component_type, VALUE length, VALUE direction)
{
    dynamic_pointer_cast<CArrayType, Type>(Type::from_value(self))->initialize(Type::from_value(component_type), NUM2INT(length), NUM2INT(direction));
    
    return self;
}

void
CArrayType::initialize(shared_ptr<Type> component_type, int length, int direction)
{
    m_component_type = component_type;
    m_length = length;
    m_direction = direction;
    m_io = CArrayIO::for_type(component_type);
}

void 
xni::type_init(VALUE xniModule)
{
    xni_cType = rb_define_class_under(xniModule, "Type", rb_cObject);
    
    rb_define_alloc_func(xni_cType, type_allocate);
    rb_define_method(xni_cType, "size", RUBY_METHOD_FUNC(type_size), 0);
    rb_define_method(xni_cType, "alignment", RUBY_METHOD_FUNC(type_alignment), 0);
    rb_define_method(xni_cType, "inspect", RUBY_METHOD_FUNC(type_inspect), 0);
    
    VALUE cBuiltinType = rb_define_class_under(xni_cType, "Builtin", xni_cType);
    
    /* Define all the builtin types */
    #define T(x, ffiType) do { \
        rb_define_const(xni_cType, #x, builtin_type_new(cBuiltinType, NativeType::x, ffiType, #x)); \
    } while(0)

    T(VOID, &ffi_type_void);
    T(SCHAR, &ffi_type_schar);
    T(UCHAR, &ffi_type_uchar);
    T(SSHORT, &ffi_type_sshort);
    T(USHORT, &ffi_type_ushort);
    T(SINT, &ffi_type_sint);
    T(UINT, &ffi_type_uint);
    T(SLONG_LONG, &ffi_type_sint64);
    T(ULONG_LONG, &ffi_type_uint64);
    T(FLOAT, &ffi_type_float);
    T(DOUBLE, &ffi_type_double);
    T(POINTER, &ffi_type_pointer);
    T(BOOL, &ffi_type_uchar);
    T(CSTRING, &ffi_type_pointer);
    T(CARRAY, &ffi_type_pointer);
    
    xni_cCArrayType = rb_define_class_under(xni_cType, "CArray", xni_cType);
    rb_define_alloc_func(xni_cCArrayType, carray_type_allocate);
    rb_define_const(xni_cCArrayType, "IN", INT2FIX(CArrayType::IN));
    rb_define_const(xni_cCArrayType, "OUT", INT2FIX(CArrayType::OUT));
    rb_define_method(xni_cCArrayType, "initialize", RUBY_METHOD_FUNC(carray_type_initialize), 3);
}

#ifdef __GNUC__
# pragma GCC visibility pop
#endif
