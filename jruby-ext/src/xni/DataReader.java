package xni;

import org.jruby.RubyModule;
import org.jruby.internal.runtime.methods.CallConfiguration;
import org.jruby.internal.runtime.methods.DynamicMethod;
import org.jruby.runtime.Arity;
import org.jruby.runtime.Block;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.Visibility;
import org.jruby.runtime.builtin.IRubyObject;

/**
 * 
 */
public final class DataReader extends DynamicMethod {
    private static final Arity ARITY = Arity.NO_ARGUMENTS;
    private final MemoryOp op;
    private final long offset;

    public DataReader(RubyModule implementationClass, MemoryOp op, long offset) {
        super(implementationClass, Visibility.PUBLIC, CallConfiguration.FrameNoneScopeNone);
        this.op = op;
        this.offset = offset;
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule rubyModule, String methodName, IRubyObject[] args, Block block) {
        ARITY.checkArity(context.runtime, args);
        return call(context, self, rubyModule, methodName);
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String name) {
        DataObject obj = (DataObject) self;
        return op.get(context, obj.getMemory(), offset);
    }

    @Override
    public DynamicMethod dup() {
        return this;
    }
}
