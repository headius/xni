require File.join(XNI::IMPL_DIR, File.basename(__FILE__))
require 'xni/util'
require 'xni/pointer'
require 'xni/types'

module XNI
  module Extension
    def self.extended(mod)
      class << mod
        attr_reader :__xni__, :__xni_library__
      end
 
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
