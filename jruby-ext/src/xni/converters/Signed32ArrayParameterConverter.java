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
public class Signed32ArrayParameterConverter implements ToNativeConverter<IRubyObject, int[]> {
    private static final Signed32ArrayParameterConverter IN = new Signed32ArrayParameterConverter(ArrayFlags.IN);
    private static final Signed32ArrayParameterConverter OUT = new Signed32ArrayParameterConverter(ArrayFlags.OUT);
    private static final Signed32ArrayParameterConverter INOUT = new Signed32ArrayParameterConverter(ArrayFlags.IN | ArrayFlags.OUT);
    
    protected final int arrayFlags;

    public static ToNativeConverter<IRubyObject, int[]> getInstance(int arrayFlags) {
        return ArrayFlags.isOut(arrayFlags) ? ArrayFlags.isIn(arrayFlags) ? INOUT : OUT : IN;
    }

    private Signed32ArrayParameterConverter(int arrayFlags) {
        this.arrayFlags = arrayFlags;
    }

    @Override
    public int[] toNative(IRubyObject value, ToNativeContext context) {
        RubyArray rbArray = value.convertToArray();
        if (value.isNil()) {
            return null;
        }

        int[] arr = new int[rbArray.getLength()];
        
        if (ArrayFlags.isIn(arrayFlags)) {
            for (int i = 0; i < arr.length; i++) {
                arr[i] = Util.int32Value(rbArray.entry(i));
            }
        }
        
        return arr;
    }

    @Override
    public NativeType nativeType() {
        return NativeType.CARRAY;
    }

    public static final class Out extends Signed32ArrayParameterConverter implements PostInvocation<IRubyObject, int[]> {
        private Out(int parameterFlags) {
            super(parameterFlags);
        }

        @Override
        public void postInvoke(IRubyObject rbValue, int[] arr, ToNativeContext context) {
            if (rbValue instanceof RubyArray && arr != null) {
                RubyArray rbArray = (RubyArray) rbValue;
                Ruby runtime = rbArray.getRuntime();
                for (int i = 0; i < arr.length; i++) {
                    rbArray.store(i, Util.newSigned32(runtime, arr[i]));
                }
            }
        }
    }
}
