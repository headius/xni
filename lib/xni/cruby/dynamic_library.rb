require 'ffi'

module XNI
  class DynamicLibrary
    RTLD_LAZY = FFI::DynamicLibrary::RTLD_LAZY
    RTLD_NOW = FFI::DynamicLibrary::RTLD_NOW
    RTLD_GLOBAL = FFI::DynamicLibrary::RTLD_GLOBAL
    RTLD_LOCAL = FFI::DynamicLibrary::RTLD_LOCAL
    
    
    def self.open(path, flags)
      self.new(FFI::DynamicLibrary.open(path, flags))
    end
    
    def initialize(handle)
      @library = handle
    end
    
    def find_function(name)
      address = @library.find_function(name)
      return nil if address.nil? || address.null?
      Symbol.new(name.to_sym, address)
    end
    
    class Symbol
      attr_reader :name
      
      def initialize(name, address)
        @name = name
        @address = address
      end
      
      def address
        @address.address
      end
      
      def ffi_pointer
        @address
      end
    end
  end
end