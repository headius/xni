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
public class AutoReleasePool extends RubyObject {
    private static final ThreadLocal<AutoReleasePool> currentPool = new ThreadLocal<AutoReleasePool>();
    private DataObject head, tail;

    public static RubyModule createAutoReleasePoolClass(Ruby runtime, RubyModule module) {

        RubyModule autoReleasePoolClass = runtime.defineClassUnder("AutoReleasePool", runtime.getObject(), ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR, 
                module);

        autoReleasePoolClass.defineAnnotatedMethods(AutoReleasePool.class);
        autoReleasePoolClass.defineAnnotatedConstants(AutoReleasePool.class);

        return autoReleasePoolClass;
    }

    public AutoReleasePool(Ruby runtime, RubyClass metaClass) {
        super(runtime, metaClass);
    }
    
    @JRubyMethod(name = "new", meta = true)
    public static IRubyObject newInstance(ThreadContext context, IRubyObject recv, Block block) {
        if (!block.isGiven()) {
            throw context.runtime.newArgumentError("no block given");
        } 
        
        AutoReleasePool pool = new AutoReleasePool(context.runtime, (RubyClass) recv);
        AutoReleasePool prevPool = currentPool.get();

        try {
            currentPool.set(pool);
            return block.yield(context, pool);
        
        } finally {
            currentPool.set(prevPool);
            pool.release();
        }
    }
    
    static AutoReleasePool getActivePool(Ruby runtime) {
        AutoReleasePool pool = currentPool.get();
        if (pool == null) {
            throw runtime.newRuntimeError("no active AutoReleasePool");
        }
        
        return pool;
    }
    
    void release() {
        DataObject obj = head;
        while (obj != null) {
            try {
                obj.release();
            } catch (Throwable unused) {}
            obj = obj.next;
        }
    }
    
    void remove(DataObject obj) {
        if (obj == head) {
            head = head.next;
        }
        if (obj == tail) {
            tail = tail.next;
        }
        if (obj.prev != null) {
            obj.prev.next = obj.next;
        }
        if (obj.next != null) {
            obj.next.prev = obj.prev;
        }
    }
    
    void add(DataObject obj) {
        if (obj.prev != null || obj.next != null) {
            return;
        }

        obj.next = null;
        if (head == null) {
            head = obj;
        }
        
        if (tail != null) {
            obj.prev = tail;
            tail.next = obj;
        }
        tail = obj;
    }
}
