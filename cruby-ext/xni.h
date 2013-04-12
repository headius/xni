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

#ifndef CRUBY_XNI_H
#define CRUBY_XNI_H

#include <ruby.h>
#include <tr1/memory>
#include <stdexcept>

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

extern VALUE xni_cType;

namespace xni {
    using std::tr1::shared_ptr;
    void data_object_init(VALUE xniModule);
    void dynamic_library_init(VALUE xniModule);
    void extension_init(VALUE xniModule);
    void extension_data_init(VALUE xniModule);
    void function_init(VALUE xniModule);
    void pointer_init(VALUE xniModule);
    void type_init(VALUE xniModule);
    void method_stub_init(VALUE module);
    void closure_pool_init(VALUE module);
    void auto_release_pool_init(VALUE module);
    
    
    class Object {
    private:
        int refcnt;

    public:
        Object(): refcnt(1) { }
        virtual ~Object() {}
        virtual void mark() {};
        inline int decref() { return --refcnt; }
      
        struct Deleter {
            void operator()(Object* obj) { if (obj->decref() == 0) delete obj; }
        };
        
        template <class T>
        struct Mark {
            void operator()(shared_ptr<T>& obj) { obj->mark(); }
        };

        template <class T> static shared_ptr<T> lock(T *obj) {
            obj->refcnt++;
            return shared_ptr<T>(obj, Deleter());        
        }
    };
    
    class RubyException : public std::exception {
    private:
        VALUE m_exc;
    
    public:
        RubyException(VALUE etype, const char* fmt, ...);
        ~RubyException() throw() {}
        
        inline VALUE exc() {
            return m_exc;
        }
        
        const char* what() throw();
    };
    
    void check_type(VALUE obj, VALUE t);
    void check_typeddata(VALUE obj, const rb_data_type_t* data_type);
    
    template <class T>
    T* object_from_value(VALUE obj, const rb_data_type_t* data_type) 
    {
        check_typeddata(obj, data_type);
        return reinterpret_cast<T *>(DATA_PTR(obj));
    }
    
    void object_mark(void *);
    void object_free(void *);
    extern const rb_data_type_t object_data_type;
    
}

#define TRY(block) do { \
    VALUE rbexc__ = Qnil; \
    try { \
        do { block; } while(0); \
    } catch (RubyException& ex) { \
        rbexc__ = ex.exc(); \
    } catch (std::exception& ex) { \
        rbexc__ = rb_exc_new2(rb_eRuntimeError, ex.what()); \
    } \
    if (unlikely(rbexc__ != Qnil)) { rb_exc_raise(rbexc__); }\
} while (0);


#ifdef __GNUC__
#  define likely(x) __builtin_expect((x), 1)
#  define unlikely(x) __builtin_expect((x), 0)
#else
#  define likely(x) (x)
#  define unlikely(x) (x)
#endif

#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* CRUBY_XNI_H */