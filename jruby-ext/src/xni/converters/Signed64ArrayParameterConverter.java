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
public class Signed64ArrayParameterConverter implements ToNativeConverter<IRubyObject, long[]> {
    private static final Signed64ArrayParameterConverter IN = new Signed64ArrayParameterConverter(ArrayFlags.IN);
    private static final Signed64ArrayParameterConverter OUT = new Signed64ArrayParameterConverter(ArrayFlags.OUT);
    private static final Signed64ArrayParameterConverter INOUT = new Signed64ArrayParameterConverter(ArrayFlags.IN | ArrayFlags.OUT);
    
    protected final int arrayFlags;

    public static ToNativeConverter<IRubyObject, long[]> getInstance(int arrayFlags) {
        return ArrayFlags.isOut(arrayFlags) ? ArrayFlags.isIn(arrayFlags) ? INOUT : OUT : IN;
    }

    private Signed64ArrayParameterConverter(int arrayFlags) {
        this.arrayFlags = arrayFlags;
    }

    @Override
    public long[] toNative(IRubyObject value, ToNativeContext context) {
        RubyArray rbArray = value.convertToArray();
        if (value.isNil()) {
            return null;
        }

        long[] arr = new long[rbArray.getLength()];
        
        if (ArrayFlags.isIn(arrayFlags)) {
            for (int i = 0; i < arr.length; i++) {
                arr[i] = Util.longValue(rbArray.entry(i));
            }
        }
        
        return arr;
    }

    @Override
    public NativeType nativeType() {
        return NativeType.CARRAY;
    }

    public static final class Out extends Signed64ArrayParameterConverter implements PostInvocation<IRubyObject, long[]> {
        private Out(int parameterFlags) {
            super(parameterFlags);
        }

        @Override
        public void postInvoke(IRubyObject rbValue, long[] arr, ToNativeContext context) {
            if (rbValue instanceof RubyArray && arr != null) {
                RubyArray rbArray = (RubyArray) rbValue;
                Ruby runtime = rbArray.getRuntime();
                for (int i = 0; i < arr.length; i++) {
                    rbArray.store(i, Util.newSigned64(runtime, arr[i]));
                }
            }
        }
    }
}
