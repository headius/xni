require 'xni/loader'
require 'xni/util'

module XNI
  class DataObject
    attr_reader :__xni_struct__
    
    def self.inherited(klass)
      class << klass
        attr_reader :__xni__
      end

      klass.singleton_class.define_singleton_method :native do |fn, params, rtype, options = {}|
        klass.__xni_define_singleton_method__ fn.to_s, Util.singleton_stub(klass, fn, params, rtype, options)
      end
      
      if (extension = Util.extension(klass)) && self == DataObject
        klass.instance_variable_set(:@__xni__, extension)
        
        cname = "xni_#{klass.to_s.split('::')[0..-2].join('_')}_sizeof_#{klass.to_s.split('::')[-1]}".downcase
        if address = extension.__xni_library__.find_function(cname)
            klass.__xni_set_size__ address
        end
      end
      
      if self != DataObject
        klass.instance_variable_set(:@__xni__, self.instance_variable_get(:@__xni__))
        klass.instance_variable_set(:@autorelease, self.instance_variable_get(:@autorelease))
        klass.instance_variable_set(:@retained, self.instance_variable_get(:@retained))
      end
    end
  end
end