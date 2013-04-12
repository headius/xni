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


#ifndef _MSC_VER
#include <sys/param.h>
#endif
#include <sys/types.h>
#ifndef _WIN32
#  include <sys/mman.h>
#endif
#include <stdio.h>
#ifndef _MSC_VER
#include <stdint.h>
#include <stdbool.h>
#else
typedef int bool;
#define true 1
#define false 0
#endif
#ifndef _WIN32
#  include <unistd.h>
#else
#  include <winsock2.h>
#  define _WINSOCKAPI_
#  include <windows.h>
#endif
#include <errno.h>
#include <ruby.h>

#if defined(_MSC_VER) && !defined(INT8_MIN)
#  include "win32/stdint.h"
#endif
#include <ffi.h>


#include "xni.h"
#include "closure_pool.h"

#ifndef roundup
#  define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))
#endif
#ifdef _WIN32
  typedef char* caddr_t;
#endif

struct Memory {
    void* code;
    void* data;
    struct Memory* next;
};

static long pageSize;

static void* allocate_page(void);
static bool free_page(void *);
static bool protect_page(void *);

ClosurePool::ClosurePool(int closureSize, bool (*prep)(void* ctx, void *code, Closure* closure, char* errbuf, size_t errbufsize),
            void* ctx)
{
    m_closure_size = closureSize;
    m_ctx = ctx;
    m_prep = prep;
    m_list = NULL;
    m_blocks = NULL;
}

ClosurePool::~ClosurePool()
{
    for (Memory* memory = m_blocks; memory != NULL; ) {
        Memory* next = memory->next;
        free_page(memory->code);
        free(memory->data);
        free(memory);
        memory = next;
    }
}

Closure*
ClosurePool::allocate()
{
    Closure *list = NULL;
    Memory* block = NULL;
    caddr_t code = NULL;
    char errmsg[256];
    int nclosures;
    long trampolineSize;
    int i;

    if (m_list != NULL) {
        Closure* closure = m_list;
        m_list = m_list->next;
    
        return closure;
    }

    trampolineSize = roundup(m_closure_size, 8);
    nclosures = (int) (pageSize / trampolineSize);
    block = (Memory *) calloc(1, sizeof(*block));
    list = (Closure *) calloc(nclosures, sizeof(*list));
    code = (caddr_t) allocate_page();
    
    if (block == NULL || list == NULL || code == NULL) {
        snprintf(errmsg, sizeof(errmsg), "failed to allocate a page. errno=%d (%s)", errno, strerror(errno));
        goto error;
    }
    
    for (i = 0; i < nclosures; ++i) {
        Closure* closure = &list[i];
        closure->next = &list[i + 1];
        closure->pool = this;        
        closure->code = (code + (i * trampolineSize));
        
        if (!(*m_prep)(m_ctx, closure->code, closure, errmsg, sizeof(errmsg))) {
            goto error;
        }
    }

    if (!protect_page(code)) {
        goto error;
    }

    /* Track the allocated page + Closure memory area */
    block->data = list;
    block->code = code;
    block->next = m_blocks;
    m_blocks = block;

    /* Thread the new block onto the free list, apart from the first one. */
    list[nclosures - 1].next = m_list;
    m_list = list->next;

    /* Use the first one as the new handle */
    return list;

error:
    free(block);
    free(list);
    if (code != NULL) {
        free_page(code);
    }
    
    return NULL;
}

void
ClosurePool::release(Closure* closure)
{
    if (closure != NULL) {
        long refcnt;
        /* Just push it on the front of the free list */
        closure->next = m_list;
        m_list = closure;
    }
}

static long
getPageSize()
{
#if defined(_WIN32) || defined(__WIN32__)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
#else
    return sysconf(_SC_PAGESIZE);
#endif
}

static void*
allocate_page(void)
{
#if defined(_WIN32) || defined(__WIN32__)
    return VirtualAlloc(NULL, pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    caddr_t page = (caddr_t) mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    return (page != (caddr_t) -1) ? page : NULL;
#endif
}

static bool
free_page(void *addr)
{
#if defined(_WIN32) || defined(__WIN32__)
    return VirtualFree(addr, 0, MEM_RELEASE);
#else
    return munmap(addr, pageSize) == 0;
#endif
}

static bool
protect_page(void* page)
{
#if defined(_WIN32) || defined(__WIN32__)
    DWORD oldProtect;
    return VirtualProtect(page, pageSize, PAGE_EXECUTE_READ, &oldProtect);
#else
    return mprotect(page, pageSize, PROT_READ | PROT_EXEC) == 0;
#endif
}

void
xni::closure_pool_init(VALUE module)
{
    pageSize = getPageSize();
}

