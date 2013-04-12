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

#include <pthread.h>
#include <ruby.h>

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

#include "xni.h"
#include "data_object.h"
#include "auto_release_pool.h"

using namespace xni;

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

struct ThreadData {
    AutoReleasePool* active_pool;
};

static inline ThreadData* thread_data_get();
static ThreadData* thread_data_init();

static pthread_key_t thread_data_key;
static VALUE cAutoReleasePool;

static size_t auto_release_pool_memsize(const void *);

static const rb_data_type_t auto_release_pool_data_type = {
    "XNI::AutoReleasePool",
    { object_mark, object_free, auto_release_pool_memsize, }, &object_data_type,
};

AutoReleasePool::AutoReleasePool(): head(NULL), tail(NULL)
{
}

AutoReleasePool::~AutoReleasePool()
{
}

AutoReleasePool* 
AutoReleasePool::from_value(VALUE v) 
{
    return object_from_value<AutoReleasePool>(v, &auto_release_pool_data_type);
}

AutoReleasePool*
AutoReleasePool::active_pool()
{
    AutoReleasePool* pool = thread_data_get()->active_pool;
    if (!pool) {
        rb_raise(rb_eRuntimeError, "no active AutoReleasePool");
        return NULL;
    }
    
    return pool;
}

void 
AutoReleasePool::add(DataObject* obj)
{
    if (obj == head) {
        head = head->next;
    }
    if (obj == tail) {
        tail = tail->next;
    }
    if (obj->prev != NULL) {
        obj->prev->next = obj->next;
    }
    if (obj->next != NULL) {
        obj->next->prev = obj->prev;
    }
}

void
AutoReleasePool::remove(DataObject* obj)
{
    if (obj->prev != NULL || obj->next != NULL) {
        return;
    }

    obj->next = NULL;
    if (head == NULL) {
        head = obj;
    }
    
    if (tail != NULL) {
        obj->prev = tail;
        tail->next = obj;
    }
    tail = obj;
}

void
AutoReleasePool::dispose()
{
    for (DataObject* obj = head; obj != NULL; obj = obj->next) {
        obj->release();
    }
    head = tail = NULL;
}

static VALUE
auto_release_pool_allocate(VALUE klass)
{
    return TypedData_Wrap_Struct(klass, &auto_release_pool_data_type, new AutoReleasePool());
}

static VALUE
auto_release_pool_initialize(VALUE self)
{
    if (!rb_block_given_p()) {
        rb_raise(rb_eArgError, "block required");
        return Qnil;
    }
    
    ThreadData* td = thread_data_get();
    AutoReleasePool* prev_pool = td->active_pool;
    TRY(td->active_pool = AutoReleasePool::from_value(self));
    
    int state = 0;
    rb_protect(rb_yield, self, &state);
    td->active_pool->dispose();
    td->active_pool = prev_pool;
    
    if (state) {
        rb_jump_tag(state);
    }

    return Qnil;
}

static VALUE
auto_release_pool_inspect(VALUE self)
{
    char buf[100];
    
    TRY({
        AutoReleasePool* pool = AutoReleasePool::from_value(self);
        snprintf(buf, sizeof(buf), "#<%s:%p>", rb_obj_classname(self), pool);
    })

    return rb_str_new_cstr(buf);
}

static size_t 
auto_release_pool_memsize(const void *obj)
{
    return obj != NULL ? sizeof(AutoReleasePool) : 0;
}

static inline ThreadData* 
thread_data_get()
{
    ThreadData* td = (ThreadData *) pthread_getspecific(thread_data_key);
    return td != NULL ? td : thread_data_init();
}

static ThreadData*
thread_data_init()
{
    ThreadData* td = (ThreadData *) calloc(1, sizeof(ThreadData));

    pthread_setspecific(thread_data_key, td);

    return td;
}


static void
thread_data_free(void *ptr)
{
    free(ptr);
}

void 
xni::auto_release_pool_init(VALUE xniModule)
{
    pthread_key_create(&thread_data_key, thread_data_free);
    cAutoReleasePool = rb_define_class_under(xniModule, "AutoReleasePool", rb_cObject);
    
    rb_define_alloc_func(cAutoReleasePool, auto_release_pool_allocate);
    rb_define_module_function(cAutoReleasePool, "initialize", RUBY_METHOD_FUNC(auto_release_pool_initialize), 0);
    rb_define_method(cAutoReleasePool, "inspect", RUBY_METHOD_FUNC(auto_release_pool_inspect), 0);
}