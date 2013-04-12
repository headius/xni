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

#include <string>
#include <tr1/memory>
#include <ruby.h>

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif
#ifdef HAVE_RB_THREAD_CALL_WITHOUT_GVL
# include <ruby/thread.h>
#endif

#include <ffi.h>

#include "xni.h"
#include "type.h"
#include "carray.h"
#include "function.h"
#include "data_object.h"
#include "pointer.h"
#include "invoke.h"

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

using namespace xni;

static size_t attached_stub_memsize(const void *);

const rb_data_type_t xni::attached_stub_data_type = {
    "XNI::AttachedStub",
    { object_mark, object_free, attached_stub_memsize, }, &object_data_type,
};

AttachedStub::AttachedStub(shared_ptr<MethodStub>& stub, shared_ptr<Method>& method, VALUE function): 
    m_rb_function(function), m_method_stub(stub), m_method(method)
{

}
        
AttachedStub::~AttachedStub() 
{
}

void
AttachedStub::mark()
{
    rb_gc_mark(m_rb_function);
}


AttachedMethod::AttachedMethod(const string& name, shared_ptr<Function> function, shared_ptr<ExtensionData> ext_data, bool is_instance): 
    m_name(name), m_function(function), m_ext_data(ext_data)
{
    m_ffi_parameter_types.push_back(&ffi_type_pointer);
    if (is_instance) m_ffi_parameter_types.push_back(&ffi_type_pointer);
    
    for (int i = 0; i < m_function->parameter_count(); ++i) {
        m_ffi_parameter_types.push_back(m_function->parameter_type(i)->ffiType());
    }
    
    ffi_status status = ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, m_ffi_parameter_types.size(), 
            m_function->result_type()->ffiType(), &m_ffi_parameter_types[0]);
    if (status != FFI_OK) {
        throw RubyException(rb_eRuntimeError, "ffi_prep_cif failed.  status=%#x", status);
    }
}

VALUE
InstanceMethod::invoke(int argc, VALUE* argv, VALUE self)
{
    void* extra_data[] = { m_ext_data->ext_data, (void *) (uintptr_t) 0xdeadbeef };
    TRY(
        extra_data[1] = DataObject::from_value(self)->address(); 
        return xni::invoke(&m_cif, argc, argv, m_function, extra_data, 2);
    );
}

VALUE
ModuleMethod::invoke(int argc, VALUE* argv, VALUE self)
{
    void* extra_data[] = { m_ext_data->ext_data };
    TRY(return xni::invoke(&m_cif, argc, argv, m_function, extra_data, 1));
}

struct call_data {
    ffi_cif* cif;
    void* fn;
    void** values;
    void* retval;
};


template <class T>
struct ArrayDeleter {
    void operator()(T *ptr) { delete[] ptr; }
};


#ifdef HAVE_RB_THREAD_CALL_WITHOUT_GVL
  typedef void* GVL_DATA_TYPE;

#elif defined(HAVE_RB_THREAD_BLOCKING_REGION)
  typedef VALUE GVL_DATA_TYPE;

#else
# error "need either rb_thread_call_without_gvl or have_rb_thread_blocking_region" 
#endif

static GVL_DATA_TYPE 
invoke_without_gvl(void* data)
{
    call_data* cd = (call_data *) data;
    ffi_call(cd->cif, FFI_FN(cd->fn), cd->retval, cd->values);
}

