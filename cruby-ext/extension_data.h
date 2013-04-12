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

#ifndef XNI_EXTENSION_DATA_H
#define XNI_EXTENSION_DATA_H

#include <tr1/memory>
#include "xni.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    using std::tr1::shared_ptr;
    
    class ExtensionData : public xni::Object {
    public:
        void* ext_data;
        
        ExtensionData(): ext_data(0) {}
        
        static shared_ptr<ExtensionData> from_value(VALUE);
        
        inline void* address() {
            return ext_data;
        }
    };
    
    extern const rb_data_type_t extension_data_type;
};

#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_EXTENSION_DATA_H */