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

#include <ruby.h>
#include <ffi.h>

#include "xni.h"
#include "type.h"
#include "dynamic_library.h"
#include "function.h"

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

using namespace xni;

VALUE xni_cFunction;

static void function_mark(void *);
static void function_free(void *);
static size_t function_memsize(const void *);
static VALUE function_allocate(VALUE klass);
static VALUE function_address(VALUE klass);

static const rb_data_type_t function_data_type = {
    "XNI::Function",
    { object_mark, object_free, function_memsize, }, &object_data_type,
};

static size_t 
function_memsize(const void *obj)
{
    return sizeof(Function);
}

static VALUE
function_allocate(VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &function_data_type, new Function());
}

static VALUE
function_initialize(VALUE self, VALUE symbol_address, VALUE result_type, VALUE parameter_types)
{
    TRY(Function::from_value(self)->initialize(symbol_address, result_type, parameter_types));
    
    return self;
}


static VALUE
function_address(VALUE self)
{
    TRY(return ULL2NUM((uintptr_t) Function::from_value(self)->address()));
}

static VALUE
function_result_type(VALUE self)
{
    TRY(return Function::from_value(self)->rb_result_type());
}

static VALUE
function_parameter_types(VALUE self)
{
    TRY(return rb_obj_dup(Function::from_value(self)->rb_parameter_types()));
}



shared_ptr<Function> 
Function::from_value(VALUE v)
{
    Function* p;
    TypedData_Get_Struct(v, Function, &function_data_type, p);
    
    return Object::lock<Function>(p);
}

void 
Function::initialize(VALUE address, VALUE result_type, VALUE parameter_types)
{
    m_rb_address = address;
    m_rb_result_type = result_type;
    m_rb_parameter_types = parameter_types;
    m_address = Symbol::from_value(address)->address();
    m_result_type = Type::from_value(result_type);
    
    m_parameter_types.clear();
    
    check_type(parameter_types, rb_cArray);
    VALUE* values = RARRAY_PTR(parameter_types);
    for (int i = 0; i < RARRAY_LENINT(parameter_types); ++i) {
        m_parameter_types.push_back(Type::from_value(values[i]));
    }
}

void
Function::mark()
{
    rb_gc_mark(m_rb_address);
    rb_gc_mark(m_rb_result_type);
    rb_gc_mark(m_rb_parameter_types);
}

void 
xni::function_init(VALUE xniModule)
{
    xni_cFunction = rb_define_class_under(xniModule, "Function", rb_cObject);
    rb_define_alloc_func(xni_cFunction, function_allocate);
    rb_define_method(xni_cFunction, "initialize", RUBY_METHOD_FUNC(function_initialize), 3);
    rb_define_method(xni_cFunction, "address", RUBY_METHOD_FUNC(function_address), 0);
    rb_define_method(xni_cFunction, "result_type", RUBY_METHOD_FUNC(function_result_type), 0);
    rb_define_method(xni_cFunction, "parameter_types", RUBY_METHOD_FUNC(function_parameter_types), 0);
}