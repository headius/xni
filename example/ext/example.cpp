#include <stdio.h>
#include <stdlib.h>
#include <xni.h>
#include "example.h"

XNI_EXPORT int
xni_example_load(RubyVM* vm, void **ext_data_ptr)
{
    void* ext_data = calloc(1024, 1);
    printf("[C] loading extension ext_data=%p\n", ext_data);
    *ext_data_ptr = ext_data;
    
    return XNI_OK;
}

XNI_EXPORT void
xni_example_unload(RubyVM* vm, void* data)
{
    free(data);
}

XNI_EXPORT void
example_foo_initialize(RubyEnv* rb, struct Example_Foo* foo, fixnum foo_value)
{
    foo->m_bar = 0xbabef00dLL;
    foo->m_foo = foo_value;
    printf("[C] initializing native Example_Foo(%p) ext_data=%p foo=%llx, bar=%llx\n", foo, rb->ext_data(), foo->m_foo, foo->m_bar);
}

XNI_EXPORT void
example_foo_finalize(RubyEnv* rb, struct Example_Foo* foo)
{
    printf("[C] finalizing native Example_Foo(%p) foo=%llx, bar=%llx\n", foo, foo->m_foo, foo->m_bar);
}

XNI_EXPORT fixnum
example_foo(RubyEnv* rb)
{
    fixnum retval = 0xfee1deadcafebabeLL;
    printf("[C] Example.foo ext_data=%p returning %llx\n", rb->ext_data(), retval);
    return retval;
}

XNI_EXPORT void 
example_array_of_double(RubyEnv *rb, double *ary) 
{

}

XNI_EXPORT void 
example_array_of_fixnum(RubyEnv *rb, const fixnum *ary)
{

}


XNI_EXPORT fixnum
example_foo_foo(RubyEnv* rb, struct Example_Foo* foo)
{
    return foo->m_foo;
}

XNI_EXPORT void *
example_foo_pointer(RubyEnv* rb, struct Example_Foo* foo)
{
    printf("[C] returning Example_Foo(%p)\n", foo);
    return foo;
}

XNI_EXPORT double 
example_foo_sum_array_of_double(RubyEnv *rb, struct Example_Foo *foo, const double *ary, fixnum length)
{
    double sum = 0;
    
    for (int i = 0; i < (int) length; i++) {
        sum += ary[i];
    }
    
    return sum;
}

XNI_EXPORT fixnum 
example_foo_sum_array_of_fixnum(RubyEnv *rb, struct Example_Foo *foo, const fixnum *ary, fixnum length)
{
    fixnum sum = 0;
    for (int i = 0; i < (int) length; i++) {
        sum += ary[i];
    }
    
    return sum;
}



struct Example_Bar {
    unsigned long long bar;
    double d[8];
    char c;
};

XNI_EXPORT int
example_sizeof_bar(void)
{
    return sizeof(struct Example_Bar);
}


XNI_EXPORT void
example_bar_initialize(RubyEnv* rb, struct Example_Bar* bar)
{
    bar->bar = 0xdeadbeefLL;
    printf("[C] initializing native Example_Bar(%p) ext_data=%p bar->bar=%llx\n", bar, rb->ext_data(), bar->bar);
}

XNI_EXPORT void
example_bar_finalize(RubyEnv* rb, struct Example_Bar* bar)
{
    bar->bar = 0xdeadbeefLL;
    printf("[C] finalizing native Example_Bar(%p) ext_data=%p bar->bar=%llx\n", bar, rb->ext_data(), bar->bar);
}


XNI_EXPORT fixnum
example_bar_bar(RubyEnv* rb, struct Example_Bar* bar)
{
    printf("[C] Example_Bar(%p)#bar ext_data=%p\n", bar, rb->ext_data());
    return (fixnum) bar->bar;
}
