package xni;

import org.jruby.Ruby;
import org.jruby.RubyModule;

import java.io.IOException;

/**
 *
 */
public class XNILibrary implements org.jruby.runtime.load.Library {
    @Override
    public void load(Ruby runtime, boolean wrap) throws IOException {
        if (!runtime.getInstanceConfig().isNativeEnabled()) {
            throw runtime.newLoadError("Native API access is disabled");
        }
        RubyModule xni = runtime.getOrCreateModule("XNI");
        Platform.createPlatformClass(runtime, xni);
        Type.createTypeClass(runtime, xni);
        Pointer.createPointerClass(runtime, xni);
        Function.createFunctionClass(runtime, xni);
        DynamicLibrary.createDynamicLibraryClass(runtime, xni);
        ExtensionData.createExtensionDataClass(runtime, xni);
        Extension.createExtensionModule(runtime, xni);
        AutoReleasePool.createAutoReleasePoolClass(runtime, xni);
        DataObject.createDataObjectClass(runtime, xni);
        DataObjectFactory.createDataObjectFactoryClass(runtime, xni);
    }
}
