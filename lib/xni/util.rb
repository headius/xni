#
# These are utilities for internal XNI use - do NOT use in general code.
#

module XNI
  
  TypeMap = {
      :char => XNI::Type::SCHAR, 
      :uchar => XNI::Type::UCHAR, 
      :short => XNI::Type::SSHORT, 
      :ushort => XNI::Type::USHORT, 
      :int => XNI::Type::SINT, 
      :uint => XNI::Type::UINT, 
      :long_long => XNI::Type::SLONG_LONG, 
      :ulong_long => XNI::Type::ULONG_LONG,
      :fixnum => XNI::Type::SLONG_LONG,
      :float => XNI::Type::FLOAT, 
      :double => XNI::Type::DOUBLE, 
      :pointer => XNI::Type::POINTER, 
      :cstring  => XNI::Type::CSTRING,
      :void => XNI::Type::VOID,
      :bool => XNI::Type::BOOL 
  }.freeze
  
  ALLOWED_RESULT_TYPES = [
      :void, :fixnum, :double, :pointer, :cstring, :bool,
      :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong, :long_long, :ulong_long, :float
  ]

  ALLOWED_PARAMETER_TYPES = [
      :fixnum, :double, :pointer, :cstring, :bool,
      :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong, :long_long, :ulong_long, :float, 
  ]
  
  DEPRECATED_TYPES = [
      :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong, :long_long, :ulong_long, :float
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
      'xni_' + module_cname(mod) + '_' + __mangle_function_name__(function)
    end

    def self.__mangle_function_name__(name)
      mangled = name.to_s.dup

      if mangled =~ /([A-Za-z0-9]+)=$/
        mangled = 'set_' + $1

      elsif mangled =~ /\?$/
        mangled.sub!(/\?$/, '_p')
      end

      mangled
    end
    
    def self.stub_address(mod, name)
      cname = Util.stub_cname(mod, name)
      address = mod.__xni__.__xni_library__.find_function(cname)
      raise RuntimeError.new("cannot find symbol #{cname}") unless address
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

      elsif type.is_a?(Type)
        return type
      
      elsif type.is_a?(Class) && type < DataObject
        return XNI::Type::DataObject.new(type)
      end
      
      raise TypeError.new("unsupported parameter type, #{type}")
    end
    
    def self.module_stub(mod, name, params, rtype, options = {})
      XNI::Function.new(stub_address(mod, name), result_type(rtype), params.map { |t| param_type(t) })
    end

    def self.singleton_stub(mod, name, params, rtype, options = {})
      XNI::Function.new(stub_address(mod, 's_' + name.to_s), result_type(rtype), params.map { |t| param_type(t) })
    end

    def self.instance_stub(mod, name, params, rtype, options = {})
      XNI::Function.new(stub_address(mod, name), result_type(rtype), params.map { |t| param_type(t) })
    end
  end
end