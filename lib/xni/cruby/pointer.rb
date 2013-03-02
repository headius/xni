require 'ffi'

module XNI
  class Pointer
    extend FFI::DataConverter
    native_type FFI::Type::POINTER
    attr_reader :ffi_pointer

    def self.to_native(value, ctx)
      value ? value.ffi_pointer : FFI::Pointer::NULL
    end

    def self.from_native(value, ctx)
      self.new(value)
    end

    def initialize(ffi_pointer)
      @ffi_pointer = ffi_pointer
    end
    
    def address
      ffi_pointer.address
    end
  end
end