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

#ifndef XNI_LAYOUT_H
#define XNI_LAYOUT_H

#include <string>
#include <vector>
#include <map>
#include <tr1/memory>

#include "xni.h"
#include "method_stub.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    using std::tr1::shared_ptr;
    using std::string;
    using std::vector;
    using std::map;
    using std::pair;
    
    class Field {
    private:
        shared_ptr<Type> m_type;
        const string m_name;
    public:
        
        Field(const string& name, shared_ptr<Type> type): m_name(name), m_type(type) {}

        inline int size() {
            return m_type->size();
        }
        
        inline int alignment() {
            return m_type->alignment();
        }
        
        inline const string& name() const {
            return m_name;
        }
    };
    
    class Layout : public xni::Object {
    private:
        vector<shared_ptr<Field> > m_fields;
        map<string, shared_ptr<Method> > m_getters; 
        map<string, shared_ptr<Method> > m_setters;
        int m_size, m_alignment;
    
    public:
        Layout(const vector<pair<string, shared_ptr<Type> > >& fields);
        Layout(int size, int alignment): m_size(size), m_alignment(alignment) {}

        shared_ptr<Method> getter(const string& field_name);
        shared_ptr<Method> setter(const string& field_name);
        
        inline int size() {
            return m_size;
        }
        
        inline int alignment() {
            return m_alignment;
        }
    };
    
    shared_ptr<Method> getter_for(shared_ptr<Type>& type, int offset);
    shared_ptr<Method> setter_for(shared_ptr<Type>& type, int offset);
};
#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_LAYOUT_H */