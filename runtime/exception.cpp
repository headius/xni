#include <typeinfo>
#include <xni.h>
#include <cstdio>

#include "xni_runtime.h"

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

using namespace xni;

struct RubyException_ {
    int exc;
};

#define EXC(name) struct RubyException_ name##_exc = { name }; RubyException xni_e##name = &name##_exc;
EXC(Exception);
EXC(RuntimeError);
EXC(ArgError);
EXC(IndexError);

xni::exception::~exception() throw() 
{
}

const char* xni::exception::what() throw()
{
    return m_message.c_str();
}

void 
xni::handle_exception(ExtensionData* ed, std::exception &ex)
{
    RubyException exc = xni_eException;
    if (typeid(ex) == typeid(std::invalid_argument)) {
        exc = xni_eArgError;
        
    } else if (typeid(ex) == typeid(std::out_of_range)) {
        exc = xni_eIndexError;
    
    } else if (typeid(ex) == typeid(std::runtime_error)) {
        exc = xni_eRuntimeError;    
    
    }
    
    if (ed->vm && ed->vm->vm_raise) {
        ed->vm->vm_raise(ed->vm_data, exc, ex.what());
    }
}

void 
xni::handle_exception(ExtensionData* ed, xni::exception &ex)
{
    if (ed->vm && ed->vm->vm_raise) {
        ed->vm->vm_raise(ed->vm_data, ex.exc(), ex.message().c_str());
    }
}