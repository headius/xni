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

#include <ruby.h>

#include "carray.h"
#include "type.h"

using namespace xni;

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

template <typename T, typename ToNative, typename FromNative>
class CArrayIOImpl : public CArrayIO {
private:
    ToNative to_native;
    FromNative from_native;

public:
    CArrayIOImpl() {}
    virtual ~CArrayIOImpl() {}
    
    void copyin(void* buffer, VALUE ary) {
        T* p = (T *) buffer;
        int len = RARRAY_LEN(ary);
        VALUE* v = RARRAY_PTR(ary);
        for (int i = 0; i < len; ++i) {
            p[i] = to_native(v[i]);
        }
    }
    
    void copyout(const void* buffer, VALUE ary) {
        const T* p = (const T *) buffer;
        int len = RARRAY_LEN(ary);
        VALUE* v = RARRAY_PTR(ary);
        for (int i = 0; i < len; ++i) {
            v[i] = from_native(p[i]);
        } 
    }
};

#define CONVERTER(name, T, to_native, from_native) \
    struct name##ToNative : public std::unary_function<VALUE, T> { \
        T operator()(VALUE v) { return (T) to_native(v); } \
    }; \
    \
    struct name##FromNative : public std::unary_function<T, VALUE> { \
        VALUE operator()(T n) { return from_native(n); } \
    }; \
    static CArrayIOImpl<T, name##ToNative, name##ToNative> name##ArrayIO;

CONVERTER(Signed8, int8_t, NUM2INT, INT2NUM);
CONVERTER(Unsigned8, uint8_t, NUM2UINT, UINT2NUM);
CONVERTER(Signed16, int16_t, NUM2INT, INT2NUM);
CONVERTER(Unsigned16, uint16_t, NUM2UINT, UINT2NUM);
CONVERTER(Signed32, int32_t, NUM2INT, INT2NUM);
CONVERTER(Unsigned32, uint32_t, NUM2UINT, UINT2NUM);
CONVERTER(Signed64, int64_t, NUM2LL, LL2NUM);
CONVERTER(Unsigned64, uint64_t, NUM2ULL, ULL2NUM);
CONVERTER(Float, float, (float) NUM2DBL, DBL2NUM);
CONVERTER(Double, double, NUM2DBL, DBL2NUM);

#define IO(nt, name) case NativeType::nt: return &name##ArrayIO;

CArrayIO* 
CArrayIO::for_type(const shared_ptr<Type>& type)
{
    switch (type->native_type()) {
            IO(SCHAR, Signed8);
            IO(SSHORT, Signed16);
            IO(SINT, Signed32);
            IO(SLONG_LONG, Signed64);
            IO(UCHAR, Unsigned8);
            IO(USHORT, Unsigned16);
            IO(UINT, Unsigned32);
            IO(ULONG_LONG, Unsigned64);
            IO(FLOAT, Float);
            IO(DOUBLE, Double);
      }
    
      return NULL;
}


#ifdef __GNUC__
# pragma GCC visibility pop
#endif
