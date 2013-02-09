#include <stdio.h>
#include <stdlib.h>
#include <xni.h>
#include "example.h"

XNI_EXPORT void*
xni_example_load(void)
{
    void* ext_data = calloc(1024, 1);
    printf("[C] loading extension ext_data=%p\n", ext_data);
    return ext_data;
}

XNI_EXPORT void
xni_example_unload(void* data)
{
    free(data);
}

XNI_EXPORT void
xni_example_foo_initialize(RubyEnv* rb, struct Example_Foo* foo, int foo_value)
{
    foo->m_bar = 0xbabef00dLL;
    foo->m_foo = foo_value;
    printf("[C] initializing native Example_Foo(%p) ext_data=%p foo=%llx, bar=%llx\n", foo, rb->ext_data(), foo->m_foo, foo->m_bar);
}

XNI_EXPORT void
xni_example_foo_finalize(RubyEnv* rb, struct Example_Foo* foo)
{
    printf("[C] finalizing native Example_Foo(%p) foo=%llx, bar=%llx\n", foo, foo->m_foo, foo->m_bar);
}

XNI_EXPORT unsigned long long
xni_example_foo(RubyEnv* rb)
{
    return 0xfee1deadcafebabeLL;
}

XNI_EXPORT unsigned long long
xni_example_foo_foo(RubyEnv* rb, struct Example_Foo* foo)
{
    return foo->m_foo;
}

XNI_EXPORT void *
xni_example_foo_pointer(RubyEnv* rb, struct Example_Foo* foo)
{
    printf("[C] returning Example_Foo(%p)\n", foo);
    return foo;
}


struct Example_Bar {
    unsigned long long bar;
    double d[8];
    char c;
};

XNI_EXPORT int
xni_example_sizeof_bar(void)
{
    return sizeof(struct Example_Bar);
}


XNI_EXPORT void
xni_example_bar_initialize(RubyEnv* rb, struct Example_Bar* bar)
{
    bar->bar = 0xdeadbeefLL;
    printf("[C] initializing native Example_Bar(%p) ext_data=%p bar->bar=%llx\n", bar, rb->ext_data(), bar->bar);
}

XNI_EXPORT void
xni_example_bar_finalize(RubyEnv* rb, struct Example_Bar* bar)
{
    bar->bar = 0xdeadbeefLL;
    printf("[C] finalizing native Example_Bar(%p) ext_data=%p bar->bar=%llx\n", bar, rb->ext_data(), bar->bar);
}


XNI_EXPORT unsigned long long
xni_example_bar_bar(RubyEnv* rb, struct Example_Bar* bar)
{
    printf("[C] Example_Bar(%p)#bar ext_data=%p\n", bar, rb->ext_data());
    return bar->bar;
}
