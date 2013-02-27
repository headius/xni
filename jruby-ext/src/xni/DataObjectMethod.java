package xni;

import com.kenai.jffi.CallContext;
import com.kenai.jffi.CallingConvention;
import com.kenai.jffi.HeapInvocationBuffer;
import com.kenai.jffi.Invoker;
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
public class DataObjectMethod extends DynamicMethod {
    private final Arity arity;
    private final ExtensionData extensionData;
    private final Function function;
    private final CallContext callContext;

    public DataObjectMethod(RubyModule implementationClass, ExtensionData extensionData, Function function) {
        super(implementationClass, Visibility.PUBLIC, CallConfiguration.FrameNoneScopeNone);
        this.arity = Arity.createArity(function.getParameterCount());
        this.extensionData = extensionData;
        this.function = function;
        
        com.kenai.jffi.Type resultType = function.getResultType().nativeType.jffiType();
        com.kenai.jffi.Type[] parameterTypes = new com.kenai.jffi.Type[function.getParameterCount() + 2];
        parameterTypes[0] = com.kenai.jffi.Type.POINTER;
        parameterTypes[1] = com.kenai.jffi.Type.POINTER;
        for (int i = 0; i < function.getParameterCount(); i++) {
            parameterTypes[i + 2] = function.getParameterType(i).nativeType.jffiType();
        }

        callContext = CallContext.getCallContext(resultType, parameterTypes, CallingConvention.DEFAULT, false);
    }

    @Override
    public IRubyObject call(ThreadContext context, IRubyObject self, RubyModule klazz, String methodName, IRubyObject[] args, Block block) {
        arity.checkArity(context.getRuntime(), args);
        
        HeapInvocationBuffer invocationBuffer = new HeapInvocationBuffer(callContext);
        invocationBuffer.putAddress(extensionData.address());
        invocationBuffer.putAddress(((DataObject) self).address());

        return BufferInvokeUtil.invoke(context, args, function, invocationBuffer, callContext);
    }

    @Override
    public DynamicMethod dup() {
        return this;
    }
}
