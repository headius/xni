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

#include <stdio.h>
#include <ruby.h>

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

#include "xni.h"

using namespace xni;

static size_t object_memsize(const void *obj);

const rb_data_type_t xni::object_data_type = {
    "XNI::Object",
    { object_mark, object_free, object_memsize, },
};

void 
xni::object_mark(void *tobj)
{
    try {
        Object* obj = reinterpret_cast<Object *>(tobj);
        if (obj) {
            obj->mark();
        }
    } catch (...) {}
}

void 
xni::object_free(void *tobj)
{
    try {
        Object* obj = reinterpret_cast<Object *>(obj);
        if (obj && obj->decref() == 0) {
            delete obj;
        }
    } catch (...) {}
}

static size_t 
object_memsize(const void *obj)
{
    return sizeof(Object);
}

RubyException::RubyException(VALUE etype, const char* fmt, ...) 
{
    va_list args;
    
    va_start(args, fmt);
    VALUE mesg = rb_vsprintf(fmt, args);
    va_end(args);

    m_exc = rb_exc_new3(etype, mesg);
}

const char*
RubyException::what() throw()
{
    VALUE mesg = rb_funcall(m_exc, rb_intern("to_s"), 0, 0);
    return StringValueCStr(mesg);
}

void 
xni::check_typeddata(VALUE obj, const rb_data_type_t* data_type)
{
    if (!rb_typeddata_is_kind_of(obj, data_type)) {
        throw RubyException(rb_eTypeError, "wrong argument type %s (expected %s)", 
            rb_obj_classname(obj), data_type->wrap_struct_name);
    } 
}

void 
xni::check_type(VALUE obj, VALUE klass)
{
    if (!RTEST(rb_obj_is_kind_of(obj, klass))) {
        throw RubyException(rb_eTypeError, "wrong argument type %s (expected %s)", 
            rb_obj_classname(obj), rb_class2name(klass));
    }
}

extern "C" void 
Init_xni_cruby()
{
    VALUE xniModule = rb_define_module("XNI");
    platform_init(xniModule);
    type_init(xniModule);
    closure_pool_init(xniModule);
    method_stub_init(xniModule);    
    dynamic_library_init(xniModule);
    function_init(xniModule);
    pointer_init(xniModule);
    extension_init(xniModule);
    extension_data_init(xniModule);
    auto_release_pool_init(xniModule);
    data_object_init(xniModule);
}