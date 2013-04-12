/*
 * Copyright (C) 2009-2013 Wayne Meissner
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

#ifndef XNI_METHOD_STUB_H
#define	XNI_METHOD_STUB_H

#include <ruby.h>
#include <tr1/memory>

#include "function.h"
#include "closure_pool.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    using std::tr1::shared_ptr;
    using std::string;

    class Method {
    public:
        virtual ~Method() {}
        virtual VALUE invoke(int argc, VALUE* argv, VALUE self) = 0;
    };
    
    
    class MethodStub {
    protected:
        shared_ptr<Method> m_method;
        Closure* m_closure;
    
    public:
        MethodStub(Closure* closure, shared_ptr<Method>& method): m_closure(closure), m_method(method) {}
        virtual ~MethodStub() {}
        virtual void invoke(void** args, void* retval) = 0;
        
        inline void* code_address() {
            return m_closure->code;
        }
        
        inline Closure* closure() {
            return m_closure;
        }
         
        static shared_ptr<MethodStub> allocate(shared_ptr<Method>& method, int arity);
    };
};

#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif	/* XNI_METHOD_STUB_H */

