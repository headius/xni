/*
 * Copyright (C) 2008-2013 Wayne Meissner
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

package xni;

import org.jruby.*;
import org.jruby.runtime.builtin.IRubyObject;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 *
 */
public final class Util {
    private Util() {}
    public static final byte int8Value(Object parameter) {
        return (byte) longValue(parameter);
    }

    public static final short uint8Value(Object parameter) {
        return (short) longValue(parameter);
    }

    public static final short int16Value(Object parameter) {
        return (short) longValue(parameter);
    }
    
    public static final int uint16Value(Object parameter) {
        return (int) longValue(parameter);
    }

    public static final int int32Value(Object parameter) {
        return (int) longValue(parameter);
    }

    public static final long uint32Value(Object parameter) {
        return longValue(parameter);
    }

    public static final long int64Value(Object parameter) {
        return longValue(parameter);
    }

    public static final long uint64Value(Object parameter) {
        final long value = parameter instanceof RubyBignum
                ? ((RubyBignum) parameter).getValue().longValue()
                :longValue(parameter);
        return value;
    }

    public static final float floatValue(IRubyObject parameter) {
        return (float) RubyNumeric.num2dbl(parameter);
    }

    public static final double doubleValue(IRubyObject parameter) {
        return RubyNumeric.num2dbl(parameter);
    }

    public static final double doubleValue(Object value) {
        return value instanceof RubyFloat ? ((RubyFloat) value).getDoubleValue() : RubyNumeric.num2dbl((IRubyObject) value);
    }

    /**
     * Converts characters like 'a' or 't' to an integer value
     *
     * @param parameter
     * @return
     */
    public static final long longValue(IRubyObject parameter) {
        return RubyNumeric.num2long(parameter);
    }

    public static final long longValue(Object value) {
        return value instanceof RubyFixnum ? ((RubyFixnum) value).getLongValue() : RubyNumeric.num2long((IRubyObject) value);
    }

    public static int intValue(IRubyObject obj, RubyHash enums) {
        if (obj instanceof RubyInteger) {
                return (int) ((RubyInteger) obj).getLongValue();

        } else if (obj instanceof RubySymbol) {
            IRubyObject value = enums.fastARef(obj);
            if (value.isNil()) {
                throw obj.getRuntime().newArgumentError("invalid enum value, " + obj.inspect());
            }
            return (int) longValue(value);
        } else {
            return (int) longValue(obj);
        }
    }

    public static final IRubyObject newSigned8(Ruby runtime, byte value) {
        return runtime.newFixnum(value);
    }

    public static final IRubyObject newUnsigned8(Ruby runtime, byte value) {
        return runtime.newFixnum(value < 0 ? (long)((value & 0x7FL) + 0x80L) : value);
    }

    public static final IRubyObject newSigned16(Ruby runtime, short value) {
        return runtime.newFixnum(value);
    }

    public static final IRubyObject newUnsigned16(Ruby runtime, short value) {
        return runtime.newFixnum(value < 0 ? (long)((value & 0x7FFFL) + 0x8000L) : value);
    }

    public static final IRubyObject newSigned32(Ruby runtime, int value) {
        return runtime.newFixnum(value);
    }

    public static final IRubyObject newUnsigned32(Ruby runtime, int value) {
        return runtime.newFixnum(value < 0 ? (long)((value & 0x7FFFFFFFL) + 0x80000000L) : value);
    }

    public static final IRubyObject newSigned64(Ruby runtime, long value) {
        return runtime.newFixnum(value);
    }

    private static final BigInteger UINT64_BASE = BigInteger.valueOf(Long.MAX_VALUE).add(BigInteger.ONE);
    public static final IRubyObject newUnsigned64(Ruby runtime, long value) {
        return value < 0
                    ? RubyBignum.newBignum(runtime, BigInteger.valueOf(value & 0x7fffffffffffffffL).add(UINT64_BASE))
                    : runtime.newFixnum(value);
    }

    public static final void checkBounds(Ruby runtime, long size, long off, long len) {
        if ((off | len | (off + len) | (size - (off + len))) < 0) {
            throw runtime.newIndexError("memory access offset="
                    + off + " size=" + len + " is out of bounds");
        }
    }

    public static int roundUpToPowerOfTwo(int v) {
        if (v < 1) return 1;
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;

        return v + 1;
    }
    
    public static int align(int offset, int align) {
        return align + ((offset - 1) & ~(align - 1));
    }
}
