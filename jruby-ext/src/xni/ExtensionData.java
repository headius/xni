package xni;

import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyModule;
import org.jruby.RubyObject;
import org.jruby.anno.JRubyMethod;
import org.jruby.ext.ffi.Pointer;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.IRubyObject;

/**
 *
 */
public class ExtensionData extends RubyObject {
    private final Pointer nativeExtensionData;
    
    public ExtensionData(Ruby runtime, RubyClass metaClass, Pointer nativeExtensionData) {
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
    public static IRubyObject newInstance(ThreadContext context, IRubyObject recv, IRubyObject pointer) {
        return new ExtensionData(context.getRuntime(), (RubyClass) recv, (Pointer) pointer);
    }
    
    Pointer getNativeExtensionData() {
        return nativeExtensionData;
    }

    @JRubyMethod(name = { "pointer", "to_ptr" })
    public IRubyObject pointer(ThreadContext context) {
        return nativeExtensionData;
    }
}
