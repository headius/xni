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

#include <iostream>
#include <vector>
#include <iterator>
#include <ruby.h>

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

#include <tr1/memory>

#include "xni.h"
#include "method_stub.h"
#include "layout.h"

using namespace xni;

static int 
align(int offset, int align) 
{
    return align + ((offset - 1) & ~(align - 1));
}

Layout::Layout(const std::vector<std::pair<string, shared_ptr<Type> > >& fields)
{
    int offset = 0, alignment = 0;
    
    for (std::vector<std::pair<string, shared_ptr<Type> > >::const_iterator it = fields.begin(); it != fields.end(); ++it) {
        shared_ptr<Type> type = it->second;
        offset = align(offset, type->alignment());
                
        m_getters.insert(std::pair<string, shared_ptr<Method> >(it->first, getter_for(type, offset)));
        m_setters.insert(std::pair<string, shared_ptr<Method> >(it->first, setter_for(type, offset)));
        m_fields.push_back(shared_ptr<Field>(new Field(it->first, type)));
        
        offset += type->size();
        alignment = std::max(alignment, type->alignment());
    }

    m_size = align(offset, alignment);
    m_alignment = alignment;
}


shared_ptr<Method> 
Layout::getter(const string& name)
{
    map<string, shared_ptr<Method> >::iterator it = m_getters.find(name);
    return it != m_getters.end() ? it->second : shared_ptr<Method>();
}

shared_ptr<Method>
Layout::setter(const string& name)
{
    map<string, shared_ptr<Method> >::iterator it = m_setters.find(name);
    return it != m_setters.end() ? it->second : shared_ptr<Method>();
}
