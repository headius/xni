package xni;

import com.kenai.jffi.*;
import org.jruby.Ruby;
import org.jruby.RubyNil;
import org.jruby.RubyString;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.IRubyObject;
import org.jruby.util.ByteList;

/**
 *
 */
class BufferInvokeUtil {
    
    static IRubyObject invoke(ThreadContext context, IRubyObject[] args, Function function, HeapInvocationBuffer invocationBuffer,
                              CallContext callContext) {
        
        InvocationSession session = new InvocationSession();
        Ruby runtime = context.getRuntime();
        for (int i = 0; i < function.getParameterCount(); i++) {
            ToNativeConverter toNativeConverter = function.getToNativeConverter(i);
            Object parameter;
            NativeType nativeType;
            
            if (toNativeConverter != null) {
                nativeType = toNativeConverter.nativeType();
                parameter = toNativeConverter.toNative(args[i], null);

                if (toNativeConverter instanceof ToNativeConverter.PostInvocation) {
                    addPostInvoke(session, (ToNativeConverter.PostInvocation) toNativeConverter,
                            args[i], parameter);
                }
            
            } else {
                nativeType = function.getParameterType(i).getNativeType();
                parameter = args[i];
            }
            
            if (nativeType == NativeType.CARRAY) {
                if (parameter == null) {
                    invocationBuffer.putAddress(0);
                } else {
                    invocationBuffer.putObject(parameter, function.getObjectParameterStrategy(i), function.getObjectParameterInfo(i));
                }
            
            } else {
                marshal(runtime, invocationBuffer, nativeType, parameter);
            }
        }
        
        try {
            return invoke(runtime, function.address(), function.getResultType().getNativeType(), invocationBuffer, callContext);
        
        } finally {
            session.finish();
        }
    }

    private static IRubyObject invoke(Ruby runtime, long function, NativeType nativeType, HeapInvocationBuffer invocationBuffer,
                                      CallContext callContext) {
        com.kenai.jffi.Invoker invoker = com.kenai.jffi.Invoker.getInstance();
        
        switch (nativeType) {
            case SCHAR:
                return Util.newSigned8(runtime, (byte) invoker.invokeInt(callContext, function, invocationBuffer));
                
            case UCHAR:
                return Util.newUnsigned8(runtime, (byte) invoker.invokeInt(callContext, function, invocationBuffer));

            case SSHORT:
                return Util.newSigned16(runtime, (short) invoker.invokeInt(callContext, function, invocationBuffer));
                
            case USHORT:
                return Util.newUnsigned16(runtime, (short) invoker.invokeInt(callContext, function, invocationBuffer));
                
            case SINT:
                return Util.newSigned32(runtime, invoker.invokeInt(callContext, function, invocationBuffer));
                
            case UINT:
                return Util.newUnsigned32(runtime, invoker.invokeInt(callContext, function, invocationBuffer));
                
            case SLONG_LONG:
                return Util.newSigned64(runtime, invoker.invokeLong(callContext, function, invocationBuffer));
                
            case ULONG_LONG:
                return Util.newUnsigned64(runtime, invoker.invokeLong(callContext, function, invocationBuffer));
                
            case FLOAT:
                return runtime.newFloat(invoker.invokeFloat(callContext, function, invocationBuffer));
                
            case DOUBLE:
                return runtime.newFloat(invoker.invokeDouble(callContext, function, invocationBuffer));
                
            case VOID:
                invoker.invokeInt(callContext, function, invocationBuffer);
                return runtime.getNil();

            case ADDRESS:
                long address = invoker.invokeAddress(callContext, function, invocationBuffer);
                return address != 0L ? new Pointer(runtime, address) : runtime.getNil();

            case CSTRING:
                long stringAddress = invoker.invokeAddress(callContext, function, invocationBuffer);
                return stringAddress != 0L 
                        ? runtime.newString(new ByteList(MemoryIO.getInstance().getZeroTerminatedByteArray(stringAddress)))
                        : runtime.getNil();

            default:
                throw runtime.newRuntimeError("invalid result type: " + nativeType);
        }
    }

    static void marshal(Ruby runtime, HeapInvocationBuffer invocationBuffer, NativeType nativeType, Object parameter) {
        switch (nativeType) {
            case SCHAR:
                invocationBuffer.putByte(Util.int8Value(parameter));
                break;

            case UCHAR:
                invocationBuffer.putByte(Util.uint8Value(parameter));
                break;

            case SSHORT:
                invocationBuffer.putShort(Util.int16Value(parameter));
                break;
            
            case USHORT:
                invocationBuffer.putShort(Util.uint16Value(parameter));
                break;

            case SINT:
                invocationBuffer.putInt(Util.int32Value(parameter));
                break;
            
            case UINT:
                invocationBuffer.putInt((int) Util.uint32Value(parameter));
                break;

            case SLONG_LONG:
                invocationBuffer.putLong(Util.int64Value(parameter));
                break;
            
            case ULONG_LONG:
                invocationBuffer.putLong(Util.uint64Value(parameter));
                break;

            case FLOAT:
                invocationBuffer.putFloat((float) Util.doubleValue(parameter));
                break;

            case DOUBLE:
                invocationBuffer.putDouble(Util.doubleValue(parameter));
                break;

            case BOOL:
                invocationBuffer.putInt(((IRubyObject) parameter).isTrue() ? 1 : 0);
                break;

            case ADDRESS:
                if (parameter instanceof MemoryObject) {
                    invocationBuffer.putAddress(((MemoryObject) parameter).address());
                
                } else if (parameter == null || parameter instanceof RubyNil) {
                    invocationBuffer.putAddress(0L);
                
                } else {
                    throw runtime.newTypeError("invalid parameter value");
                }

                break;

            case CSTRING:
                RubyString str = parameter instanceof RubyString ? (RubyString) parameter : ((IRubyObject) parameter).convertToString();
                ByteList bl = str.getByteList();
                invocationBuffer.putArray(bl.getUnsafeBytes(), bl.begin(), bl.length(), ArrayFlags.IN | ArrayFlags.NULTERMINATE);
                break;
        }
    }
    
    private static void addPostInvoke(InvocationSession session, final ToNativeConverter.PostInvocation postInvoke,
                                      final IRubyObject rbValue, final Object nativeValue) {
        session.addPostInvoke(new InvocationSession.PostInvoke() {
            @Override
            public void postInvoke() {
                postInvoke.postInvoke(rbValue, nativeValue, null);
            }
        });
    }
}
