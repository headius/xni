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

#include <stdexcept>

#include <ruby.h>

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

#include "xni.h"
#include "method_stub.h"
#include "layout.h"
#include "invoke.h"
#include "dynamic_library.h"
#include "auto_release_pool.h"
#include "data_object.h"

using namespace xni;
using std::tr1::shared_ptr;
using std::string;

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

VALUE xni_cDataObject, xni_cLayout, xni_cDataObjectFactory, xni_cMetaData;

class AllocatorMethod : public xni::Method {
private:
    shared_ptr<ClassMetaData> m_metadata;

public:
    AllocatorMethod(shared_ptr<ClassMetaData>& cmd): m_metadata(cmd) {}
    VALUE invoke(int, VALUE*, VALUE);
};

class DataObjectFactory : public xni::Object {
private:
    bool m_autorelease;
    shared_ptr<ClassMetaData> m_metadata;
    VALUE m_klass;
    
public:
    DataObjectFactory(VALUE klass, shared_ptr<ClassMetaData> cmd, bool autorelease): 
        m_klass(klass), m_metadata(cmd), m_autorelease(autorelease) {}
    virtual ~DataObjectFactory() {}
    VALUE new_instance(int argc, VALUE* argv);
    void mark() { rb_gc_mark(m_klass); }
}; 

class xni::ClassMetaData : public xni::Object {
private:
    std::vector<shared_ptr<Method> > m_attached_methods;
    std::vector<shared_ptr<MethodStub> > m_stubs;

public:
    shared_ptr<MemoryAllocator> allocator;
    shared_ptr<DataObjectFactory> retained, autorelease;
    shared_ptr<Layout> layout;
    shared_ptr<ExtensionData> ext_data;
    shared_ptr<Function> finalizer;

public:
    ClassMetaData() {}
    virtual ~ClassMetaData() {}
    
    void add_method(shared_ptr<Method>& m);
    void add_stub(shared_ptr<MethodStub>& stub);
    
    static shared_ptr<ClassMetaData> for_class(VALUE klass);
};

static size_t data_object_memsize(const void *);
static size_t class_data_memsize(const void *);
static size_t data_object_factory_memsize(const void *);
static size_t allocator_memsize(const void *);

static const rb_data_type_t data_object_data_type = {
    "XNI::DataObject",
    { NULL, object_free, data_object_memsize, }, &object_data_type,
};

static const rb_data_type_t class_data_data_type = {
    "XNI::DataObject::MetaData",
    { object_mark, object_free, class_data_memsize, }, &object_data_type,
};

static const rb_data_type_t data_object_factory_data_type = {
    "XNI::DataObject::Factory",
    { NULL, object_free, data_object_factory_memsize, }, &object_data_type,
};


static size_t 
data_object_memsize(const void *obj)
{
    return obj != NULL ? (sizeof(DataObject) + reinterpret_cast<const DataObject *>(obj)->meta_data()->allocator->memsize()) : 0;
}

static size_t 
data_object_factory_memsize(const void *obj)
{
    return sizeof(DataObjectFactory);
}

static size_t 
class_data_memsize(const void *obj)
{
    return sizeof(ClassMetaData);
}

static VALUE
data_object_retain(VALUE self)
{
    TRY(AutoReleasePool::active_pool()->remove(DataObject::from_value(self).get()));
    return self;
}

static VALUE
data_object_autorelease(VALUE self)
{
    TRY(AutoReleasePool::active_pool()->add(DataObject::from_value(self).get()));
    return self;
}

static void
set_allocator(VALUE self, int size)
{
    shared_ptr<ClassMetaData> cmd = ClassMetaData::for_class(self);
    
    cmd->allocator = shared_ptr<MemoryAllocator>(new DefaultAllocator(size));
    
    shared_ptr<Method> method(new AllocatorMethod(cmd));
    shared_ptr<MethodStub> stub(MethodStub::allocate(method, 0));
    cmd->add_stub(stub);
    rb_define_alloc_func(self, (VALUE (*)(VALUE)) stub->code_address());

    rb_ivar_set(self, rb_intern("@autorelease"), 
        TypedData_Wrap_Struct(xni_cDataObjectFactory, &data_object_factory_data_type, new DataObjectFactory(self, cmd, true)));

    rb_ivar_set(self, rb_intern("@retained"), 
        TypedData_Wrap_Struct(xni_cDataObjectFactory, &data_object_factory_data_type, new DataObjectFactory(self, cmd, false)));
}

