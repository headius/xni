package xni;

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
public class Pointer extends MemoryObject {
    public static RubyClass createPointerClass(Ruby runtime, RubyModule xniModule) {
        RubyClass pointerClass = xniModule.defineClassUnder("Pointer", runtime.getObject(),
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        pointerClass.defineAnnotatedMethods(Pointer.class);
        pointerClass.defineAnnotatedConstants(Pointer.class);

        return pointerClass;
    }
    
    public Pointer(Ruby runtime, long address) {
        super(runtime, runtime.getModule("XNI").getClass("Pointer"), address);
    }
    
    @JRubyMethod
    public final IRubyObject address(ThreadContext context) {
        return context.getRuntime().newFixnum(address());
    }
}
