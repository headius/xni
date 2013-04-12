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

#include <stdint.h>

#include "layout.h"
#include "method_stub.h"
#include "data_object.h"
#include "native_type.h"

using namespace xni;


#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

#define GETTER(name, type, from_native) \
class name##Getter : public Method { \
    const int m_offset; \
public: \
    name##Getter(int offset): m_offset(offset) {} \
    VALUE invoke(int argc, VALUE *argv, VALUE self) { \
        if (argc != 0) rb_raise(rb_eArgError, "incorrect number of arguments (%d != 0)", argc); \
        type value; \
        TRY(memcpy(&value, (caddr_t) DataObject::from_value(self)->address() + m_offset, sizeof(value))); \
        return from_native(value); \
    } \
};

#define SETTER(name, type, to_native) \
class name##Setter : public Method { \
    const int m_offset; \
public: \
    name##Setter(int offset): m_offset(offset) {} \
    VALUE invoke(int argc, VALUE *argv, VALUE self) { \
        if (argc != 1) rb_raise(rb_eArgError, "incorrect number of arguments (%d != 1)", argc); \
        type value = to_native(argv[0]); \
        TRY(memcpy((caddr_t) DataObject::from_value(self)->address() + m_offset, &value, sizeof(value))); \
        return argv[0]; \
    } \
};

#define ACCESSOR(name, type, from_native, to_native) \
    GETTER(name, type, from_native) \
    SETTER(name, type, to_native);


ACCESSOR(Signed8, int8_t, INT2NUM, NUM2INT)
ACCESSOR(Signed16, int16_t, INT2NUM, NUM2INT)
ACCESSOR(Signed32, int32_t, INT2NUM, NUM2INT)
ACCESSOR(Signed64, int64_t, LL2NUM, NUM2LL)
ACCESSOR(Unsigned8, uint8_t, UINT2NUM, NUM2UINT)
ACCESSOR(Unsigned16, uint16_t, UINT2NUM, NUM2UINT)
ACCESSOR(Unsigned32, uint32_t, UINT2NUM, NUM2UINT)
ACCESSOR(Unsigned64, uint64_t, ULL2NUM, NUM2ULL)
ACCESSOR(Float, float, DBL2NUM, (float) NUM2DBL)
ACCESSOR(Double, double, DBL2NUM, NUM2DBL)

#undef GETTER
#define GETTER(nt, gt) case NativeType::nt: return shared_ptr<Method>(new gt##Getter(offset));

shared_ptr<Method>
xni::getter_for(shared_ptr<Type>& type, int offset)
{
    switch (type->native_type()) {
        GETTER(SCHAR, Signed8);
        GETTER(SSHORT, Signed16);
        GETTER(SINT, Signed32);
        GETTER(SLONG_LONG, Signed64);
        GETTER(UCHAR, Unsigned8);
        GETTER(USHORT, Unsigned16);
        GETTER(UINT, Unsigned32);
        GETTER(ULONG_LONG, Unsigned64);
        GETTER(FLOAT, Float);
        GETTER(DOUBLE, Double);
    }

    return shared_ptr<Method>();
}

#undef SETTER
#define SETTER(nt, gt) case NativeType::nt: return shared_ptr<Method>(new gt##Setter(offset));

shared_ptr<Method>
xni::setter_for(shared_ptr<Type>& type, int offset)
{
  switch (type->native_type()) {
        SETTER(SCHAR, Signed8);
        SETTER(SSHORT, Signed16);
        SETTER(SINT, Signed32);
        SETTER(SLONG_LONG, Signed64);
        SETTER(UCHAR, Unsigned8);
        SETTER(USHORT, Unsigned16);
        SETTER(UINT, Unsigned32);
        SETTER(ULONG_LONG, Unsigned64);
        SETTER(FLOAT, Float);
        SETTER(DOUBLE, Double);
  }

  return shared_ptr<Method>();
}


#ifdef __GNUC__
# pragma GCC visibility pop
#endif