static void
set_data_fields(VALUE self, int argc, const char** names, VALUE* types)
{
    shared_ptr<ClassMetaData> cmd = ClassMetaData::for_class(self);
    if (cmd->layout) {
        throw RubyException(rb_eRuntimeError, "data fields already specified");
    }
    
    std::vector<std::pair<std::string, shared_ptr<Type> > > fields;
    for (int i = 0; i < argc; ++i) {
        fields.push_back(std::pair<std::string, shared_ptr<Type> >(names[i], Type::from_value(types[i])));
    }

    cmd->layout = shared_ptr<Layout>(new Layout(fields));
    set_allocator(self, cmd->layout->size());
}

static VALUE
data_object_s_data_fields(int argc, VALUE *argv, VALUE self)
{

    const char** names = ALLOCA_N(const char *, argc / 2);
    VALUE* types = ALLOCA_N(VALUE, argc / 2);
    
    if ((argc % 2) != 0) {
        rb_raise(rb_eRuntimeError, "invalid name, field sequence");
    }
            
    for (int i = 0; i < argc; i += 2) {
        VALUE field_name = rb_obj_as_string(argv[i]);
        names[i / 2] = StringValueCStr(field_name);
        types[i / 2] = argv[i + 1];
    }

    TRY(set_data_fields(self, argc / 2, names, types));

    return Qnil;
}

static VALUE
data_object_s_set_size(VALUE self, VALUE address)
{
    TRY(
        ffi_cif cif;
        ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 0, &ffi_type_sint, NULL);
        ffi_sarg retval;
        void* argv[] = {};
    
        shared_ptr<Symbol> symbol = Symbol::from_value(address);
        ffi_call(&cif, FFI_FN(symbol->address()), &retval, argv);
        if (retval < 0) {
            throw RubyException(rb_eLoadError, "%s failed with error %d", symbol->name().c_str(), (int) retval);
        }
        
        set_allocator(self, (int) retval);
    );
    
    return Qnil;
}

static VALUE
data_accessors(int argc, VALUE *argv, VALUE self, bool read, bool write)
{
    const char** names = ALLOCA_N(const char *, argc);
    for (int i = 0; i < argc; ++i) {
        VALUE field_name = rb_obj_as_string(argv[i]);
        names[i] = StringValueCStr(field_name);
    }

    void** getters = ALLOCA_N(void *, argc);
    void** setters = ALLOCA_N(void *, argc);
    memset(getters, 0, sizeof(void *) * argc);
    memset(setters, 0, sizeof(void *) * argc);
    
    TRY(
        shared_ptr<ClassMetaData> cmd = ClassMetaData::for_class(self);
        for (int i = 0; i < argc; ++i) {
            std::string field_name = names[i];

            if (read) {
                shared_ptr<Method> getter = cmd->layout->getter(field_name);
                if (!getter) {
                    throw RubyException(rb_eRuntimeError, "invalid field, %s", field_name.c_str());
                }
                
                shared_ptr<MethodStub> stub(MethodStub::allocate(getter, -1));
                cmd->add_stub(stub);
                getters[i] = stub->code_address();
            }
    
            if (write) {
                shared_ptr<Method> setter = cmd->layout->setter(field_name);                
                if (!setter) {
                    throw RubyException(rb_eRuntimeError, "invalid field, %s", field_name.c_str());
                }
                
                shared_ptr<MethodStub> stub(MethodStub::allocate(setter, -1));
                cmd->add_stub(stub);
                setters[i] = stub->code_address();    
            }
        }
    )
    
    for (int i = 0; i < argc; ++i) {
        if (getters[i] != NULL) {
            rb_define_method(self, names[i], RUBY_METHOD_FUNC(getters[i]), -1);
        }
        if (setters[i] != NULL) {
            char name[strlen(names[i]) + 2];
            strcpy(name, names[i]);
            strcat(name, "=");
            rb_define_method(self, name, RUBY_METHOD_FUNC(setters[i]), -1);
        }
    }
    
    return Qnil;
}

