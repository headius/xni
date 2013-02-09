package xni;

import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyModule;
import org.jruby.anno.JRubyMethod;
import org.jruby.ext.ffi.jffi.Function;
import org.jruby.internal.runtime.methods.DynamicMethod;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.Visibility;
import org.jruby.runtime.builtin.IRubyObject;

/**
 * 
 */
public class Extension {
    /**
     * Registers the Extension module in the JRuby runtime.
     * @param runtime The JRuby runtime to register the new class in.
     * @return The new class
     */
    public static RubyModule createExtensionModule(Ruby runtime, RubyModule module) {

        RubyModule extensionModule = runtime.defineModuleUnder("Extension", module);

        extensionModule.defineAnnotatedMethods(Extension.class);
        extensionModule.defineAnnotatedConstants(Extension.class);

        return extensionModule;
    }

    @JRubyMethod(name = "__xni_define_method__", visibility = Visibility.PRIVATE, module=true)
    public static IRubyObject define_method(ThreadContext context, IRubyObject self, IRubyObject rbMethodName, 
                                            IRubyObject rbFunction, IRubyObject parameterCount) {
        Function function = (Function) rbFunction;

        DynamicMethod nativeMethod = function.createDynamicMethod((RubyModule) self);
        nativeMethod.setName(rbMethodName.asJavaString());
        ExtensionMethod method = new ExtensionMethod(self.getSingletonClass(), getExtensionData(context.getRuntime(), self), nativeMethod);

        self.getSingletonClass().addMethod(rbMethodName.asJavaString(), method);
        return context.nil;
    }

    @JRubyMethod(name = "__xni_ext_data__", visibility = Visibility.PRIVATE, module=true)
    public static IRubyObject ext_data(ThreadContext context, IRubyObject self, IRubyObject extData) {
        if (!(extData instanceof ExtensionData)) {
            throw context.getRuntime().newTypeError(extData, context.getRuntime().getModule("XNI").getClass("ExtensionData"));
        }
        
        self.getInternalVariables().setInternalVariable("__xni_ext_data__", extData);
        
        return context.getRuntime().getNil();
    }
    
    static ExtensionData getExtensionData(Ruby runtime, IRubyObject self) {
        IRubyObject extData = (IRubyObject) self.getInternalVariables().getInternalVariable("__xni_ext_data__");
        if (!(extData instanceof ExtensionData)) {
            throw runtime.newRuntimeError("invalid __xni_ext_data__");
        }
        
        return (ExtensionData) extData;
    }
}
