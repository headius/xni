module XNI
  class ExtensionData
    def initialize(load_address, unload_address)
      if load_address
        ref = FFI::MemoryPointer.new(:pointer)
        result = FFI::Function.new(:int, [ :pointer, :pointer  ], FFI::Pointer.new(load_address.ffi_pointer)).call(nil, ref)
        raise "#{load_address.name} failed with error code #{result}" if result < 0
        @ptr = ref.read_pointer
      end
    end

    def to_ptr
      @ptr
    end
  end
end