static VALUE
data_object_s_data_reader(int argc, VALUE *argv, VALUE self)
{
    return data_accessors(argc, argv, self, true, false);
}

static VALUE
data_object_s_data_accessor(int argc, VALUE *argv, VALUE self)
{
    return data_accessors(argc, argv, self, true, true);    
}

static VALUE
data_object_s_define_singleton_method(VALUE self, VALUE name, VALUE function)
{
    void* code;
    TRY(
        shared_ptr<ClassMetaData> cmd = ClassMetaData::for_class(self);
        shared_ptr<Method> m(new ModuleMethod(StringValueCStr(name), Function::from_value(function), cmd->ext_data));
        shared_ptr<MethodStub> stub(MethodStub::allocate(m, -1));
        cmd->add_stub(stub);
        code = stub->code_address()
    )
    rb_define_singleton_method(self, StringValueCStr(name), RUBY_METHOD_FUNC(code), -1);
    
    return Qnil;
}

static VALUE
data_object_s_define_method(VALUE self, VALUE name, VALUE function)
{
    void* code;
    TRY(        
        shared_ptr<ClassMetaData> cmd = ClassMetaData::for_class(self);
        shared_ptr<Method> m(new InstanceMethod(StringValueCStr(name), Function::from_value(function), cmd->ext_data));
        shared_ptr<MethodStub> stub(MethodStub::allocate(m, -1));
        cmd->add_stub(stub);
        code = stub->code_address()
    )
    
    rb_define_method(self, StringValueCStr(name), RUBY_METHOD_FUNC(code), -1);

    return Qnil;
}

static VALUE
data_object_s_finalizer(VALUE self, VALUE rb_function)
{
    TRY(
        shared_ptr<ClassMetaData> cmd = ClassMetaData::for_class(self);
        cmd->finalizer = Function::from_value(rb_function);
    )

    return Qnil;
}

static VALUE
data_object_s_allocate(VALUE self)
{
    rb_raise(rb_eRuntimeError, "no data fields declared, and no size set");
    return Qnil;
}

static VALUE
allocate_data_object(VALUE klass, shared_ptr<ClassMetaData> cmd, bool autorelease)
{
    void* memory = cmd->allocator->allocate();
    if (memory == NULL) {
        throw RubyException(rb_eRuntimeError, "failed to allocate memory for object");
    }
    
    DataObject* obj = new DataObject(memory, cmd);
    if (autorelease) {
        AutoReleasePool::active_pool()->add(obj);
    }

    return TypedData_Wrap_Struct(klass, &data_object_data_type, obj); 

}

static VALUE
data_object_factory_new(int argc, VALUE* argv, VALUE self)
{
    DataObjectFactory* factory;
    TypedData_Get_Struct(self, DataObjectFactory, &data_object_factory_data_type, factory);
    
    TRY(return factory->new_instance(argc, argv));
}

DataObject::DataObject(void* memory, shared_ptr<ClassMetaData>& meta_data): m_memory(memory), m_metadata(meta_data), prev(NULL), next(NULL)  
{
}
        
DataObject::~DataObject()
{
    release();
}

void
DataObject::release()
{
    if (m_memory != NULL) {
        if (m_metadata->finalizer) {
            void (*finalizer)(void *, void *) = (void (*)(void *, void *)) m_metadata->finalizer->address(); 
            (*finalizer)(m_metadata->ext_data->address(), m_memory);
        }

        m_metadata->allocator->release(m_memory);
        m_memory = NULL;
    }
}

shared_ptr<DataObject> 
DataObject::from_value(VALUE v)
{
    return shared_ptr<DataObject>(lock(object_from_value<DataObject>(v, &data_object_data_type)));
}


VALUE
AllocatorMethod::invoke(int argc, VALUE* argv, VALUE klass)
{
    TRY(return allocate_data_object(klass, m_metadata, false));
}

