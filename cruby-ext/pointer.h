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

#ifndef XNI_POINTER_H
#define XNI_POINTER_H


#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    class Pointer : public xni::Object {
    private:
        void* m_address;
    
    public:
        Pointer(): m_address(0) {}
        Pointer(void* address): m_address(address) {}
        
        static Pointer* from_value(VALUE v);
        static VALUE new_pointer(void* address);
        
        inline void* address() {
            return m_address;
        }
    };
    extern const rb_data_type_t pointer_data_type;
};


#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_POINTER_H */