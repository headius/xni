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
#include "pointer.h"

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

using namespace xni;

VALUE xni_cPointer;

static size_t pointer_memsize(const void *);

const rb_data_type_t xni::pointer_data_type = {
    "XNI::Pointer",
    { NULL, object_free, pointer_memsize, }, &object_data_type,
};


static size_t 
pointer_memsize(const void *obj)
{
    return sizeof(Pointer);
}

static VALUE
pointer_allocate(VALUE klass)
{
    TRY(return TypedData_Wrap_Struct(klass, &pointer_data_type, new Pointer()));
}

static VALUE
pointer_address(VALUE self)
{
    TRY(return ULL2NUM((uintptr_t) Pointer::from_value(self)->address()));
}

Pointer*
Pointer::from_value(VALUE v)
{
    TRY(return object_from_value<Pointer>(v, &pointer_data_type));
}

VALUE
Pointer::new_pointer(void* address)
{
    TRY(return TypedData_Wrap_Struct(xni_cPointer, &pointer_data_type, new Pointer(address)));
}

void 
xni::pointer_init(VALUE xniModule)
{
    xni_cPointer = rb_define_class_under(xniModule, "Pointer", rb_cObject);
    rb_define_alloc_func(xni_cPointer, pointer_allocate);
    rb_define_method(xni_cPointer, "address", RUBY_METHOD_FUNC(pointer_address), 0);
}
