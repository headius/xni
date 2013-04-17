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
#include "function.h"
#include "dynamic_library.h"
#include "xni.h"
#include "extension_data.h"

using namespace xni;

static size_t extension_data_memsize(const void *);

const rb_data_type_t xni::extension_data_type = {
    "XNI::ExtensionData",
    { NULL, object_free, extension_data_memsize, },
};

static ffi_cif load_cif, unload_cif;
static ffi_type* load_parameter_types[] = { &ffi_type_pointer, &ffi_type_pointer };
static ffi_type* unload_parameter_types[] = { &ffi_type_pointer, &ffi_type_pointer };

static size_t 
extension_data_memsize(const void *obj)
{
    return sizeof(ExtensionData);
}

static VALUE
extension_data_allocate(VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &extension_data_type, new ExtensionData());
}

static VALUE 
extension_load(VALUE self, VALUE load)
{
    ffi_sarg retval;
    void* vm = (void *) (uintptr_t) 0xfee1deadcafebabeLL;
    void* ext_data;
    void** ext_data_ptr = &ext_data;
    void* values[] = { &vm, &ext_data_ptr };
    
    ffi_call(&load_cif, FFI_FN(Symbol::from_value(load)->address()), &retval, values);

    if (retval < 0) {
        throw RubyException(rb_eLoadError, "%s failed with error %d", Symbol::from_value(load)->name().c_str(), (int) retval);
    }
    ExtensionData::from_value(self)->ext_data(ext_data);
}

static VALUE
extension_data_initialize(VALUE self, VALUE load, VALUE unload)
{
    if (!NIL_P(load)) {
        TRY(extension_load(self, load));
    }

    return self;
}

static VALUE
extension_data_inspect(VALUE klass)
{
    return rb_str_new_cstr("extension_data");
}


shared_ptr<ExtensionData> 
ExtensionData::from_value(VALUE v)
{
    return shared_ptr<ExtensionData>(lock(object_from_value<ExtensionData>(v, &extension_data_type)));
}

void 
xni::extension_data_init(VALUE xniModule)
{
    VALUE cExtensionData = rb_define_class_under(xniModule, "ExtensionData", rb_cObject);
    rb_define_alloc_func(cExtensionData, extension_data_allocate);
    rb_define_method(cExtensionData, "initialize", RUBY_METHOD_FUNC(extension_data_initialize), 2);
    rb_define_method(cExtensionData, "inspect", RUBY_METHOD_FUNC(extension_data_inspect), 0);
    
    if (ffi_prep_cif(&load_cif, FFI_DEFAULT_ABI, 2, &ffi_type_sint, load_parameter_types) != FFI_OK) {
        rb_raise(rb_eRuntimeError, "ffi_prep_cif failed");
    }
    
    if (ffi_prep_cif(&unload_cif, FFI_DEFAULT_ABI, 2, &ffi_type_void, unload_parameter_types) != FFI_OK) {
        rb_raise(rb_eRuntimeError, "ffi_prep_cif failed");
    }
}