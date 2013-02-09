package xni;

import jnr.ffi.util.ref.FinalizableReferenceQueue;
import jnr.ffi.util.ref.FinalizableWeakReference;
import org.jruby.*;
import org.jruby.anno.JRubyMethod;
import org.jruby.ext.ffi.AbstractMemory;
import org.jruby.ext.ffi.Struct;
import org.jruby.ext.ffi.jffi.Function;
import org.jruby.internal.runtime.methods.DynamicMethod;
import org.jruby.runtime.Block;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.Visibility;
import org.jruby.runtime.builtin.IRubyObject;
import org.jruby.runtime.callsite.CachingCallSite;
import org.jruby.runtime.callsite.FunctionalCachingCallSite;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;

/**
 *
 */
public final class DataObject extends RubyObject {
    private static final int RELEASED = 0x1;
    private static final int AUTORELEASE = 0x2;
    private static final FinalizableReferenceQueue finalizerQueue = new FinalizableReferenceQueue();
    private static final ConcurrentMap<Finalizer, Boolean> finalizers = new ConcurrentHashMap<Finalizer, Boolean>();


    private final MetaData metaData;
    DataObject prev, next;
    private volatile int flags;
    private volatile Struct struct;
    private Finalizer finalizer;
    private static final AtomicReferenceFieldUpdater<DataObject, Struct> STRUCT_UPDATER
            = AtomicReferenceFieldUpdater.newUpdater(DataObject.class, Struct.class, "struct");

    private static final AtomicIntegerFieldUpdater<DataObject> FLAGS_UPDATER
            = AtomicIntegerFieldUpdater.newUpdater(DataObject.class, "flags");
    
    static final class MetaData {
        private final RubyClass structClass;
        private final CachingCallSite finalizerCallSite = new FunctionalCachingCallSite("call");
        private final IRubyObject finalizer;
        private final ExtensionData extensionData;

        private MetaData(ExtensionData extensionData, RubyClass structClass, IRubyObject finalizer) {
            this.extensionData = extensionData;
            this.structClass = structClass;
            this.finalizer = finalizer;
        }
    }
    
    DataObject(org.jruby.Ruby runtime, org.jruby.RubyClass klass, MetaData metaData) {
        super(runtime, klass);
        this.metaData = metaData;
    }
    
    private static final IRubyObject getSize(Ruby runtime, RubyClass klass) {
        IRubyObject size = klass.getInstanceVariable("@__xni_size__");
        if ((size == null || size.isNil()) && klass.getSuperClass() != null) {
            return getSize(runtime, klass.getSuperClass());
        }
        
        return size != null ? size : runtime.getNil();
    }
    
    private static IRubyObject getStructIVar(Ruby runtime, RubyClass klass) {
        IRubyObject structClass = (IRubyObject) klass.getInternalVariable("__xni_struct_class__");
        if ((structClass == null || structClass.isNil()) && klass.getSuperClass() != null) {
            return getStructIVar(runtime, klass.getSuperClass());
        }
        
        return structClass != null ? structClass : runtime.getNil();
    }
    
    private static final RubyClass getStructClass(Ruby runtime, RubyClass klass) {
        IRubyObject structClass, size;
        if ((structClass = getStructIVar(runtime, klass)).isNil() && !(size = getSize(runtime, klass)).isNil()) {
            
            // This allocates sufficient 64 bit integers to fulfill the size requirement, to ensure 8 byte alignment 
            IRubyObject[] fields = new IRubyObject[(((int) size.convertToInteger().getLongValue() + 7) / 8 * 2)];
            IRubyObject paddingType = runtime.getModule("FFI").getClass("Type").getConstant("INT64");
            for (int i = 0; i < fields.length; i += 2) {
                fields[i] = runtime.newSymbol("pad" + (i / 2));
                fields[i + 1] = paddingType;
            }
            
            structClass = initStructClass(runtime, klass, fields);
        }
        
        if (!(structClass instanceof RubyClass)) {
            throw runtime.newRuntimeError("no data layout and no sizeof function for " + klass.getName());
        }

        return (RubyClass) structClass;
    }

    static final MetaData getMetaData(Ruby runtime, RubyClass klass) {
        RubyClass structClass = getStructClass(runtime, klass);
        ExtensionData extensionData = Extension.getExtensionData(runtime, klass.getInstanceVariable("@__xni__"));
        return new MetaData((ExtensionData) extensionData, structClass,
                (IRubyObject) klass.getInternalVariable("__xni_finalizer__"));

    }