void*
DefaultAllocator::allocate()
{
    return xcalloc(1, m_size);
}

void
DefaultAllocator::release(void *memory)
{
    xfree(memory);
}

VALUE
DataObjectFactory::new_instance(int argc, VALUE* argv)
{
    VALUE obj = allocate_data_object(m_klass, m_metadata, m_autorelease);
    rb_obj_call_init(obj, argc, argv);
    return obj;
}

shared_ptr<ClassMetaData>
ClassMetaData::for_class(VALUE klass)
{
    ID id = rb_intern("__xni_class_meta_data__");
    
    VALUE base_class = klass, v = Qnil;
    
    // Search up the inheritance chain
    while (!rb_ivar_defined(base_class, id) || NIL_P((v = rb_ivar_get(base_class, id)))) {
        VALUE super = rb_class_superclass(base_class);
        if (super == xni_cDataObject) {
            break;
        }
        base_class = super;
    }

    ClassMetaData* cmd;
    if (NIL_P(v)) {
        rb_ivar_set(base_class, id, v = TypedData_Wrap_Struct(xni_cMetaData, &class_data_data_type, cmd = new ClassMetaData()));
        cmd->ext_data = ExtensionData::from_value(rb_ivar_get(rb_ivar_get(klass, rb_intern("@__xni__")), rb_intern("__xni_ext_data__")));

    } else {
                
        TypedData_Get_Struct(v, ClassMetaData, &class_data_data_type, cmd);
    }

    return lock(cmd);
}

void 
ClassMetaData::add_method(shared_ptr<Method>& m) 
{ 
    m_attached_methods.push_back(m); 
}

void 
ClassMetaData::add_stub(shared_ptr<MethodStub>& stub)
{
    m_stubs.push_back(stub);
}

void
xni::data_object_init(VALUE xniModule)
{
    xni_cDataObject = rb_define_class_under(xniModule, "DataObject", rb_cObject);
    xni_cLayout = rb_define_class_under(xniModule, "Layout", rb_cObject);
    rb_define_alloc_func(xni_cDataObject, data_object_s_allocate);

    rb_define_method(xni_cDataObject, "retain", RUBY_METHOD_FUNC(data_object_retain), 0);
    rb_define_method(xni_cDataObject, "release", RUBY_METHOD_FUNC(data_object_retain), 0);
    rb_define_method(xni_cDataObject, "autorelease", RUBY_METHOD_FUNC(data_object_autorelease), 0);
    rb_define_module_function(xni_cDataObject, "__xni_data_fields__", RUBY_METHOD_FUNC(data_object_s_data_fields), -1);
    rb_define_module_function(xni_cDataObject, "__xni_data_reader__", RUBY_METHOD_FUNC(data_object_s_data_reader), -1);
    rb_define_module_function(xni_cDataObject, "__xni_data_accessor__", RUBY_METHOD_FUNC(data_object_s_data_accessor), -1);
    rb_define_module_function(xni_cDataObject, "__xni_define_singleton_method__", RUBY_METHOD_FUNC(data_object_s_define_singleton_method), 2);
    rb_define_module_function(xni_cDataObject, "__xni_define_method__", RUBY_METHOD_FUNC(data_object_s_define_method), 2);
    rb_define_module_function(xni_cDataObject, "__xni_finalizer__", RUBY_METHOD_FUNC(data_object_s_finalizer), 1);
    rb_define_module_function(xni_cDataObject, "__xni_set_size__", RUBY_METHOD_FUNC(data_object_s_set_size), 1);
    rb_define_attr(rb_singleton_class(xni_cDataObject), "autorelease", 1, 0);
    rb_define_attr(rb_singleton_class(xni_cDataObject), "retained", 1, 0);
    
    xni_cDataObjectFactory = rb_define_class_under(xni_cDataObject, "Factory", rb_cObject);
    rb_define_method(xni_cDataObjectFactory, "new", RUBY_METHOD_FUNC(data_object_factory_new), -1);
    
    xni_cMetaData = rb_define_class_under(xni_cDataObject, "MetaData", rb_cObject);
}

#ifdef __GNUC__
# pragma GCC visibility pop
#endif
