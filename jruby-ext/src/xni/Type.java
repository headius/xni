package xni;

import com.kenai.jffi.ArrayFlags;
import org.jruby.*;
import org.jruby.anno.JRubyClass;
import org.jruby.anno.JRubyConstant;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.IRubyObject;

import java.util.Map;

/**
 *
 */
public class Type extends RubyObject {
    private static final java.util.Locale LOCALE = java.util.Locale.ENGLISH;
    
    protected final NativeType nativeType;

    /** Size of this type in bytes */
    protected final int size;

    /** Minimum alignment of this type in bytes */
    protected final int alignment;


    public static RubyClass createTypeClass(Ruby runtime, RubyModule xniModule) {
        RubyClass typeClass = xniModule.defineClassUnder("Type", runtime.getObject(),
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        typeClass.defineAnnotatedMethods(Type.class);
        typeClass.defineAnnotatedConstants(Type.class);

        RubyClass builtinClass = typeClass.defineClassUnder("Builtin", typeClass,
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        builtinClass.defineAnnotatedMethods(Builtin.class);

        defineBuiltinType(runtime, builtinClass, NativeType.SCHAR);
        defineBuiltinType(runtime, builtinClass, NativeType.UCHAR);
        defineBuiltinType(runtime, builtinClass, NativeType.SSHORT);
        defineBuiltinType(runtime, builtinClass, NativeType.USHORT);
        defineBuiltinType(runtime, builtinClass, NativeType.SINT);
        defineBuiltinType(runtime, builtinClass, NativeType.UINT);
        defineBuiltinType(runtime, builtinClass, NativeType.SLONG_LONG);
        defineBuiltinType(runtime, builtinClass, NativeType.ULONG_LONG);
        defineBuiltinType(runtime, builtinClass, NativeType.SLONG);
        defineBuiltinType(runtime, builtinClass, NativeType.ULONG);
        defineBuiltinType(runtime, builtinClass, NativeType.FLOAT);
        defineBuiltinType(runtime, builtinClass, NativeType.DOUBLE);
        defineBuiltinType(runtime, builtinClass, NativeType.BOOL);
        defineBuiltinType(runtime, builtinClass, NativeType.VOID);
        defineBuiltinType(runtime, builtinClass, NativeType.ADDRESS, "pointer");
        defineBuiltinType(runtime, builtinClass, NativeType.CSTRING);

        //
        // Add aliases for builtin types in Type::*
        //
        for (Map.Entry<String, RubyModule.ConstantEntry> c : builtinClass.getConstantMap().entrySet()) {
            if (c.getValue().value instanceof Type.Builtin) {
                typeClass.defineConstant(c.getKey(), c.getValue().value);
            }
        }

        RubyClass arrayTypeClass = typeClass.defineClassUnder("CArray", typeClass,
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        arrayTypeClass.defineAnnotatedMethods(CArray.class);
        arrayTypeClass.defineAnnotatedConstants(CArray.class);

        RubyClass dataObjectTypeClass = typeClass.defineClassUnder("DataObject", typeClass,
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        dataObjectTypeClass.defineAnnotatedMethods(Type.DataObject.class);
        dataObjectTypeClass.defineAnnotatedConstants(Type.DataObject.class);

        return typeClass;
    }

    private static final void defineBuiltinType(Ruby runtime, RubyClass builtinClass, NativeType nativeType, String... names) {
        try {
            if (names.length > 0) {
                for (String n : names) {
                    builtinClass.setConstant(n.toUpperCase(LOCALE),
                            new Builtin(runtime, builtinClass, nativeType, n.toLowerCase(LOCALE)));
                }
            } else {
                builtinClass.setConstant(nativeType.name(),
                        new Builtin(runtime, builtinClass, nativeType, nativeType.name().toLowerCase(LOCALE)));
            }
        } catch (UnsupportedOperationException ex) {
        }
    }

    /**
     * Initializes a new <tt>Type</tt> instance.
     */
    protected Type(Ruby runtime, RubyClass klass, NativeType type, int size, int alignment) {
        super(runtime, klass);
        this.nativeType = type;
        this.size = size;
        this.alignment = alignment;
    }

    /**
     * Initializes a new <tt>Type</tt> instance.
     */
    protected Type(Ruby runtime, RubyClass klass, NativeType type) {
        super(runtime, klass);
        this.nativeType = type;
        this.size = type.size();
        this.alignment = type.alignment();
    }

    /**
     * Gets the native size of this <tt>Type</tt> in bytes
     *
     * @param context The Ruby thread context.
     * @return The native size of this Type.
     */
    @JRubyMethod(name = "size")
    public IRubyObject size(ThreadContext context) {
        return context.runtime.newFixnum(size);
    }

    /**
     * Gets the native alignment of this <tt>Type</tt> in bytes
     *
     * @param context The Ruby thread context.
     * @return The native alignment of this Type.
     */
    @JRubyMethod(name = "alignment")
    public IRubyObject alignment(ThreadContext context) {
        return context.runtime.newFixnum(alignment);
    }

    /**
     * Gets the native size of this <tt>Type</tt> in bytes
     *
     * @return The native size of this Type.
     */
    public final int size() {
        return size;
    }

    /**
     * Gets the native alignment of this <tt>Type</tt> in bytes
     *
     * @return The native alignment of this Type.
     */
    public final int alignment() {
        return alignment;
    }

    /**
     * Gets the native type of this <tt>Type</tt> when passed as a parameter
     *
     * @return The native type of this Type.
     */
    public final NativeType getNativeType() {
        return nativeType;
    }
    
    @JRubyClass(name = "FFI::Type::Builtin", parent = "FFI::Type")
    public final static class Builtin extends Type {
        private final RubySymbol sym;

        /**
         * Initializes a new <tt>BuiltinType</tt> instance.
         */
        private Builtin(Ruby runtime, RubyClass klass, NativeType nativeType, String symName) {
            super(runtime, klass, nativeType, nativeType.size(), nativeType.size());
            this.sym = runtime.newSymbol(symName);
        }

        @JRubyMethod(name = "to_s")
        public final IRubyObject to_s(ThreadContext context) {
            return RubyString.newString(context.runtime,
                    String.format("#<FFI::Type::Builtin:%s size=%d alignment=%d>",
                            nativeType.name(), size, alignment));
        }

        @Override
        public final String toString() {
            return nativeType.name();
        }

        @Override
        public boolean equals(Object obj) {
            return (obj instanceof Builtin) && ((Builtin) obj).nativeType.equals(nativeType);
        }

        @Override
        public int hashCode() {
            int hash = 5;
            hash = 23 * hash + nativeType.hashCode();
            return hash;
        }

        @JRubyMethod
        public final IRubyObject to_sym(ThreadContext context) {
            return sym;
        }

        @Override
        @JRubyMethod(name = "==", required = 1)
        public IRubyObject op_equal(ThreadContext context, IRubyObject obj) {
            return context.runtime.newBoolean(this.equals(obj));
        }

        @Override
        @JRubyMethod(name = "equal?", required = 1)
        public IRubyObject equal_p(ThreadContext context, IRubyObject obj) {
            return context.runtime.newBoolean(this.equals(obj));
        }

        @JRubyMethod(name = "eql?", required = 1)
        public IRubyObject eql_p(ThreadContext context, IRubyObject obj) {
            return context.runtime.newBoolean(this.equals(obj));
        }

    }

    public final static class CArray extends Type {
        private final Type componentType;
        private final int length;
        
        @JRubyConstant
        public static final int IN = 0x1;
        
        @JRubyConstant
        public static final int OUT = 0x2;
        

        /**
         * Initializes a new <tt>Type.CArray</tt> instance.
         */
        public CArray(Ruby runtime, RubyClass klass, Type componentType, int length) {
            super(runtime, klass, NativeType.CARRAY, componentType.size() * length, componentType.alignment());
            this.componentType = componentType;
            this.length = length;
        }

        /**
         * Initializes a new <tt>Type.CArray</tt> instance.
         */
        public CArray(Ruby runtime, Type componentType, int length) {
            this(runtime, runtime.getModule("XNI").getClass("Type").getClass("CArray"), componentType, length);
        }


        public final Type getComponentType() {
            return componentType;
        }

        public final int length() {
            return length;
        }

        public final int flags() {
            return ArrayFlags.IN | ArrayFlags.OUT;
        }

        @JRubyMethod(name = "new", required = 3, meta = true)
        public static final IRubyObject newInstance(ThreadContext context, IRubyObject klass, 
                                                    IRubyObject componentType, IRubyObject length, IRubyObject flags) {
            if (!(componentType instanceof Type)) {
                throw context.runtime.newTypeError(componentType, context.getRuntime().getModule("XNI").getClass("Type"));
            }

            return new CArray(context.runtime, (RubyClass) klass, (Type) componentType, RubyNumeric.fix2int(length));
        }

        @JRubyMethod
        public final IRubyObject length(ThreadContext context) {
            return context.runtime.newFixnum(length);
        }

        @JRubyMethod
        public final IRubyObject component_type(ThreadContext context) {
            return componentType;
        }
        
        

    }

    public final static class DataObject extends Type {
        private final RubyClass objectClass;

        public DataObject(Ruby runtime, RubyClass klass, NativeType type, RubyClass objectClass) {
            super(runtime, klass, type);
            this.objectClass = objectClass;
        }

        @JRubyMethod(name = "new", required = 1, meta = true)
        public static final IRubyObject newInstance(ThreadContext context, IRubyObject klass, IRubyObject objectClass) {
            if (!(objectClass instanceof RubyClass && ((RubyClass) objectClass).isKindOfModule(context.getRuntime().getModule("XNI").getClass("DataObject")))) {
                throw context.runtime.newTypeError(objectClass, context.getRuntime().getClassClass());
            }

            return new DataObject(context.getRuntime(), (RubyClass) klass, NativeType.ADDRESS, (RubyClass) objectClass);
        }

        public RubyClass getObjectClass() {
            return objectClass;
        }
    }
}
