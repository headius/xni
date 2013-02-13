package xni;

import org.jruby.RubyModule;
import org.jruby.internal.runtime.methods.CacheableMethod;
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
public class DataObjectMethod extends DynamicMethod implements CacheableMethod {
    private final Arity arity;
    private final ExtensionData extensionData;
    private final DynamicMethod nativeMethod;

    public DataObjectMethod(RubyModule implementationClass, ExtensionData extensionData, DynamicMethod nativeMethod) {
        super(implementationClass, Visibility.PUBLIC, CallConfiguration.FrameNoneScopeNone);
        this.arity = Arity.createArity(nativeMethod.getArity().getValue() - 2);
        this.extensionData = extensionData;
        this.nativeMethod = nativeMethod;
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String methodName, IRubyObject[] args, Block block) {
        arity.checkArity(context.runtime, args);
        IRubyObject[] nativeArgs = new IRubyObject[args.length + 2];
        nativeArgs[0] = extensionData.getNativeExtensionData();
        nativeArgs[1] = ((DataObject) self).getMemory(context);
        System.arraycopy(args, 0, nativeArgs, 2, args.length);
        return nativeMethod.call(context, self, klazz, methodName, nativeArgs);
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String methodName) {
        arity.checkArity(context.runtime, 0);
        return nativeMethod.call(context, self, klazz, methodName, 
                extensionData.getNativeExtensionData(), ((DataObject) self).getMemory(context));
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String methodName, IRubyObject arg1) {
        arity.checkArity(context.runtime, 1);
        return nativeMethod.call(context, self, klazz, methodName, 
                extensionData.getNativeExtensionData(), ((DataObject) self).getMemory(context), arg1);
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String methodName, IRubyObject arg1, IRubyObject arg2) {
        arity.checkArity(context.runtime, 2);
        return nativeMethod.call(context, self, klazz, methodName, 
                new IRubyObject[] { extensionData.getNativeExtensionData(), ((DataObject) self).getMemory(context), arg1, arg2 });
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String methodName, IRubyObject arg1, IRubyObject arg2, IRubyObject arg3) {
        arity.checkArity(context.runtime, 3);
        return nativeMethod.call(context, self, klazz, methodName,
                new IRubyObject[] { extensionData.getNativeExtensionData(), ((DataObject) self).getMemory(context), arg1, arg2, arg3 });
    }

    @Override
    public DynamicMethod dup() {
        return this;
    }

    @Override
    public DynamicMethod getMethodForCaching() {
        return new DataObjectMethod(getImplementationClass(), extensionData,
                nativeMethod instanceof CacheableMethod ? ((CacheableMethod) nativeMethod).getMethodForCaching() : nativeMethod);
    }
}
