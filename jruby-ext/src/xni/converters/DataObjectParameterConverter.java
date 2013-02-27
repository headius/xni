package xni.converters;

import org.jruby.runtime.builtin.IRubyObject;
import xni.*;

public class DataObjectParameterConverter implements ToNativeConverter<IRubyObject, Object> {
    Type.DataObject type;

    public DataObjectParameterConverter(Type.DataObject type) {
        this.type = type;
    }

    @Override
    public Object toNative(IRubyObject value, ToNativeContext context) {
        if ((value instanceof DataObject && type.getObjectClass().isInstance(value)) || value.isNil()) {
            return value;

        } else {
            throw value.getRuntime().newTypeError(value, type.getObjectClass().getName() + " or nil");
        }
    }

    @Override
    public NativeType nativeType() {
        return NativeType.ADDRESS;
    }
}
