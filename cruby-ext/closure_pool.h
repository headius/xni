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

#ifndef XNI_CLOSURE_POOL_H
#define XNI_CLOSURE_POOL_H

struct ClosurePool;
struct Closure;

struct Closure {
    void* info;      /* opaque handle for storing closure-instance specific data */
    void* function;  /* closure-instance specific function, called by custom trampoline */
    void* code;      /* The native trampoline code location */
    ClosurePool* pool;
    Closure* next;

public:
    inline void* code_address() {
        return code;
    }
};

class ClosurePool {
private:
    void* m_ctx;
    int m_closure_size;
    bool (*m_prep)(void* ctx, void *code, Closure* closure, char* errbuf, size_t errbufsize);
    struct Memory* m_blocks; /* Keeps track of all the allocated memory for this pool */
    Closure* m_list;

public:
    ClosurePool(int closureSize, bool (*prep)(void* ctx, void *code, Closure* closure, char* errbuf, size_t errbufsize),
            void* ctx);
    virtual ~ClosurePool();
    Closure* allocate();
    void release(Closure *);
};

void closure_pool_init(VALUE module);
void closure_free(Closure *);

#endif /* XNI_CLOSURE_POOL_H */

