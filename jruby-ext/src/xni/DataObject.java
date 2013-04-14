package xni;

import com.kenai.jffi.CallContext;
import com.kenai.jffi.CallingConvention;
import com.kenai.jffi.Invoker;
import jnr.ffi.util.ref.FinalizableReferenceQueue;
import jnr.ffi.util.ref.FinalizableWeakReference;
import org.jruby.*;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.Visibility;
import org.jruby.runtime.builtin.IRubyObject;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;

import static xni.Util.align;

/**
 *
 */
public final class DataObject extends RubyObject {
    private static final int RELEASED = 0x1;
    private static final int AUTORELEASE = 0x2;
    private static final FinalizableReferenceQueue finalizerQueue = new FinalizableReferenceQueue();
    private static final ConcurrentMap<Finalizer, Boolean> finalizers = new ConcurrentHashMap<Finalizer, Boolean>();


    private final MetaData metaData;
    private final jnr.ffi.Pointer memory;
    DataObject prev, next;
    private volatile int flags;
    private final Finalizer finalizer;

    private static final AtomicIntegerFieldUpdater<DataObject> FLAGS_UPDATER
            = AtomicIntegerFieldUpdater.newUpdater(DataObject.class, "flags");
    
    static final class MetaData {
        private final Function finalizer;
        private final ExtensionData extensionData;
        private final Layout layout;

        private MetaData(ExtensionData extensionData, Layout layout, Function finalizer) {
            this.extensionData = extensionData;
            this.finalizer = finalizer;
            this.layout = layout;
        }

        public Layout getLayout() {
            return layout;
        }
        
        public int getSize() {
            return layout.size();
        }
    }
    
    DataObject(org.jruby.Ruby runtime, org.jruby.RubyClass klass, MetaData metaData) {
        super(runtime, klass);
        this.metaData = metaData;
        this.memory =  jnr.ffi.Memory.allocateDirect(jnr.ffi.Runtime.getSystemRuntime(), metaData.getSize());

        Finalizer finalizer = null;
        if (metaData.finalizer != null && !metaData.finalizer.isNil()) {
            finalizer = new Finalizer(this, finalizerQueue, memory, metaData.finalizer);
            finalizers.put(finalizer, Boolean.TRUE);
        }

        this.finalizer = finalizer;
    }
    
    private static final int getSize(Ruby runtime, RubyClass klass) {
        Object size = klass.getInternalVariables().getInternalVariable("__xni_size__");
        if (!(size instanceof Integer) && klass.getSuperClass() != null) {
            return getSize(runtime, klass.getSuperClass());
        }
        
        return size != null ? (Integer) size : 0;
    }

    private static Layout getLayoutIVar(Ruby runtime, RubyClass klass) {
        Object layout = klass.getInternalVariable("__xni_layout__");
        if (!(layout instanceof Layout) && klass.getSuperClass() != null) {
            return getLayoutIVar(runtime, klass.getSuperClass());
        }

        return layout != null ? (Layout) layout : null;
    }
    
    private static Layout getLayout(Ruby runtime, RubyClass klass) {
        Layout layout;
        int size;
        if ((layout = getLayoutIVar(runtime, klass)) == null && (size = getSize(runtime, klass)) != 0) {
            layout = new Layout(size);
            klass.getInternalVariables().setInternalVariable("__xni_layout__", layout);
        }
        
        return layout;
    }

    static final MetaData getMetaData(Ruby runtime, RubyClass klass) {
        Layout layout = getLayout(runtime, klass);
        ExtensionData extensionData = Extension.getExtensionData(runtime, klass.getInstanceVariable("@__xni__"));
        return new MetaData(extensionData, layout, (Function) klass.getInternalVariable("__xni_finalizer__"));

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
        
        return klass;
    }

    
    @JRubyMethod(name = "__xni_data_fields__", visibility = Visibility.PRIVATE, module = true, rest=true)
    public static IRubyObject data_fields(ThreadContext context, IRubyObject self, IRubyObject[] fields) {
        Ruby runtime = context.getRuntime();
        
        if (self.getInternalVariables().hasInternalVariable("__xni_layout__")) {
            throw runtime.newRuntimeError("data fields already specified");
        }
        
        List<Layout.Field> dataFields = new ArrayList<Layout.Field>();
        for (int i = 0, offset = 0; i < fields.length; i += 2) {
            String name = fields[i].asJavaString();
            Type type = (Type) fields[i + 1];
            offset = align(offset, type.alignment());
            dataFields.add(new Layout.Field(name, type, offset));
            offset += type.size();
        }
        
        self.getInternalVariables().setInternalVariable("__xni_layout__", new Layout(dataFields));
        
        return context.getRuntime().getNil();
    }


