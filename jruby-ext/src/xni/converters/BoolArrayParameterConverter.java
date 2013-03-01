package xni.converters;

import com.kenai.jffi.ArrayFlags;
import org.jruby.Ruby;
import org.jruby.RubyArray;
import org.jruby.runtime.builtin.IRubyObject;
import xni.NativeType;
import xni.ToNativeContext;
import xni.ToNativeConverter;
import xni.Util;

/**
 *
 */
public class BoolArrayParameterConverter implements ToNativeConverter<IRubyObject, byte[]> {
    private static final BoolArrayParameterConverter IN = new BoolArrayParameterConverter(ArrayFlags.IN);
    private static final BoolArrayParameterConverter OUT = new BoolArrayParameterConverter.Out(ArrayFlags.OUT);
    private static final BoolArrayParameterConverter INOUT = new BoolArrayParameterConverter.Out(ArrayFlags.IN | ArrayFlags.OUT);
    
    protected final int arrayFlags;

    public static ToNativeConverter<IRubyObject, byte[]> getInstance(int arrayFlags) {
        return ArrayFlags.isOut(arrayFlags) ? ArrayFlags.isIn(arrayFlags) ? INOUT : OUT : IN;
    }

    private BoolArrayParameterConverter(int arrayFlags) {
        this.arrayFlags = arrayFlags;
    }

    @Override
    public byte[] toNative(IRubyObject value, ToNativeContext context) {
        RubyArray rbArray = value.convertToArray();
        byte[] arr = new byte[rbArray.getLength()];
        
        if (ArrayFlags.isIn(arrayFlags)) {
            for (int i = 0; i < arr.length; i++) {
                arr[i] = (byte) (rbArray.entry(i).isTrue() ? 1 : 0);
            }
        }
        
        return arr;
    }

    @Override
    public NativeType nativeType() {
        return NativeType.CARRAY;
    }

    public static final class Out extends BoolArrayParameterConverter implements PostInvocation<IRubyObject, byte[]> {
        private Out(int parameterFlags) {
            super(parameterFlags);
        }

        @Override
        public void postInvoke(IRubyObject rbValue, byte[] arr, ToNativeContext context) {
            if (rbValue instanceof RubyArray && arr != null) {
                RubyArray rbArray = (RubyArray) rbValue;
                Ruby runtime = rbArray.getRuntime();
                for (int i = 0; i < arr.length; i++) {
                    rbArray.store(i, arr[i] != 0 ? runtime.getTrue() : runtime.getFalse());
                }
            }
        }
    }
}
