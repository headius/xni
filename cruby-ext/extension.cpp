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

#include "xni.h"
#include "function.h"
#include "extension_data.h"
#include "method_stub.h"
#include "invoke.h"

using namespace xni;
using std::tr1::shared_ptr;

static VALUE
extension_set_data(VALUE self, VALUE data)
{
    Check_TypedStruct(data, &extension_data_type);
    rb_ivar_set(self, rb_intern("__xni_ext_data__"), data);
    return Qnil;
}

static VALUE
extension_define_method(VALUE self, VALUE name, VALUE function)
{
    VALUE ext_data = rb_ivar_get(self, rb_intern("__xni_ext_data__"));
    void* code_address = NULL;
    AttachedStub* attached_stub = NULL;
    
    TRY(
        shared_ptr<Method> m(new ModuleMethod(StringValueCStr(name), Function::from_value(function), 
            ExtensionData::from_value(ext_data)));
        
        shared_ptr<MethodStub> stub(MethodStub::allocate(m, -1));
        attached_stub = new AttachedStub(stub, m, function);
        code_address = stub->code_address();
    )
    
    rb_ivar_set(rb_singleton_class(self), rb_intern(StringValueCStr(name)), 
        TypedData_Wrap_Struct(rb_cObject, &attached_stub_data_type, attached_stub));
    
    rb_define_module_function(self, StringValueCStr(name), RUBY_METHOD_FUNC(code_address), -1);
    
    return Qnil;
}

void 
xni::extension_init(VALUE xniModule)
{
    VALUE mExtension = rb_define_module_under(xniModule, "Extension");
    rb_define_module_function(mExtension, "__xni_ext_data__", RUBY_METHOD_FUNC(extension_set_data), 1);
    rb_define_module_function(mExtension, "__xni_define_method__", RUBY_METHOD_FUNC(extension_define_method), 2);
}