    private static final class DefaultAllocator implements ObjectAllocator {
        public final IRubyObject allocate(Ruby runtime, RubyClass klass) {
            MetaData metaData = getMetaData(runtime, klass);
            ObjectAllocator allocator = "autorelease".equals(klass.getInternalVariable("__xni_lifecycle__")) 
                    ? new AutoReleaseAllocator(metaData) : new GarbageCollectedAllocator(metaData);
            klass.setAllocator(allocator);
            
            return allocator.allocate(runtime, klass);
        }

        private static final ObjectAllocator INSTANCE = new DefaultAllocator();
    }
    
    static abstract class DataObjectAllocator implements ObjectAllocator {
        protected final MetaData metaData;

        protected DataObjectAllocator(MetaData metaData) {
            this.metaData = metaData;
        }
    }
    
    static final class AutoReleaseAllocator extends DataObjectAllocator {
        AutoReleaseAllocator(MetaData metaData) {
            super(metaData);
        }

        public final IRubyObject allocate(Ruby runtime, RubyClass klass) {
            DataObject obj = new DataObject(runtime, klass, metaData);
            obj.flags |= AUTORELEASE;
            AutoReleasePool.getActivePool(runtime).add(obj);
            return obj;
        }
    }

    static final class GarbageCollectedAllocator extends DataObjectAllocator {
        GarbageCollectedAllocator(MetaData metaData) {
            super(metaData);
        }

        public final IRubyObject allocate(Ruby runtime, RubyClass klass) {
            return new DataObject(runtime, klass, metaData);
        }
    }

    public final Struct getStruct(ThreadContext context) {
        return struct != null ? struct : allocateStruct(context);
    }
    
    public final AbstractMemory getMemory(ThreadContext context) {
        return getStruct(context).getMemory();
    }
    
    private Struct allocateStruct(ThreadContext context) {
        if ((flags & RELEASED) != 0) {
            throw context.getRuntime().newRuntimeError(inspect().asJavaString() + " has been released");
        }

        IRubyObject obj = metaData.structClass.newInstance(context, Block.NULL_BLOCK);
        if (!(obj instanceof Struct)) {
            throw context.getRuntime().newRuntimeError("structClass.newInstance() did not return instance of Struct");
        }
        
        if (STRUCT_UPDATER.compareAndSet(this, null, (Struct) obj)) {
            if (metaData.finalizer != null && !metaData.finalizer.isNil()) {
                finalizer = new Finalizer(this, finalizerQueue, struct, metaData.finalizer, metaData.finalizerCallSite);
                finalizers.put(finalizer, Boolean.TRUE);
            }
        }
        
        return struct;
    }
    
    private AbstractMemory allocateMemory(ThreadContext context) {
        return getStruct(context).getMemory();
    }

    /**
     * Registers the DataObject class in the JRuby runtime.
     * @param runtime The JRuby runtime to register the new class in.
     * @return The new class
     */
    public static RubyClass createDataObjectClass(Ruby runtime, RubyModule module) {

        RubyClass klass = runtime.defineClassUnder("DataObject", runtime.getObject(),
                DefaultAllocator.INSTANCE, module);
        
        klass.defineAnnotatedMethods(DataObject.class);
        klass.defineAnnotatedConstants(DataObject.class);
        runtime.getLoadService().require("ffi");
        klass.extend(new IRubyObject[]{ runtime.getModule("FFI").getConstant("DataConverter") });
        
        return klass;
    }

    @JRubyMethod(name = "to_native", module = true)
    public static IRubyObject to_native(ThreadContext context, IRubyObject self, IRubyObject value, IRubyObject ctx) {
        if (value instanceof DataObject) {
            return ((DataObject) value).getMemory(context);
        }

        throw context.getRuntime().newTypeError(value, context.getRuntime().getModule("XNI").getClass("DataObject"));
    }

    @JRubyMethod(name = "from_native", module = true)
    public static IRubyObject from_native(ThreadContext context, IRubyObject self, IRubyObject value, IRubyObject ctx) {
        throw context.getRuntime().newRuntimeError("cannot coerce native pointer to DataObject instance");
    }

    @JRubyMethod(name = "native_type", module = true)
    public static IRubyObject native_type(ThreadContext context, IRubyObject self) {
        return context.getRuntime().getModule("FFI").getClass("Type").getConstant("POINTER");
    }