VALUE
xni::invoke(ffi_cif* cif, int argc, VALUE* argv, const shared_ptr<Function>& function, void** extra_data, int extra_data_len)
{
    union {
        signed char schar;
        unsigned char uchar;
        signed short sshort;
        unsigned short ushort;
        signed int sint;
        unsigned int uint;
        signed long long slong_long;
        unsigned long long ulong_long;
        ffi_sarg sarg;
        ffi_arg arg;
        void* p;
        double d;
        float f;
    } data[argc], retval;
    
    void* values[argc + extra_data_len];
    std::vector<std::tr1::shared_ptr<char> > array_data;
    std::vector<std::pair<int, void*> > copy_out;
         
    if (unlikely(argc != function->parameter_count())) {
        throw RubyException(rb_eArgError, "wrong number of arguments (%d for %d)", argc, function->parameter_count());
    }

    for (int i = 0; i < extra_data_len; ++i) {
        values[i] = &extra_data[i];
    }

    for (int idx = 0; idx < function->parameter_count(); ++idx) {
        VALUE arg = argv[idx]; 
        
        switch (function->parameter_type(idx)->native_type()) {
            case NativeType::SCHAR:
                data[idx].schar = (signed char) NUM2INT(arg);
                break;
            
            case NativeType::UCHAR:
                data[idx].uchar = (unsigned char) NUM2UINT(arg);
                break;
                
            case NativeType::SSHORT:
                data[idx].sshort = (signed short) NUM2INT(arg);
                break;
            
            case NativeType::USHORT:
                data[idx].ushort = (unsigned short) NUM2UINT(arg);
                break;
            
            case NativeType::SINT:
                data[idx].sint = (signed int) NUM2INT(arg);
                break;
            
            case NativeType::UINT:
                data[idx].uint = (unsigned int) NUM2UINT(arg);
                break;                
                    
            case NativeType::SLONG_LONG:
                data[idx].slong_long = NUM2LL(arg);
                break;
                
            case NativeType::ULONG_LONG:
                data[idx].ulong_long = NUM2ULL(arg);
                break;
                
            case NativeType::FLOAT:
                data[idx].f = (float) NUM2DBL(arg);
                break;

            case NativeType::DOUBLE:
                data[idx].d = NUM2DBL(arg);
                break;
            
            case NativeType::BOOL:
                data[idx].uchar = RTEST(arg);
                break;

            case NativeType::POINTER:
                data[idx].p = NIL_P(arg) ? NULL : Pointer::from_value(arg)->address();
                break;
                
            case NativeType::CSTRING:
                data[idx].p = NIL_P(arg) ? NULL : StringValueCStr(arg);
                break;
            
            case NativeType::CARRAY:
                if (rb_type_p(arg, T_ARRAY)) {   
                    CArrayType* carray = dynamic_cast<CArrayType *>(function->parameter_type(idx).get());
                    char* buffer = new char[carray->component_type()->size() * RARRAY_LEN(arg)];
                    data[idx].p = buffer;
                    array_data.push_back(std::tr1::shared_ptr<char>(buffer, ArrayDeleter<char>()));
                                        
                    if (carray->is_in()) {
                        carray->io()->copyin(buffer, arg);
                    }
                    
                    if (carray->is_out()) {
                        copy_out.push_back(std::pair<int, void*>(idx, buffer));
                    }
                
                } else {
                    throw RubyException(rb_eTypeError, "parameter is not an array");
                }
                break;

            default:
                throw RubyException(rb_eRuntimeError, "unsupported parameter type, %x", function->parameter_type(idx)->native_type());
        }
        
        values[idx + extra_data_len] = &data[idx];
    }
    
    if (function->is_intrinsic()) {
        ffi_call(cif, FFI_FN(function->address()), &retval, values);
    
    } else {
        call_data cd;
        cd.cif = cif;
        cd.values = values;
        cd.retval = &retval;
        cd.fn = function->address();

#ifdef HAVE_RB_THREAD_CALL_WITHOUT_GVL
        rb_thread_call_without_gvl(invoke_without_gvl, &cd, RUBY_UBF_IO, NULL);
#else
        rb_thread_blocking_region(invoke_without_gvl, &cd, RUBY_UBF_IO, NULL);
#endif
    }
    
    if (unlikely(!copy_out.empty())) {
        for (std::vector<std::pair<int, void*> >::iterator it = copy_out.begin(); it != copy_out.end(); ++it) {
            dynamic_cast<CArrayType *>(function->parameter_type(it->first).get())->io()->copyout(it->second, argv[it->first]);
        }
    }
    
    switch (function->result_type()->native_type()) {
        case NativeType::SCHAR:
            return INT2NUM(retval.schar);
            
        case NativeType::UCHAR:
            return UINT2NUM(retval.uchar);
        
        case NativeType::SSHORT:
            return INT2NUM(retval.sshort);
            
        case NativeType::USHORT:
            return UINT2NUM(retval.ushort);
        
        case NativeType::SINT:
            return INT2NUM(retval.sint);
            
        case NativeType::UINT:
            return UINT2NUM(retval.uint);
            
        case NativeType::SLONG_LONG:
            return LL2NUM(retval.slong_long);
            
        case NativeType::ULONG_LONG:
            return ULL2NUM(retval.ulong_long);
            
        case NativeType::FLOAT:
            return rb_float_new(retval.f);
            
        case NativeType::DOUBLE:
            return rb_float_new(retval.d);
            
        case NativeType::VOID:
            return Qnil;
            
        case NativeType::BOOL:
            return retval.uchar != 0 ? Qtrue : Qfalse;
            
        case NativeType::POINTER:
            return retval.p != NULL ? Pointer::new_pointer(retval.p) : Qnil;
            
        case NativeType::CSTRING:
            return retval.p != NULL ? rb_str_new_cstr((char *) retval.p) : Qnil;            

        default:
            throw RubyException(rb_eRuntimeError, "unsupported result type, %x", function->result_type()->native_type());
        
    }
    return Qnil;
}

static size_t 
attached_stub_memsize(const void *obj)
{
    return obj != NULL ? sizeof(AttachedStub) : 0;
}