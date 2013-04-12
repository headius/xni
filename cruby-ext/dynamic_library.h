/*
 * Copyright (C) 2008-2013 Wayne Meissner
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

#ifndef XNI_DYNAMIC_LIBRARY_H
#define XNI_DYNAMIC_LIBRARY_H

#include <string>
#include <tr1/memory>
#include "xni.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    using std::tr1::shared_ptr;

    class DynamicLibrary : public xni::Object {
    public:
        void* handle;
        
        static shared_ptr<DynamicLibrary> from_value(VALUE v);
    };
    
    class Symbol : public xni::Object {
    private:
        void* m_address;
        const std::string m_name;
        shared_ptr<DynamicLibrary> m_library;
    
    public:
        Symbol(void* address, const std::string& name, shared_ptr<DynamicLibrary> library): m_address(address), m_name(name), m_library(library) {}
        void* address() const {
            return m_address;
        }
        
        const std::string& name() const {
            return m_name;
        }
        
        static Symbol* from_value(VALUE v);
    };
};

#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_DYNAMIC_LIBRARY_H */