/*
 * Copyright (C) 2009-2010 Wayne Meissner
 *
 * This file is part of the JNR project.
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

import java.util.Collections;
import java.util.EnumMap;
import java.util.Map;

/**
 * NativeType defines the primitive types supported internally.
 *
 * Usually you will not use these types directly, and should instead use the standard
 * types such as {@link jnr.ffi.Pointer}, {@link jnr.ffi.NativeLong}, or any of the normal java
 * types such as {@code int}, {@code short}.
 *
 * All other types are defined in terms of these primitive types.
 */
public enum NativeType {
    /** Void type.  Only used for function return types. */
    VOID,

    /** Signed char.  Equivalent to a C char or signed char type.  Usually 1 byte in size. */
    SCHAR,

    /** Unsigned char.  Equivalent to a C unsigned char type.  Usually 1 byte in size */
    UCHAR,

    /** Signed short integer.  Equivalent to a C short or signed short type.  Usually 2 bytes in size. */
    SSHORT,

    /** Unsigned short integer.  Equivalent to a C unsigned short type.  Usually 2 bytes in size. */
    USHORT,

    /** Signed integer.  Equivalent to a C int or signed int type.  Usually 4 bytes in size. */
    SINT,

    /** Unsigned integer.  Equivalent to a C unsigned int type.  Usually 4 bytes in size. */
    UINT,

    /** Signed long integer.  Equivalent to a C long or signed long type.  Can be either 4 or 8 bytes in size, depending on the platform. */
    SLONG,

    /** Unsigned long integer.  Equivalent to a C unsigned long type.  Can be either 4 or 8 bytes in size, depending on the platform. */
    ULONG,

    /** Signed long long integer.  Equivalent to a C long long or signed long long type.  Usually 8 bytes in size. */
    SLONG_LONG,

    /** Unsigned long long integer.  Equivalent to a C unsigned long long type.  Usually 8 bytes in size. */
    ULONG_LONG,

    /** Single precision floating point.  Equivalent to a C float type.  Usually 4 bytes in size. */
    FLOAT,

    /** Double precision floating point.  Equivalent to a C double type.  Usually 8 bytes in size. */
    DOUBLE,

    BOOL,

    /** Native memory address.  Equivalent to a C void* or char* pointer type.  Can be either 4 or 8 bytes in size, depending on the platform. */
    ADDRESS,

    /** Native C string (const char *) type */
    CSTRING,

    /** Native C array type */
    CARRAY;
    
    private static final Map<NativeType, com.kenai.jffi.Type> typeMap;
    static {
        Map<NativeType, com.kenai.jffi.Type> m = new EnumMap<NativeType, com.kenai.jffi.Type>(NativeType.class);
        
        m.put(SCHAR, com.kenai.jffi.Type.SCHAR);
        m.put(UCHAR, com.kenai.jffi.Type.UCHAR);
        m.put(SSHORT, com.kenai.jffi.Type.SSHORT);
        m.put(USHORT, com.kenai.jffi.Type.USHORT);
        m.put(SINT, com.kenai.jffi.Type.SINT);
        m.put(UINT, com.kenai.jffi.Type.UINT);
        m.put(SLONG, com.kenai.jffi.Type.SLONG);
        m.put(ULONG, com.kenai.jffi.Type.ULONG);
        m.put(SLONG_LONG, com.kenai.jffi.Type.SINT64);
        m.put(ULONG_LONG, com.kenai.jffi.Type.UINT64);
        m.put(FLOAT, com.kenai.jffi.Type.FLOAT);
        m.put(DOUBLE, com.kenai.jffi.Type.DOUBLE);
        m.put(ADDRESS, com.kenai.jffi.Type.POINTER);
        m.put(CARRAY, com.kenai.jffi.Type.POINTER);
        m.put(BOOL, com.kenai.jffi.Type.UCHAR);
        m.put(VOID, com.kenai.jffi.Type.VOID);
        
        typeMap = Collections.unmodifiableMap(m);
    }
    
    public final com.kenai.jffi.Type jffiType() {
        return typeMap.get(this);
    }

    private static final boolean isPrimitive(NativeType type) {
        switch (type) {
            case VOID:
            case BOOL:
            case SCHAR:
            case UCHAR:
            case SSHORT:
            case USHORT:
            case SINT:
            case UINT:
            case SLONG_LONG:
            case ULONG_LONG:
            case SLONG:
            case ULONG:
            case FLOAT:
            case DOUBLE:
            case ADDRESS:
                return true;
            default:
                return false;
        }
    }
    
    public final int size() {
        return isPrimitive(this) ? jffiType().size() : 0;
    }

    public final int alignment() {
        return isPrimitive(this) ? jffiType().alignment() : 1;
    }
}
