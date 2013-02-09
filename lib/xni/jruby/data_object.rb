
require 'xni/loader'
require 'xni/util'
require 'xni/xni_ext'

module XNI
  class DataObject
    class << self
      alias_method :__orig_inherited, :inherited
    end
    
    def self.inherited(klass)
      class << klass
        attr_reader :__xni__
        # Switch back to default inherited() method for subclasses of this new class
        alias_method :inherited, :__orig_inherited
      end

      if extension = Util.extension(klass)
        klass.instance_variable_set(:@__xni__, extension)
        cname = "xni_#{klass.to_s.split('::')[0..-2].join('_')}_sizeof_#{klass.to_s.split('::')[-1]}".downcase
        address = extension.__ffi__.ffi_libraries.first.find_symbol(cname)
        if address && !address.null?
          klass.instance_variable_set :@__xni_size__, size = FFI::Function.new(:int, [], address, :save_errno => false).call
        end
      end
    end       
  end
end