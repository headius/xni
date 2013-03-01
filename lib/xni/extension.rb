require 'xni/util'

if defined?(JRUBY_VERSION) && JRUBY_VERSION >= "1.7.0"
  require 'xni/jruby/extension'
else
  require 'xni/cruby/type'
  require 'xni/cruby/extension'
  require 'xni/cruby/extension_data'
  require 'xni/cruby/dynamic_library'
end
require 'xni/pointer'
require 'xni/types'
require 'ffi'

module XNI
  module Extension
    def self.extended(mod)
      class << mod
        attr_reader :__ffi__, :__xni__, :__xni_library__
      end

      ffi = Module.new do |c|
        c.extend FFI::Library
      end

      mod.instance_variable_set :@__ffi__, ffi 
      mod.instance_variable_set :@__xni__, mod
    end

    def extension(name)
      @__xni_library__ = library = XNI::DynamicLibrary.open(XNI::Loader.find(name, caller[0].split(/:/)[0]), XNI::DynamicLibrary::RTLD_LOCAL | XNI::DynamicLibrary::RTLD_LAZY)
      
      load_address = library.find_function("xni_#{Util.module_cname(self)}_load")
      unload_address = library.find_function("xni_#{Util.module_cname(self)}_unload")
      __xni_ext_data__ ExtensionData.new(load_address, unload_address)
    end

    def native(fn, params, rtype)
      __xni_define_method__ fn.to_s, Util.module_stub(self, fn, params, rtype)
    end

    def carray(type, direction)
      XNI.carray(type, direction)
    end
  end
end