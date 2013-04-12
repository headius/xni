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

#ifndef XNI_FUNCTION_H
#define XNI_FUNCTION_H

#include <ruby.h>
#include <vector>
#include <tr1/memory>

#include "xni.h"
#include "type.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    using std::tr1::shared_ptr;

    class Function : public xni::Object {
    private:
        void* m_address;
        shared_ptr<Type> m_result_type;
        std::vector<shared_ptr<Type> > m_parameter_types;
        VALUE m_rb_address;
        VALUE m_rb_result_type;
        VALUE m_rb_parameter_types;
        
    public:
        Function(): m_address(0), m_rb_address(Qnil), m_rb_result_type(Qnil), m_rb_parameter_types(Qnil) {}
        void initialize(VALUE address, VALUE result_type, VALUE parameter_types);
        void mark();
        
        inline void* address() {
            return m_address;
        }
        
        inline const shared_ptr<Type>& result_type() {
            return m_result_type;
        }
        
        inline const std::vector<shared_ptr<Type> >& parameter_types() {
            return m_parameter_types;
        }
        
        inline const shared_ptr<Type>& parameter_type(int idx) {
            return m_parameter_types[idx];
        }
        
        inline int parameter_count() {
            return m_parameter_types.size();
        }
        
        inline VALUE rb_result_type() {
            return m_rb_result_type;
        }
        
        inline VALUE rb_parameter_types() {
            return m_rb_parameter_types;
        }
        
        inline bool is_intrinsic() {
            return false;
        }
        
        static shared_ptr<Function> from_value(VALUE v);
    };
};

#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_FUNCTION_H */