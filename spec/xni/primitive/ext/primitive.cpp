#include <iostream>

#include "primitive.h"

#define RET(t, name) \
XNI_EXPORT t \
primitive_ret_##name(RubyEnv *rb, t v) \
{ \
    return v; \
}

RET(char, char);
RET(unsigned char, uchar);
RET(short, short);
RET(unsigned short, ushort);
RET(int, int);
RET(unsigned int, uint);
RET(long, long);
RET(unsigned long, ulong);
RET(long long, long_long);
RET(unsigned long long, ulong_long);

#define XOR2(t, n) XNI_EXPORT t primitive_xor_##n##_2(RubyEnv *rb, t v1, t v2) { return v1 ^ v2; }
#define XOR3(t, n) XNI_EXPORT t primitive_xor_##n##_3(RubyEnv *rb, t v1, t v2, t v3) { return v1 ^ v2 ^ v3; }
#define XOR4(t, n) XNI_EXPORT t primitive_xor_##n##_4(RubyEnv *rb, t v1, t v2, t v3, t v4) { return v1 ^ v2 ^ v3 ^ v4; }
#define XOR5(t, n) XNI_EXPORT t primitive_xor_##n##_5(RubyEnv *rb, t v1, t v2, t v3, t v4, t v5) { return v1 ^ v2 ^ v3 ^ v4 ^ v5; }
#define XOR6(t, n) XNI_EXPORT t primitive_xor_##n##_6(RubyEnv *rb, t v1, t v2, t v3, t v4, t v5, t v6) { return v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6; }
#define XOR(t, n) XOR2(t, n); XOR3(t, n); XOR4(t, n); XOR5(t, n); XOR6(t, n);

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
