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

#ifndef XNI_INVOKE_H
#define XNI_INVOKE_H

#include <ffi.h>
#include <ruby.h>
#include <string>
#include <vector>
#include <tr1/memory>

#include "function.h"
#include "extension_data.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    using std::tr1::shared_ptr;
    using std::string;
    using std::vector;

    VALUE invoke(ffi_cif*, int argc, VALUE* argv, const shared_ptr<Function>& function, void** data, int data_len);
    
    
    class AttachedMethod : public virtual xni::Object, public virtual xni::Method {
    private:
        vector<ffi_type *> m_ffi_parameter_types;
        string m_name;
    protected:
        ffi_cif m_cif;
        shared_ptr<Function> m_function;
        shared_ptr<ExtensionData> m_ext_data;
    public:
        AttachedMethod(const string& name, shared_ptr<Function> function, shared_ptr<ExtensionData> ext_data, bool is_instance);
        
        inline const string& name() {
            return m_name;
        }
    };
    
    class InstanceMethod : public AttachedMethod {
    public:
        InstanceMethod(const string& name, shared_ptr<Function> function, shared_ptr<ExtensionData> ext_data): AttachedMethod(name, function, ext_data, true) {} 
        VALUE invoke(int argc, VALUE* argv, VALUE self);
    };
    
    class ModuleMethod : public AttachedMethod {
    public:
        ModuleMethod(const string& name, shared_ptr<Function> function, shared_ptr<ExtensionData> ext_data): AttachedMethod(name, function, ext_data, false) {} 
        VALUE invoke(int argc, VALUE* argv, VALUE self);
    };
    
    class AttachedStub : public virtual xni::Object {
    private:
        VALUE m_rb_function;
        shared_ptr<MethodStub> m_method_stub;
        shared_ptr<Method> m_method;
    
    public:
        AttachedStub(shared_ptr<MethodStub>& stub, shared_ptr<Method>& method, VALUE function);
        virtual ~AttachedStub();
        void mark();
    };

    extern const rb_data_type_t attached_stub_data_type;
};


#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_INVOKE_H */