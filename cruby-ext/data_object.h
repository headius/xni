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

#ifndef XNI_DATA_OBJECT_H
#define XNI_DATA_OBJECT_H

#include <string>
#include <tr1/memory>

#include "xni.h"
#include "method_stub.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

namespace xni {
    using std::tr1::shared_ptr;
    class MemoryAllocator;
    class AutoReleasePool;
    class ClassMetaData;
    
    class DataObject : public xni::Object {
    private:
        DataObject* prev;
        DataObject* next;
        shared_ptr<ClassMetaData> m_metadata;
        void* m_memory;
    
    public:
        DataObject(void* memory, shared_ptr<ClassMetaData>& meta_data);
        virtual ~DataObject();
    
        inline void* address() {
            return m_memory;
        }
    
        inline const shared_ptr<ClassMetaData>& meta_data() const {
            return m_metadata;
        }
        
        void release();
    
        static shared_ptr<DataObject> from_value(VALUE);
        friend class xni::AutoReleasePool;
    };
    
    class MemoryAllocator {
    protected:
        shared_ptr<Function> m_finalizer;
        const int m_size;
    
    public:
        MemoryAllocator(int size_): m_size(size_) {}
        virtual ~MemoryAllocator() {}
        virtual void* allocate() = 0;
        virtual void release(void *memory) = 0;
        
        inline int memsize() const {
            return m_size;
        }
    };
    
    class DefaultAllocator : public MemoryAllocator {
    public:
        DefaultAllocator(int size_): MemoryAllocator(size_) {}
        void* allocate();
        void release(void *);
    };

};

#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_DATA_OBJECT_H */