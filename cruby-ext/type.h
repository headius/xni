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

#ifndef XNI_TYPE_H
#define XNI_TYPE_H

#include <string>
#include <tr1/memory>

#include <ffi.h>

#include "xni.h"
#include "native_type.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    using std::tr1::shared_ptr;
    class CArrayIO;

    class Type : public xni::Object {
    protected:
        ffi_type* m_ffi_type;
        const int m_native_type;
        
    public:
        Type(int native_type, ffi_type* ffi_type_): m_ffi_type(ffi_type_), m_native_type(native_type) {}
        virtual ~Type();
        
        inline int size() const { 
            return ffiType()->size; 
        }
        
        inline int alignment() const { 
            return ffiType()->alignment; 
        }
        
        inline ffi_type* ffiType() const { 
            return m_ffi_type; 
        }
        
        inline const int native_type() const {
            return m_native_type;
        }
        
        static shared_ptr<Type> from_value(VALUE v);
    };
    
    class BuiltinType: public Type {
    public:
        BuiltinType(int native_type, ffi_type* ffi_type, const std::string& name_): Type(native_type, ffi_type), name(name_) {}
        virtual ~BuiltinType() {}
        const std::string name;
    };
    
    
    class CArrayType: public Type {
    private:
        shared_ptr<Type> m_component_type;
        CArrayIO* m_io;
        int m_length;
        int m_direction;
    
    public:
        CArrayType(): Type(NativeType::CARRAY, &ffi_type_pointer) {}
        virtual ~CArrayType() {}
        void initialize(shared_ptr<Type> component_type, int length, int direction);
    
        inline shared_ptr<Type> component_type() {
            return m_component_type;
        }
        
        inline int length() {
            return m_length;
        }
        
        inline int ioflags() {
            return m_direction;
        }
        
        inline CArrayIO* io() {
            return m_io;
        }
        
        inline bool is_in() { return (ioflags() & IN) != 0; }
        inline bool is_out() { return (ioflags() & OUT) != 0; }
        
        static const int IN = 0x1;
        static const int OUT = 0x2;
    };
};

#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_TYPE_H */
