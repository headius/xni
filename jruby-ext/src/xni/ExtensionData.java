package xni;

import com.kenai.jffi.CallContext;
import com.kenai.jffi.CallingConvention;
import com.kenai.jffi.Type;
import jnr.ffi.Memory;
import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyModule;
import org.jruby.RubyObject;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.IRubyObject;

/**
 *
 */
public class ExtensionData extends RubyObject {
    private static final CallContext loadContext = CallContext.getCallContext(com.kenai.jffi.Type.SINT, new Type[] { Type.POINTER, com.kenai.jffi.Type.POINTER },
            CallingConvention.DEFAULT, false);
    private static final CallContext unloadContext = CallContext.getCallContext(com.kenai.jffi.Type.VOID, new Type[] { Type.POINTER, com.kenai.jffi.Type.POINTER },
            CallingConvention.DEFAULT, false);
    private final jnr.ffi.Pointer nativeExtensionData;
    
    public ExtensionData(Ruby runtime, RubyClass metaClass, jnr.ffi.Pointer nativeExtensionData) {
        super(runtime, metaClass);
        this.nativeExtensionData = nativeExtensionData;
    }

    public static RubyClass createExtensionDataClass(Ruby runtime, RubyModule module) {

        RubyClass klass = runtime.defineClassUnder("ExtensionData", runtime.getObject(), ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR, 
                module);

        klass.defineAnnotatedMethods(ExtensionData.class);
        klass.defineAnnotatedConstants(ExtensionData.class);

        return klass;
    }

    @JRubyMethod(name = { "new" }, meta = true)
    public static IRubyObject newInstance(ThreadContext context, IRubyObject recv, IRubyObject load, IRubyObject unload) {
        jnr.ffi.Pointer memory = Memory.allocateDirect(jnr.ffi.Runtime.getSystemRuntime(), jnr.ffi.NativeType.ADDRESS);
        if (!load.isNil()) {
            long res = com.kenai.jffi.Invoker.getInstance().invokeN2(loadContext, ((DynamicLibrary.Symbol) load).address(),
                    0L, memory.address());

            if (res < 0) {
                throw context.getRuntime().newLoadError(((DynamicLibrary.Symbol) load).name() 
                        + " failed with error code " + res);
            }
        }

        jnr.ffi.Pointer nativeExtensionData = memory.getPointer(0);
        
        return new ExtensionData(context.getRuntime(), (RubyClass) recv, 
                nativeExtensionData != null ? nativeExtensionData : jnr.ffi.Runtime.getSystemRuntime().getMemoryManager().newOpaquePointer(0));
    }
    
    long address() {
        return nativeExtensionData.address();
    }
}
