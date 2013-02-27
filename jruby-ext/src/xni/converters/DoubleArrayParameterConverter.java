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
public class DoubleArrayParameterConverter implements ToNativeConverter<IRubyObject, double[]> {
    private static final DoubleArrayParameterConverter IN = new DoubleArrayParameterConverter(ArrayFlags.IN);
    private static final DoubleArrayParameterConverter OUT = new DoubleArrayParameterConverter(ArrayFlags.OUT);
    private static final DoubleArrayParameterConverter INOUT = new DoubleArrayParameterConverter(ArrayFlags.IN | ArrayFlags.OUT);
    
    protected final int arrayFlags;

    public static ToNativeConverter<IRubyObject, double[]> getInstance(int arrayFlags) {
        return ArrayFlags.isOut(arrayFlags) ? ArrayFlags.isIn(arrayFlags) ? INOUT : OUT : IN;
    }

    private DoubleArrayParameterConverter(int arrayFlags) {
        this.arrayFlags = arrayFlags;
    }

    @Override
    public double[] toNative(IRubyObject value, ToNativeContext context) {
        RubyArray rbArray = value.convertToArray();
        if (value.isNil()) {
            return null;
        }

        double[] arr = new double[rbArray.getLength()];
        
        if (ArrayFlags.isIn(arrayFlags)) {
            for (int i = 0; i < arr.length; i++) {
                arr[i] = Util.doubleValue(rbArray.entry(i));
            }
        }
        
        return arr;
    }

    @Override
    public NativeType nativeType() {
        return NativeType.CARRAY;
    }

    public static final class Out extends DoubleArrayParameterConverter implements ToNativeConverter.PostInvocation<IRubyObject, double[]> {
        private Out(int parameterFlags) {
            super(parameterFlags);
        }

        @Override
        public void postInvoke(IRubyObject rbValue, double[] arr, ToNativeContext context) {
            if (rbValue instanceof RubyArray && arr != null) {
                RubyArray rbArray = (RubyArray) rbValue;
                Ruby runtime = rbArray.getRuntime();
                for (int i = 0; i < arr.length; i++) {
                    rbArray.store(i, runtime.newFloat(arr[i]));
                }
            }
        }
    }
}
