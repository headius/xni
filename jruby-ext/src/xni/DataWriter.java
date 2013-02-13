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
public final class DataWriter extends DynamicMethod {
    private static final Arity ARITY = Arity.ONE_ARGUMENT;
    private final IRubyObject fieldName;

    public DataWriter(RubyModule implementationClass, IRubyObject fieldName) {
        super(implementationClass, Visibility.PUBLIC, CallConfiguration.FrameNoneScopeNone);
        this.fieldName = fieldName;
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule rubyModule, String methodName, IRubyObject[] args, Block block) {
        ARITY.checkArity(context.runtime, args);
        return call(context, self, rubyModule, methodName, args[0]);
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String name, IRubyObject value) {
        DataObject obj = (DataObject) self;
        return obj.getMetaData().getLayout().put(context, obj.getMemory(context), fieldName, value);
    }

    @Override
    public DynamicMethod dup() {
        return this;
    }
}
