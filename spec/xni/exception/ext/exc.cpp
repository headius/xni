#include "exc.h"

XNI_EXPORT void 
exc_throw_xni_runtime_error(RubyEnv *rb, const char* message)
{
    throw xni::runtime_error(message);
}

XNI_EXPORT void 
exc_throw_xni_arg_error(RubyEnv *rb, const char* message)
{
    throw xni::arg_error(message);
}

XNI_EXPORT void 
exc_throw_std_runtime_error(RubyEnv *rb, const char* message)
{
    throw std::runtime_error(message);
}

XNI_EXPORT void 
exc_throw_std_invalid_argument(RubyEnv *rb, const char* message)
{
    throw std::invalid_argument(message);
}

XNI_EXPORT void 
exc_throw_std_out_of_range(RubyEnv *rb, const char* message)
{
    throw std::out_of_range(message);
}