    @JRubyMethod(name = "reference_required?", module = true)
    public static IRubyObject reference_required_p(ThreadContext context, IRubyObject self) {
        return context.getRuntime().getTrue();
    }

    private static RubyClass initStructClass(Ruby runtime, IRubyObject self, IRubyObject[] fields) {
        RubyClass superClass = runtime.getModule("FFI").getClass("Struct");
        RubyClass structClass = RubyClass.newClass(runtime, superClass, "Struct", superClass.getAllocator(), 
                (RubyClass) self, true);
        structClass.callMethod(runtime.getCurrentContext(), "layout", fields);
        self.getInternalVariables().setInternalVariable("__xni_struct_class__", structClass);
        
        return structClass;
    }
    
    @JRubyMethod(name = "__xni_data_fields__", visibility = Visibility.PRIVATE, module = true, rest=true)
    public static IRubyObject data_fields(ThreadContext context, IRubyObject self, IRubyObject[] fields) {
        Ruby runtime = context.getRuntime();
        
        if (self.getInternalVariables().hasInternalVariable("__xni_struct_class__")) {
            throw runtime.newRuntimeError("data fields already specified");
        }
        
        initStructClass(runtime, self, fields);
        
        return context.getRuntime().getNil();
    }


    private static IRubyObject data_accessors(ThreadContext context, IRubyObject self, IRubyObject[] args, boolean read, boolean write) {
        for (IRubyObject o : args) {
            if (!(o instanceof RubySymbol)) {
                throw context.getRuntime().newTypeError(o, context.getRuntime().getSymbol());
            }
            if (read) ((RubyModule) self).addMethod(o.toString(), new DataReader((RubyModule) self, o));
            if (write) ((RubyModule) self).addMethod(o.toString() + "=", new DataWriter((RubyModule) self, o));
        }
        return context.getRuntime().getNil();
    }
        
    @JRubyMethod(name = "__xni_data_reader__", visibility = Visibility.PRIVATE, module = true, rest=true)
    public static IRubyObject data_reader(ThreadContext context, IRubyObject self, IRubyObject[] args) {
        return data_accessors(context, self, args, true, false);
    }

    @JRubyMethod(name = "__xni_data_accessor__", visibility = Visibility.PRIVATE, module = true, rest=true)
    public static IRubyObject data_accessor(ThreadContext context, IRubyObject self, IRubyObject[] args) {
        return data_accessors(context, self, args, true, true);
    }


    @JRubyMethod(name = "__xni_define_method__", visibility = Visibility.PRIVATE, module = true)
    public static IRubyObject define_method(ThreadContext context, IRubyObject self, IRubyObject rbMethodName, IRubyObject rbFunction, IRubyObject parameterCount) {
        Function function = (Function) rbFunction;

        DynamicMethod nativeMethod = function.createDynamicMethod((RubyModule) self);
        nativeMethod.setName(rbMethodName.asJavaString());
        ExtensionData extData = Extension.getExtensionData(context.getRuntime(), self.getInstanceVariables().getInstanceVariable("@__xni__"));
        DataObjectMethod method = new DataObjectMethod((RubyClass) self, extData, nativeMethod);
        method.setName(rbMethodName.asJavaString());

        ((RubyClass) self).addMethod(rbMethodName.asJavaString(), method);
        return context.nil;
    }

    @JRubyMethod(name = "__xni_finalizer__", visibility = Visibility.PRIVATE, module = true)
    public static IRubyObject define_finalizer(ThreadContext context, IRubyObject self, IRubyObject rbFunction) {
        if (self.getInternalVariables().getInternalVariable("__xni_finalizer__") != null) {
            throw context.getRuntime().newRuntimeError("finalizer already set");
        }

        Function function = (Function) rbFunction;
        self.getInternalVariables().setInternalVariable("__xni_finalizer__", function);
        
        return context.getRuntime().getNil();
    }
    
    @JRubyMethod(name = { "__xni_lifecycle__" }, visibility = Visibility.PRIVATE, module = true)
    public static IRubyObject lifecycle(ThreadContext context, IRubyObject recv, IRubyObject lifecycle) {
        recv.getInternalVariables().setInternalVariable("__xni_lifecycle__", lifecycle.asJavaString());
        
        return context.getRuntime().getNil();
    }

