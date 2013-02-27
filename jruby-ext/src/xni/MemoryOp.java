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

import org.jruby.Ruby;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.IRubyObject;

import java.nio.ByteOrder;

/**
 * Defines memory operations for a primitive type
 */
abstract class MemoryOp {
    public static final MemoryOp BOOL = new BooleanOp();
    public static final MemoryOp INT8 = new Signed8();
    public static final MemoryOp UINT8 = new Unsigned8();
    public static final MemoryOp INT16 = new Signed16();
    public static final MemoryOp UINT16 = new Unsigned16();
    public static final MemoryOp INT32 = new Signed32();
    public static final MemoryOp UINT32 = new Unsigned32();
    public static final MemoryOp INT64 = new Signed64();
    public static final MemoryOp UINT64 = new Unsigned64();
    public static final MemoryOp FLOAT32 = new Float32();
    public static final MemoryOp FLOAT64 = new Float64();
    public static final MemoryOp INT16SWAP = new Signed16Swapped();
    public static final MemoryOp UINT16SWAP = new Unsigned16Swapped();
    public static final MemoryOp INT32SWAP = new Signed32Swapped();
    public static final MemoryOp UINT32SWAP = new Unsigned32Swapped();
    public static final MemoryOp INT64SWAP = new Signed64Swapped();
    public static final MemoryOp UINT64SWAP = new Unsigned64Swapped();
    public static final MemoryOp POINTER = new PointerOp();

    public static MemoryOp getMemoryOp(NativeType type) {
        return getMemoryOp(type, ByteOrder.nativeOrder());
    }
    
    public static MemoryOp getMemoryOp(NativeType type, ByteOrder order) {
        switch (type) {
            case BOOL:
                return BOOL;
            case SCHAR:
                return INT8;
            case UCHAR:
                return UINT8;
            case SSHORT:
                return order.equals(ByteOrder.nativeOrder()) ? INT16 : INT16SWAP;
            case USHORT:
                return order.equals(ByteOrder.nativeOrder()) ? UINT16 : UINT16SWAP;
            case SINT:
                return order.equals(ByteOrder.nativeOrder()) ? INT32 : INT32SWAP;
            case UINT:
                return order.equals(ByteOrder.nativeOrder()) ? UINT32 : UINT32SWAP;
            case SLONG_LONG:
                return order.equals(ByteOrder.nativeOrder()) ? INT64 : INT64SWAP;
            case ULONG_LONG:
                return order.equals(ByteOrder.nativeOrder()) ? UINT64 : UINT64SWAP;
            case FLOAT:
                return FLOAT32;
            case DOUBLE:
                return FLOAT64;
            case SLONG:
                return jnr.ffi.Runtime.getSystemRuntime().longSize() == 4
                        ? getMemoryOp(NativeType.SINT, order) : getMemoryOp(NativeType.SLONG_LONG, order);
            case ULONG:
                return jnr.ffi.Runtime.getSystemRuntime().longSize() == 4
                        ? getMemoryOp(NativeType.UINT, order) : getMemoryOp(NativeType.ULONG_LONG, order);
            case ADDRESS:
                return POINTER;
            default:
                return null;
        }
    }


    public static MemoryOp getMemoryOp(Type type) {
        return getMemoryOp(type, ByteOrder.nativeOrder());
    }

    public static MemoryOp getMemoryOp(Type type, ByteOrder order) {
        if (type instanceof Type.Builtin) {
            return getMemoryOp(type.getNativeType(), order);

//        } else if (type instanceof MappedType) {
//            return new Mapped(getMemoryOp(((MappedType) type).getRealType(), order), (MappedType) type);
        }

        return null;
    }
    
