#
# These are utilities for internal XNI use - do NOT use in general code.
#

require 'ffi'
require 'xni/pointer'

module XNI
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
  
  TypeMap = {
      :char => FFI::Type::CHAR, 
      :uchar => FFI::Type::UCHAR, 
      :short => FFI::Type::SHORT, 
      :ushort => FFI::Type::USHORT, 
      :int => FFI::Type::INT, 
      :uint => FFI::Type::UINT, 
      :long => FFI::Type::LONG, 
      :ulong => FFI::Type::ULONG, 
      :long_long => FFI::Type::LONG_LONG, 
      :ulong_long => FFI::Type::ULONG_LONG,
      :float => FFI::Type::FLOAT, 
      :double => FFI::Type::DOUBLE, 
      :pointer => FFI::Type::Mapped.new(Pointer), 
      :cstring  => FFI::Type.const_defined?(:TRANSIENT_STRING) ? FFI::Type.const_get(:TRANSIENT_STRING) : FFI::Type::Mapped.new(TransientString),
      :void => FFI::Type::VOID,
      :bool => FFI::Type::BOOL 
  }.freeze
  
  ALLOWED_RESULT_TYPES = [
      :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong, :long_long, :ulong_long,
      :float, :double, :pointer, :cstring, :void, :bool
  ]

  ALLOWED_PARAMETER_TYPES = [
      :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong, :long_long, :ulong_long,
      :float, :double, :pointer, :cstring, :bool
  ]

  module Util
    def self.extension(mod)
      begin
        mod = mod.name.split("::")[0..-2].inject(Object) { |obj, c| obj.const_get(c) }
        mod.is_a?(Extension) ? mod : nil
      rescue Exception
        nil
      end
    end
    
    def self.module_cname(mod)
      mod.to_s.gsub('::', '_').downcase
    end
    
    def self.stub_cname(mod, function)
      'xni_' + module_cname(mod) + '_' + function.to_s.sub(/\?$/, '_p')
    end
    
    def self.stub_address(mod, name)
      cname = Util.stub_cname(mod, name)
      address = mod.__xni__.__ffi__.ffi_libraries.first.find_symbol(cname)
      raise RuntimeError.new("cannot find symbol #{cname}") if address.nil? || address.null?
      address
    end
    
    def self.result_type(type)
      if ALLOWED_RESULT_TYPES.include?(type) && TypeMap.has_key?(type)
        return TypeMap[type]
      end

      raise TypeError.new("unsupported result type, #{type}")
    end
    
    def self.param_type(type)
      if ALLOWED_PARAMETER_TYPES.include?(type) && TypeMap.has_key?(type)
        return TypeMap[type]
        
      elsif type.is_a?(Class) && type < DataObject
        return FFI::Type::Mapped.new(type)
      end
      
      raise TypeError.new("unsupported parameter type, #{type}")
    end
    
    def self.module_stub(mod, name, params, rtype, options = {})
      options = { :save_errno => false }.merge(options)
      ffi_params = [ FFI::Type::POINTER ] + params.map { |t| param_type(t) }
      FFI::Function.new(result_type(rtype), ffi_params, stub_address(mod, name), options)
    end

    def self.instance_stub(mod, name, params, rtype, options = {})
      options = { :save_errno => false }.merge(options)
      ffi_params = [ FFI::Type::POINTER, FFI::Type::POINTER ] + params.map { |t| param_type(t) }
      FFI::Function.new(result_type(rtype), ffi_params, stub_address(mod, name), options)
    end
  end
end