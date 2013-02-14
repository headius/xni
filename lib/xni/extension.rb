require 'xni/util'

if defined?(JRUBY_VERSION) && JRUBY_VERSION >= "1.7.0"
  require 'xni/jruby/extension'
else
  require 'xni/cruby/extension'
  require 'xni/cruby/extension_data'
end
require 'xni/pointer'

module XNI
  module Extension
    def self.extended(mod)
      class << mod
        attr_reader :__ffi__, :__xni__
      end

      ffi = Module.new do |c|
        c.extend FFI::Library
        typedef Pointer, :pointer
        typedef FFI::Type.const_get(:TRANSIENT_STRING), :string if FFI::Type.const_defined?(:TRANSIENT_STRING)
      end

      mod.instance_variable_set :@__ffi__, ffi 
      mod.instance_variable_set :@__xni__, mod
    end

    def extension(name)
      __ffi__.ffi_lib XNI::Loader.find(name, caller[0].split(/:/)[0])

      load_address = __ffi__.ffi_libraries.first.find_symbol("xni_#{Util.module_cname(self)}_load")
      if load_address && !load_address.null?
        ref = FFI::MemoryPointer.new(:pointer)
        result = FFI::Function.new(:int, [ :pointer, :pointer  ], load_address).call(nil, ref)
        raise "extension #{name} failed to load" unless result.zero?
        
        __xni_ext_data__ ExtensionData.new(ref.read_pointer)
      else
        __xni_ext_data__ ExtensionData.new(FFI::Pointer::NULL)
      end
    end

    def native(fn, params, rtype)
      __xni_define_method__ fn.to_s, Util.module_stub(self, fn, params, rtype), params.length
    end
  end
end