    @JRubyMethod(name = "__xni_sizeof__", visibility = Visibility.PRIVATE, module = true)
    public static IRubyObject sizeof(ThreadContext context, IRubyObject recv, IRubyObject obj) {
        if (obj instanceof DataObject) {
            return ((DataObject) obj).getStruct(context).callMethod(context, "size");
        }
        
        throw context.getRuntime().newTypeError(obj, context.getRuntime().getModule("XNI").getClass("DataObject"));
    }

    @JRubyMethod(name = "autorelease", module = true, omit = true)
    public static IRubyObject autorelease(ThreadContext context, IRubyObject recv) {
        return getFactory(context, recv, "__xni_autorelease_factory__", DataObjectFactory.Type.AUTORELEASE);
    }

    @JRubyMethod(name = "retained", module = true, omit = true)
    public static IRubyObject retained(ThreadContext context, IRubyObject recv) {
        return getFactory(context, recv, "__xni_retained_factory__", DataObjectFactory.Type.GC_MANAGED);
    }
    
    private static DataObjectFactory getFactory(ThreadContext context, IRubyObject recv, String variableName,
                                          DataObjectFactory.Type type) {
        Object factory = recv.getInternalVariables().getInternalVariable(variableName);
        if (factory instanceof DataObjectFactory) {
            return (DataObjectFactory) factory;
        }
        
        return initFactory(context, recv, variableName, type);
    }
    
    private static DataObjectFactory initFactory(ThreadContext context, IRubyObject recv, String varName, DataObjectFactory.Type type) {
        synchronized (recv) {
            Object factory = recv.getInternalVariables().getInternalVariable(varName);
            if (factory instanceof DataObjectFactory) {
                return (DataObjectFactory) factory;
            }
            factory = new DataObjectFactory(context.getRuntime(), (RubyClass) recv, type);
            recv.getInternalVariables().setInternalVariable(varName, factory);
            
            return (DataObjectFactory) factory;
        }
    }

    //
    // Instance methods 
    //
    @JRubyMethod(name = "__xni_struct__")
    public IRubyObject __struct__(ThreadContext context) {
        return getStruct(context);
    }

    @JRubyMethod(name = "to_ptr")
    public IRubyObject to_ptr(ThreadContext context) {
        return getMemory(context);
    }

    @JRubyMethod(name = "autorelease")
    public IRubyObject autorelease(ThreadContext context) {
        int f = flags;
        if ((f & AUTORELEASE) == 0 && FLAGS_UPDATER.compareAndSet(this, f, f | AUTORELEASE)) {
            AutoReleasePool.getActivePool(context.getRuntime()).add(this);
        }

        return this;
    }
    
    @JRubyMethod(name = "retain")
    public IRubyObject retain(ThreadContext context) {
        int f = flags;
        if ((f & AUTORELEASE) != 0 && FLAGS_UPDATER.compareAndSet(this, f, f & ~AUTORELEASE)) {
            AutoReleasePool.getActivePool(context.getRuntime()).remove(this);
            next = prev = null;
        }
        return this;
    }    

    @JRubyMethod(name = "release")
    public IRubyObject release(ThreadContext context) {
        if ((flags & RELEASED) != 0) {
            throw context.getRuntime().newRuntimeError(inspect().asJavaString() + " has been released");

        }

        release();
        
        return context.getRuntime().getNil();
    }


    void release() {
        int f = flags;
        if ((f & RELEASED) == 0 && FLAGS_UPDATER.compareAndSet(this, f, f | RELEASED)) {
            struct = null;

            if (finalizer != null) {
                finalizer.finalizeReferent();
            }
        }
    }


    private static final class Finalizer extends FinalizableWeakReference<Object> {
        private final CachingCallSite finalizerCallSite;
        private final Struct struct;
        private final IRubyObject finalizer;
        private final ExtensionData extensionData;
        private final AtomicBoolean finalized = new AtomicBoolean();

        private Finalizer(DataObject dataObject, FinalizableReferenceQueue queue, Struct struct, 
                          IRubyObject finalizer, CachingCallSite finalizerCallSite) {
            super(dataObject, queue);
            this.struct = struct;
            this.finalizer = finalizer;
            this.finalizerCallSite = finalizerCallSite;
            this.extensionData = dataObject.metaData.extensionData;
        }

        @Override
        public void finalizeReferent() {
            if (!finalized.getAndSet(true)) {
                try {
                    finalizerCallSite.call(struct.getRuntime().getCurrentContext(), finalizer, finalizer,
                            extensionData.getNativeExtensionData(), struct.getMemory());
                } finally {
                    finalizers.remove(this);
                }
            }
        }
    }
}
