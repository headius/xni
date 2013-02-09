package xni;

import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyModule;
import org.jruby.RubyObject;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.Block;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.IRubyObject;

/**
 *
 */
public class DataObjectFactory extends RubyObject {
    public static enum Type {
        AUTORELEASE,
        GC_MANAGED;
    }

    private final RubyClass dataClass;
    private final ObjectAllocator allocator;
    
    /**
     * Registers the StructLayout class in the JRuby runtime.
     * @param runtime The JRuby runtime to register the new class in.
     * @return The new class
     */
    public static RubyClass createDataObjectFactoryClass(Ruby runtime, RubyModule module) {

        RubyClass klass = runtime.defineClassUnder("Factory", 
                runtime.getObject(), ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR,
                module.getClass("DataObject"));

        klass.defineAnnotatedMethods(DataObjectFactory.class);
        klass.defineAnnotatedConstants(DataObjectFactory.class);

        return klass;
    }


    public DataObjectFactory(Ruby runtime, RubyClass dataClass, Type type) {
        super(runtime, runtime.getModule("XNI").getClass("DataObject").getClass("Factory"));
        this.dataClass = dataClass;
        this.allocator = type == Type.AUTORELEASE 
                ? new DataObject.AutoReleaseAllocator(DataObject.getMetaData(runtime, dataClass))
                : new DataObject.GarbageCollectedAllocator(DataObject.getMetaData(runtime, dataClass));
    }

    private DataObject allocate(ThreadContext context) {
        return (DataObject) allocator.allocate(context.getRuntime(), dataClass);
    }
    
    @JRubyMethod(name = "new", omit = true)
    public IRubyObject newInstance(ThreadContext context, Block block) {
        DataObject obj = allocate(context);
        dataClass.getBaseCallSite(RubyClass.CS_IDX_INITIALIZE).call(context, obj, obj, block);

        return obj;
    }
    
    @JRubyMethod(name = "new", omit = true)
    public IRubyObject newInstance(ThreadContext context, IRubyObject arg0, Block block) {
        DataObject obj = allocate(context);
        dataClass.getBaseCallSite(RubyClass.CS_IDX_INITIALIZE).call(context, obj, obj, arg0, block);

        return obj;
    }

    @JRubyMethod(name = "new", omit = true)
    public IRubyObject newInstance(ThreadContext context, IRubyObject arg0, IRubyObject arg1, Block block) {
        DataObject obj = allocate(context);
        dataClass.getBaseCallSite(RubyClass.CS_IDX_INITIALIZE).call(context, obj, obj, arg0, arg1, block);

        return obj;
    }

    @JRubyMethod(name = "new", omit = true)
    public IRubyObject newInstance(ThreadContext context, IRubyObject arg0, IRubyObject arg1, IRubyObject arg2, Block block) {
        DataObject obj = allocate(context);
        dataClass.getBaseCallSite(RubyClass.CS_IDX_INITIALIZE).call(context, obj, obj, arg0, arg1, arg2, block);

        return obj;
    }
    
    @JRubyMethod(name = "new", omit = true, rest = true)
    public IRubyObject newInstance(ThreadContext context, IRubyObject[] args, Block block) {
        DataObject obj = allocate(context);
        dataClass.getBaseCallSite(RubyClass.CS_IDX_INITIALIZE).call(context, obj, obj, args, block);
        
        return obj;
    }
}