    abstract IRubyObject get(ThreadContext context, jnr.ffi.Pointer pointer, long offset);
    abstract void put(ThreadContext context, jnr.ffi.Pointer pointer, long offset, IRubyObject value);
    
    
    static abstract class PrimitiveOp extends MemoryOp {
        abstract IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset);
        abstract void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value);
    
        IRubyObject get(ThreadContext context, jnr.ffi.Pointer memory, long offset) {
            return get(context.runtime, memory, offset);
        }
        void put(ThreadContext context, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            put(context.runtime, memory, offset, value);
        }
    }
    
    
    static final class BooleanOp extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putByte(offset, (byte) (value.isTrue() ? 1 : 0));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return runtime.newBoolean(memory.getByte(offset) != 0);
        }
    }
    

    static final class Signed8 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putByte(offset, Util.int8Value(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newSigned8(runtime, memory.getByte(offset));
        }
    }

    
    static final class Unsigned8 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putByte(offset, (byte) Util.uint8Value(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newUnsigned8(runtime, memory.getByte(offset));
        }
    }

    
    static final class Signed16 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putShort(offset, Util.int16Value(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newSigned16(runtime, memory.getShort(offset));
        }
    }
    

    static final class Signed16Swapped extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putShort(offset, Short.reverseBytes(Util.int16Value(value)));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newSigned16(runtime, Short.reverseBytes(memory.getShort(offset)));
        }
    }

    
    static final class Unsigned16 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putShort(offset, (short) Util.uint16Value(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newUnsigned16(runtime, memory.getShort(offset));
        }
    }
    
    
    static final class Unsigned16Swapped extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putShort(offset, Short.reverseBytes((short) Util.uint16Value(value)));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newUnsigned16(runtime, Short.reverseBytes(memory.getShort(offset)));
        }
    }

    
    static final class Signed32 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putInt(offset, Util.int32Value(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newSigned32(runtime, memory.getInt(offset));
        }
    }

    
    static final class Signed32Swapped extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putInt(offset, Integer.reverseBytes(Util.int32Value(value)));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newSigned32(runtime, Integer.reverseBytes(memory.getInt(offset)));
        }
    }

    
    static final class Unsigned32 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putInt(offset, (int) Util.uint32Value(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newUnsigned32(runtime, memory.getInt(offset));
        }
    }

    
    static final class Unsigned32Swapped extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putInt(offset, Integer.reverseBytes((int) Util.uint32Value(value)));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newUnsigned32(runtime, Integer.reverseBytes(memory.getInt(offset)));
        }
    }
    
    
    static final class Signed64 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putLong(offset, Util.int64Value(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newSigned64(runtime, memory.getLong(offset));
        }
    }

    
    static final class Signed64Swapped extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putLong(offset, Long.reverseBytes(Util.int64Value(value)));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newSigned64(runtime, Long.reverseBytes(memory.getLong(offset)));
        }
    }

    
    static final class Unsigned64 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putLong(offset, Util.uint64Value(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newUnsigned64(runtime, memory.getLong(offset));
        }
    }

    
    static final class Unsigned64Swapped extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putLong(offset, Long.reverseBytes(Util.uint64Value(value)));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return Util.newUnsigned64(runtime, Long.reverseBytes(memory.getLong(offset)));
        }
    }
    
    
    static final class Float32 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putFloat(offset, Util.floatValue(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return runtime.newFloat(memory.getFloat(offset));
        }
    }
    
    
    static final class Float64 extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putDouble(offset, Util.doubleValue(value));
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return runtime.newFloat(memory.getDouble(offset));
        }
    }

    
    static final class PointerOp extends PrimitiveOp {
        public final void put(Ruby runtime, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
            memory.putAddress(offset, ((Pointer) value).address());
        }

        public final IRubyObject get(Ruby runtime, jnr.ffi.Pointer memory, long offset) {
            return new Pointer(runtime, memory.getAddress(offset));
        }
    }
    
//    
//    static final class Mapped extends MemoryOp {
//        private final MemoryOp nativeOp;
//        private final MappedType mappedType;
//
//        public Mapped(MemoryOp nativeOp, MappedType mappedType) {
//            this.nativeOp = nativeOp;
//            this.mappedType = mappedType;
//        }
//
//        @Override
//        IRubyObject get(ThreadContext context, AbstractMemory ptr, long offset) {
//            return mappedType.fromNative(context, nativeOp.get(context, ptr, offset));
//        }
//
//        @Override
//        void put(ThreadContext context, AbstractMemory ptr, long offset, IRubyObject value) {
//            nativeOp.put(context, ptr, offset, mappedType.toNative(context, value));
//        }
//        
//        @Override
//        IRubyObject get(ThreadContext context, jnr.ffi.Pointer memory, long offset) {
//            return mappedType.fromNative(context, nativeOp.get(context, memory, offset));
//        }
//
//        @Override
//        void put(ThreadContext context, jnr.ffi.Pointer memory, long offset, IRubyObject value) {
//            nativeOp.put(context, memory, offset, mappedType.toNative(context, value));
//        }
//    }
}
