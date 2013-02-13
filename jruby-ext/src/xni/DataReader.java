package xni;

import org.jruby.RubyModule;
import org.jruby.ext.ffi.StructLayout;
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
    private final IRubyObject fieldName;

    public DataReader(RubyModule implementationClass, IRubyObject fieldName) {
        super(implementationClass, Visibility.PUBLIC, CallConfiguration.FrameNoneScopeNone);
        this.fieldName = fieldName;
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule rubyModule, String methodName, IRubyObject[] args, Block block) {
        ARITY.checkArity(context.runtime, args);
        return call(context, self, rubyModule, methodName);
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String name) {
        DataObject obj = (DataObject) self;
        return obj.getMetaData().getLayout().get(context, obj.getMemory(context), fieldName);
    }

    @Override
    public DynamicMethod dup() {
        return this;
    }
}
