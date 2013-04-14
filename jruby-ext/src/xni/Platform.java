package xni;

import org.jruby.Ruby;
import org.jruby.RubyClass;
import org.jruby.RubyModule;
import org.jruby.RubyObject;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.Visibility;
import org.jruby.runtime.builtin.IRubyObject;


public final class Platform extends RubyObject {
    private static final java.util.Locale LOCALE = java.util.Locale.ENGLISH;
    private final jnr.ffi.Platform jnrPlatform;
    
    public static RubyClass createPlatformClass(Ruby runtime, RubyModule xniModule) {
        RubyClass platformClass = xniModule.defineClassUnder("Platform", runtime.getObject(),
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        platformClass.defineAnnotatedMethods(Platform.class);
        platformClass.defineAnnotatedConstants(Platform.class);

        return platformClass;
    }
    
    @JRubyMethod(meta = true)
    public static final IRubyObject system(ThreadContext context, IRubyObject self) {
        Object system = self.getInternalVariables().getInternalVariable("system");
        if (system instanceof Platform) {
            return (Platform) system;
        }

        synchronized (self) {
            if (!((system = self.getInternalVariables().getInternalVariable("system")) instanceof Platform)) {
                self.getInternalVariables().setInternalVariable("system", system = new Platform(context.getRuntime(), (RubyClass) self));
            }
        }        

        return (Platform) system;
    }


    public Platform(Ruby runtime, RubyClass metaClass) {
        super(runtime, metaClass);
        jnrPlatform = jnr.ffi.Platform.getNativePlatform();
    }
    
    @JRubyMethod(name = { "cpu", "arch" })
    public final IRubyObject cpu(ThreadContext context) {
        return context.getRuntime().newString(jnrPlatform.getCPU().name().toLowerCase(LOCALE));
    }

    @JRubyMethod
    public final IRubyObject os(ThreadContext context) {
        return context.getRuntime().newString(jnrPlatform.getOS().name().toLowerCase(LOCALE));
    }
    
    @JRubyMethod
    public final IRubyObject name(ThreadContext context) {
        return context.getRuntime().newString(String.format("%s-%s", jnrPlatform.getCPU().name().toLowerCase(LOCALE), jnrPlatform.getOS().name().toLowerCase(LOCALE)));
    }
    
    @JRubyMethod
    public final IRubyObject map_library_name(ThreadContext context, IRubyObject libraryName) {
        return context.getRuntime().newString(String.format(LOCALE, "%s-%s", 
                libraryName.asString().asJavaString(), determineLibExt()));
    }
    
    @JRubyMethod(name = { "mac?" })
    public final IRubyObject mac_p(ThreadContext context) {
        return context.getRuntime().newBoolean(jnrPlatform.getOS() == jnr.ffi.Platform.OS.DARWIN);
    }


    private final String determineLibExt() {
        switch (jnrPlatform.getOS()) {
            case WINDOWS:
                return "dll";
            
            case DARWIN:
                return "bundle";
            
            default:
                return "so";
        }
    }
}
