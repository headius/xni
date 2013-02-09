#
# These are utilities for internal XNI use - do NOT use in general code.
#
module XNI
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
      '__xni_' + module_cname(mod) + '_' + function.to_s.sub(/\?$/, '_p')
    end
    
    def self.stub_address(mod, name)
      cname = Util.stub_cname(mod, name)
      address = mod.__xni__.__ffi__.ffi_libraries.first.find_symbol(cname)
      raise RuntimeError.new("cannot find symbol #{cname}") if address.nil? || address.null?
      address
    end
    
    def self.module_stub(mod, name, params, rtype, options = {})
      ffi_params = ([ FFI::Type::POINTER ] + params).map { |t| mod.__xni__.__ffi__.find_type(t) }
      options = { :save_errno => false } .merge(options)
      FFI::Function.new(mod.__xni__.__ffi__.find_type(rtype), ffi_params, stub_address(mod, name), options)
    end

    def self.instance_stub(mod, name, params, rtype, options = {})
      ffi_params = ([ FFI::Type::POINTER, FFI::Type::POINTER ] + params).map { |t| mod.__xni__.__ffi__.find_type(t) }
      options = { :save_errno => false } .merge(options)
      FFI::Function.new(mod.__xni__.__ffi__.find_type(rtype), ffi_params, stub_address(mod, name), options)
    end
  end
end