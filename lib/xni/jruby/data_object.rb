
require 'xni/loader'
require 'xni/util'
require 'xni/xni_ext'

module XNI
  class DataObject
    def self.inherited(klass)
      class << klass
        attr_reader :__xni__
      end

      if (extension = Util.extension(klass)) && self == DataObject
        klass.instance_variable_set(:@__xni__, extension)
        cname = "xni_#{klass.to_s.split('::')[0..-2].join('_')}_sizeof_#{klass.to_s.split('::')[-1]}".downcase
        if address = extension.__xni__.__xni_library__.find_function(cname)
          __xni_set_size__ XNI::Function.new(address, XNI::Type::SINT, [])
        end
      end
    end       
  end
end