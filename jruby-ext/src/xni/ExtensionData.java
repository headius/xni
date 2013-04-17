package xni;

import com.kenai.jffi.*;
import com.kenai.jffi.CallingConvention;
import com.kenai.jffi.Type;
import jnr.ffi.Memory;
import jnr.ffi.Struct;
import jnr.ffi.Runtime;
import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyModule;
import org.jruby.RubyObject;
import org.jruby.anno.JRubyMethod;
import org.jruby.exceptions.RaiseException;
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
    private final NativeExtensionData nativeExtensionData;
    
    public ExtensionData(Ruby runtime, RubyClass metaClass, NativeExtensionData nativeExtensionData) {
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
        jnr.ffi.Runtime runtime = jnr.ffi.Runtime.getSystemRuntime();
        jnr.ffi.Pointer memory = Memory.allocateDirect(runtime, jnr.ffi.NativeType.ADDRESS);
        if (!load.isNil()) {
            long res = com.kenai.jffi.Invoker.getInstance().invokeN2(loadContext, ((DynamicLibrary.Symbol) load).address(),
                    0L, memory.address());

            if (res < 0) {
                throw context.getRuntime().newLoadError(((DynamicLibrary.Symbol) load).name() 
                        + " failed with error code " + res);
            }
        }

        NativeExtensionData nativeExtensionData = new NativeExtensionData(context.getRuntime(), runtime, memory.getPointer(0));
        
        return new ExtensionData(context.getRuntime(), (RubyClass) recv, nativeExtensionData);
    }
    
    long address() {
        return nativeExtensionData.address;
    }
    
    long ext_data() {
        return nativeExtensionData.extData.address();
    }
    

    public static final class VMException extends Struct {
        public final Signed32 exc = new Signed32();

        public VMException(Runtime runtime, jnr.ffi.Pointer memory) {
            super(runtime);
            useMemory(memory);
        }
    }
    
    public static class VMRaiseException implements Closure {
        private final Ruby ruby;
        private final jnr.ffi.Runtime runtime;

        public VMRaiseException(Ruby ruby, jnr.ffi.Runtime runtime) {
            this.ruby = ruby;
            this.runtime = runtime;
        }

        public void invoke(Buffer buffer) {
            long exptr = buffer.getAddress(1);
            VMException ex = exptr != 0 ? new VMException(runtime, runtime.getMemoryManager().newPointer(exptr)) : null;
            long msgptr = buffer.getAddress(2);
            jnr.ffi.Pointer msg = msgptr != 0 ? runtime.getMemoryManager().newPointer(msgptr) : null;

            RubyClass exceptionClass = ruby.getException();
            switch (ex.exc.intValue()) {
                case 2:
                    exceptionClass = ruby.getRuntimeError();
                    break;
                
                case 3:
                    exceptionClass = ruby.getArgumentError();
                    break;

                case 4:
                    exceptionClass = ruby.getIndexError();
                    break;
            }

            throw ruby.newRaiseException(exceptionClass, msg.getString(0));
        }
    }
    
    public static final class NativeRubyVM extends jnr.ffi.Struct {
        public final Pointer raise = new Pointer();
        
        private final Closure.Handle raiseHandle;

        public NativeRubyVM(Ruby ruby, Runtime runtime) {
            super(runtime);
            useMemory(Memory.allocateDirect(runtime, Struct.size(this)));
            raiseHandle = com.kenai.jffi.ClosureManager.getInstance().newClosure(new VMRaiseException(ruby, runtime), Type.VOID,
                    new Type[] { Type.POINTER, Type.POINTER, Type.POINTER }, CallingConvention.DEFAULT);
            raise.set(raiseHandle.getAddress());
        }
    }
    
    public static final class NativeExtensionData extends jnr.ffi.Struct {
        // Same data layout as the C interface expects
        public final Pointer ext_data = new Pointer();
        public final Pointer vm_data = new Pointer();
        public final Pointer vm = new Pointer();
        
        // Keep-alive references 
        private final NativeRubyVM nativeRubyVM;
        private final jnr.ffi.Pointer extData;
        final long address;

        public NativeExtensionData(Ruby ruby, jnr.ffi.Runtime runtime, jnr.ffi.Pointer ext_data) {
            super(runtime);
            useMemory(Memory.allocateDirect(runtime, Struct.size(this)));
            this.address = Struct.getMemory(this).address();
            this.extData = ext_data != null ? ext_data : runtime.getMemoryManager().newOpaquePointer(0xdeadbeef);
            this.ext_data.set(extData);
            nativeRubyVM = new NativeRubyVM(ruby, runtime);
            this.vm.set(Struct.getMemory(nativeRubyVM));
        }
    }
    
}
