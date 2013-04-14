#include "data_object.h"


XNI_EXPORT fixnum 
dataobject_foobar_s_foo(RubyEnv *rb, fixnum value)
{
    return value;
}

XNI_EXPORT void 
dataobject_foobar_initialize(RubyEnv *rb, struct DataObject_FooBar *foo, fixnum foo_, fixnum bar_)
{
    foo->m_foo = foo_;
    foo->m_bar = bar_;
}

XNI_EXPORT fixnum 
dataobject_foobar_foo(RubyEnv *rb, struct DataObject_FooBar *foo)
{
    return foo->m_foo;
}

XNI_EXPORT fixnum 
dataobject_foobar_bar(RubyEnv *rb, struct DataObject_FooBar *foo)
{
    return foo->m_bar;
}

XNI_EXPORT void 
dataobject_foobar_set_foo(RubyEnv *rb, struct DataObject_FooBar *foobar, fixnum foo)
{
    foobar->m_foo = foo;
}

XNI_EXPORT void 
dataobject_foobar_set_bar(RubyEnv *rb, struct DataObject_FooBar *foobar, fixnum bar)
{
    foobar->m_bar = bar;
}

XNI_EXPORT int
dataobject_sizeof_xor(void)
{
    return 8;
}


#define XOR2_S(t, n) XNI_EXPORT t dataobject_xor_s_xor_##n##_2(RubyEnv *rb, t v1, t v2) { return v1 ^ v2; }
#define XOR3_S(t, n) XNI_EXPORT t dataobject_xor_s_xor_##n##_3(RubyEnv *rb, t v1, t v2, t v3) { return v1 ^ v2 ^ v3; }
#define XOR4_S(t, n) XNI_EXPORT t dataobject_xor_s_xor_##n##_4(RubyEnv *rb, t v1, t v2, t v3, t v4) { return v1 ^ v2 ^ v3 ^ v4; }
#define XOR5_S(t, n) XNI_EXPORT t dataobject_xor_s_xor_##n##_5(RubyEnv *rb, t v1, t v2, t v3, t v4, t v5) { return v1 ^ v2 ^ v3 ^ v4 ^ v5; }
#define XOR6_S(t, n) XNI_EXPORT t dataobject_xor_s_xor_##n##_6(RubyEnv *rb, t v1, t v2, t v3, t v4, t v5, t v6) { return v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6; }
#define XOR2(t, n) XNI_EXPORT t dataobject_xor_xor_##n##_2(RubyEnv *rb, struct DataObject_XOR* x, t v1, t v2) { return v1 ^ v2; }
#define XOR3(t, n) XNI_EXPORT t dataobject_xor_xor_##n##_3(RubyEnv *rb, struct DataObject_XOR* x, t v1, t v2, t v3) { return v1 ^ v2 ^ v3; }
#define XOR4(t, n) XNI_EXPORT t dataobject_xor_xor_##n##_4(RubyEnv *rb, struct DataObject_XOR* x, t v1, t v2, t v3, t v4) { return v1 ^ v2 ^ v3 ^ v4; }
#define XOR5(t, n) XNI_EXPORT t dataobject_xor_xor_##n##_5(RubyEnv *rb, struct DataObject_XOR* x, t v1, t v2, t v3, t v4, t v5) { return v1 ^ v2 ^ v3 ^ v4 ^ v5; }
#define XOR6(t, n) XNI_EXPORT t dataobject_xor_xor_##n##_6(RubyEnv *rb, struct DataObject_XOR* x, t v1, t v2, t v3, t v4, t v5, t v6) { return v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6; }

#define XOR(t, n) XOR2_S(t, n); XOR3_S(t, n); XOR4_S(t, n); XOR5_S(t, n); XOR6_S(t, n); \
  XOR2(t, n); XOR3(t, n); XOR4(t, n); XOR5(t, n); XOR6(t, n);

XOR(char, char);
XOR(unsigned char, uchar);
XOR(short, short);
XOR(unsigned short, ushort);
XOR(int, int);
XOR(unsigned int, uint);
XOR(long, long);
XOR(unsigned long, ulong);
XOR(long long, long_long);
XOR(unsigned long long, ulong_long);
