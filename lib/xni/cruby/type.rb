require 'xni/cruby/pointer'

module XNI
  class Type
    module TransientString
      extend FFI::DataConverter
      native_type FFI::Type::POINTER
      def self.to_native(value, ctx)
        value ? FFI::MemoryPointer.from_string(value) : FFI::Pointer::NULL
      end

      def self.from_native(value, ctx)
        value.null? ? nil : value.get_string(0)
      end
    end


    def initialize(ffi_type)
      @ffi_type = ffi_type
    end

    attr_reader :ffi_type
    
    SCHAR = Type.new(FFI::Type::SCHAR)
    UCHAR = Type.new(FFI::Type::UCHAR)
    SSHORT = Type.new(FFI::Type::SSHORT)
    USHORT = Type.new(FFI::Type::USHORT)
    SINT = Type.new(FFI::Type::SINT)
    UINT = Type.new(FFI::Type::UINT)
    SLONG = Type.new(FFI::Type::SLONG)
    ULONG = Type.new(FFI::Type::ULONG)
    SLONG_LONG = Type.new(FFI::Type::SLONG_LONG)
    ULONG_LONG = Type.new(FFI::Type::ULONG_LONG)
    FLOAT = Type.new(FFI::Type::FLOAT)
    DOUBLE = Type.new(FFI::Type::DOUBLE)
    VOID = Type.new(FFI::Type::VOID)
    POINTER = Type.new(FFI::Type::Mapped.new(Pointer))
    CSTRING = Type.new(FFI::Type::Mapped.new(TransientString))
    BOOL = Type.new(FFI::Type::BOOL)
        
    class DataObject < Type
      def initialize(object_class)
        super(FFI::Type::Mapped.new(object_class))
      end
    end
    
    class CArray < Type
      attr_reader :component_type, :length
      
      def initialize(component_type, length)
        @component_type = component_type
        @length = length
      end
    end
  end
end