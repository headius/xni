package xni;

import com.kenai.jffi.ObjectParameterInfo;
import com.kenai.jffi.ObjectParameterStrategy;
import org.jruby.*;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.IRubyObject;
import xni.converters.DataObjectParameterConverter;
import xni.converters.DoubleArrayParameterConverter;
import xni.converters.Signed32ArrayParameterConverter;
import xni.converters.Signed64ArrayParameterConverter;

/**
 *
 */
public class Function extends RubyObject {
    private final DynamicLibrary.Symbol address;
    private final Type resultType;
    private final Type[] parameterTypes;
    private final ToNativeConverter[] toNativeConverters;
    private final ObjectParameterStrategy[] objectParameterStrategies;
    private final ObjectParameterInfo[] objectParameterInfo;
    
    public Function(Ruby runtime, RubyClass metaClass, DynamicLibrary.Symbol address, Type resultType, Type[] parameterTypes) {
        super(runtime, metaClass);
        this.address = address;
        this.resultType = resultType;
        this.parameterTypes = parameterTypes.clone();
        this.objectParameterInfo = new ObjectParameterInfo[parameterTypes.length];
        this.toNativeConverters = new ToNativeConverter[parameterTypes.length];
        this.objectParameterStrategies = new ObjectParameterStrategy[parameterTypes.length];
        
        for (int i = 0; i < parameterTypes.length; i++) {
            objectParameterInfo[i] = getObjectParameterInfo(parameterTypes[i], i);
            objectParameterStrategies[i] = PrimitiveArrayParameterStrategy.getObjectParameterStrategy(parameterTypes[i]);
            toNativeConverters[i] = getToNativeConverter(parameterTypes[i]);
        }
    }
    
    public static RubyClass createFunctionClass(Ruby runtime, RubyModule xniModule) {
        RubyClass functionClass = xniModule.defineClassUnder("Function", runtime.getObject(),
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        functionClass.defineAnnotatedMethods(Function.class);
        functionClass.defineAnnotatedConstants(Function.class);
        
        return functionClass;
    }

    @JRubyMethod(name = "new", required = 3, meta = true)
    public static final IRubyObject newInstance(ThreadContext context, IRubyObject klass, 
                                                IRubyObject address, IRubyObject resultType, IRubyObject rbParameterTypes) {
        RubyArray rbParameterArray = rbParameterTypes.convertToArray();
        Type[] parameterTypes = new Type[rbParameterArray.getLength()];
        for (int i = 0; i < parameterTypes.length; i++) {
            parameterTypes[i] = (Type) rbParameterArray.aref(context.getRuntime().newFixnum(i));
        }

        return new Function(context.getRuntime(), (RubyClass) klass, (DynamicLibrary.Symbol) address, (Type) resultType,
                parameterTypes);
    }

    public Type getResultType() {
        return resultType;
    }

    public int getParameterCount() {
        return parameterTypes.length;
    }
    
    public Type getParameterType(int idx) {
        return parameterTypes[idx];
    }
    
    public ToNativeConverter getToNativeConverter(int idx) {
        return toNativeConverters[idx];
    }
    
    public ObjectParameterStrategy getObjectParameterStrategy(int idx) {
        return objectParameterStrategies[idx];
    }
    
    public ObjectParameterInfo getObjectParameterInfo(int idx) {
        return objectParameterInfo[idx];
    }
    
    
    final long address() {
        return address.address();
    }
    
    private static ObjectParameterInfo getObjectParameterInfo(Type type, int parameterIndex) {
        if (type instanceof Type.CArray) {
            Type.CArray arrayType = (Type.CArray) type;
            return ObjectParameterInfo.create(parameterIndex, ObjectParameterInfo.ARRAY, 
                    getObjectInfoComponentType(arrayType.getComponentType().getNativeType()), arrayType.flags());
        }

        return null;
    }
    
    private static ToNativeConverter getToNativeConverter(Type type) {
        if (type instanceof Type.DataObject) {
            return new DataObjectParameterConverter((Type.DataObject) type);

        } else if (type instanceof Type.CArray) {
            Type.CArray arrayType = (Type.CArray) type;
            switch (arrayType.getComponentType().getNativeType()) {
                case SINT:
                    return Signed32ArrayParameterConverter.getInstance(arrayType.flags());
                
                case SLONG_LONG:
                    return Signed64ArrayParameterConverter.getInstance(arrayType.flags());

                case DOUBLE:
                    return DoubleArrayParameterConverter.getInstance(arrayType.flags());
                
            }
        }
        return null;
    }
    
    private static ObjectParameterInfo.ComponentType getObjectInfoComponentType(NativeType nativeType) {
        switch (nativeType) {
            case SCHAR:
            case UCHAR:
                return ObjectParameterInfo.ComponentType.BYTE;

            case SSHORT:
            case USHORT:
                return ObjectParameterInfo.ComponentType.SHORT;

            case SINT:
            case UINT:
                return ObjectParameterInfo.ComponentType.INT;
            
            case SLONG_LONG:
            case ULONG_LONG:
                return ObjectParameterInfo.ComponentType.LONG;

            case FLOAT:
                return ObjectParameterInfo.ComponentType.FLOAT;
            
            case DOUBLE:
                return ObjectParameterInfo.ComponentType.DOUBLE;
            
            default:
                throw new RuntimeException("unsupported array component type: " + nativeType);
        }
    }
}