    private static IRubyObject data_accessors(ThreadContext context, IRubyObject self, IRubyObject[] args, boolean read, boolean write) {
        Layout layout = getLayout(context.getRuntime(), (RubyClass) self);
        for (IRubyObject o : args) {
            if (!(o instanceof RubySymbol)) {
                throw context.getRuntime().newTypeError(o, context.getRuntime().getSymbol());
            }
            
            Layout.Field field = layout.getField(o.asJavaString());
            MemoryOp op = MemoryOp.getMemoryOp(field.type);
            if (op == null) {
                throw context.getRuntime().newRuntimeError("unsupported type " + field.type);
            }
            
            if (read) ((RubyModule) self).addMethod(o.toString(), new DataReader((RubyModule) self, op, field.offset));
            if (write) ((RubyModule) self).addMethod(o.toString() + "=", new DataWriter((RubyModule) self, op, field.offset));
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
    public static IRubyObject define_method(ThreadContext context, IRubyObject self, IRubyObject rbMethodName, IRubyObject rbFunction) {

        ExtensionData extData = Extension.getExtensionData(context.getRuntime(), self.getInstanceVariables().getInstanceVariable("@__xni__"));
        DataObjectMethod method = new DataObjectMethod((RubyClass) self, extData, (Function) rbFunction);
        method.setName(rbMethodName.asJavaString());

        ((RubyClass) self).addMethod(rbMethodName.asJavaString(), method);
        return context.nil;
    }

    @JRubyMethod(name = "__xni_define_singleton_method__", visibility = Visibility.PRIVATE, module = true)
    public static IRubyObject define_singleton_method(ThreadContext context, IRubyObject self, IRubyObject rbMethodName, IRubyObject rbFunction) {

        ExtensionData extData = Extension.getExtensionData(context.getRuntime(), self.getInstanceVariables().getInstanceVariable("@__xni__"));
        ExtensionMethod method = new ExtensionMethod(self.getSingletonClass(), extData, (Function) rbFunction);
        method.setName(rbMethodName.asJavaString());

        self.getSingletonClass().addMethod(rbMethodName.asJavaString(), method);
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

    @JRubyMethod(name = "__xni_set_size__", required = 1, visibility = Visibility.PRIVATE, module = true)
    public static IRubyObject set_size(ThreadContext context, IRubyObject recv, IRubyObject function) {
        CallContext callContext = CallContext.getCallContext(com.kenai.jffi.Type.SINT, new com.kenai.jffi.Type[0], 
                CallingConvention.DEFAULT, false);
        int size = (int) Invoker.getInstance().invokeN0(callContext, ((Function) function).address());
        recv.getInternalVariables().setInternalVariable("__xni_size__", size);
        
        return context.getRuntime().getNil(); 
    }

    @JRubyMethod(name = "__xni_sizeof__", visibility = Visibility.PRIVATE, module = true)
    public static IRubyObject sizeof(ThreadContext context, IRubyObject recv, IRubyObject obj) {
        if (obj instanceof DataObject) {
            return context.getRuntime().newFixnum(((DataObject) obj).metaData.getSize());
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

    @JRubyMethod(name = "__xni_address__")
    public IRubyObject address(ThreadContext context) {
        return context.getRuntime().newFixnum(memory.address());
    }

    void release() {
        int f = flags;
        if ((f & RELEASED) == 0 && FLAGS_UPDATER.compareAndSet(this, f, f | RELEASED)) {
            if (finalizer != null) {
                finalizer.finalizeReferent();
            }
        }
    }

    MetaData getMetaData() {
        return metaData;
    }
    
    jnr.ffi.Pointer getMemory() {
        return memory;
    }
    
    long address() {
        return memory.address();
    }


    private static final class Finalizer extends FinalizableWeakReference<Object> {
        private static final CallContext callContext = CallContext.getCallContext(com.kenai.jffi.Type.VOID, 
                new com.kenai.jffi.Type[] { com.kenai.jffi.Type.POINTER, com.kenai.jffi.Type.POINTER }, CallingConvention.DEFAULT, false);
        private final jnr.ffi.Pointer memory;
        private final Function finalizer;
        private final ExtensionData extensionData;
        private final AtomicBoolean finalized = new AtomicBoolean();

        private Finalizer(DataObject dataObject, FinalizableReferenceQueue queue, jnr.ffi.Pointer memory, 
                          Function finalizer) {
            super(dataObject, queue);
            this.memory = memory;
            this.finalizer = finalizer;
            this.extensionData = dataObject.metaData.extensionData;
        }

        @Override
        public void finalizeReferent() {
            if (!finalized.getAndSet(true)) {
                try {
                    Invoker.getInstance().invokeN2(callContext, finalizer.address(), extensionData.address(), memory.address());
                } finally {
                    finalizers.remove(this);
                }
            }
        }
    }
}
