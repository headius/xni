#include <xni.h>

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

using namespace xni;

static void* __xni_data(RubyEnv* rb);
static void __xni_raise(RubyEnv* rb, RubyException ex, const char* fmt, va_list ap);

struct RubyInterface_ xni::ruby_functions = {
  __xni_data,
  __xni_raise,
};

static void*
__xni_data(RubyEnv* rb)
{
    return rb->ext_data();
}

void
__xni_raise(RubyEnv* rb, RubyException ex, const char* fmt, va_list ap)
{

}


#ifdef __GNUC__
# pragma GCC visibility pop
#endif
