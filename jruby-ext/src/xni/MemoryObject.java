package xni;

import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyObject;

/**
 *
 */
abstract public class MemoryObject extends RubyObject {
    private final long address;
    protected MemoryObject(Ruby runtime, RubyClass metaClass, long address) {
        super(runtime, metaClass);
        this.address = address;
    }
    
    public final long address() {
        return address;
    }
}
