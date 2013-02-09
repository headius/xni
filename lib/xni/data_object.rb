if defined?(JRUBY_VERSION) && JRUBY_VERSION >= "1.7.0"
  require 'xni/jruby/data_object'
else
  require 'xni/cruby/data_object'
end

module XNI
  def self.autoreleasepool(&b)
    AutoReleasePool.new &b
  end

  class DataObject
    class << self
      protected
      def custom_finalizer(name = :finalize)
        __xni_finalizer__ Util.instance_stub(self, name, [], :void)
      end

      def native(fn, params, rtype, options = {})
        __xni_define_method__ fn.to_s, Util.instance_stub(self, fn, params, rtype, options), params.length
      end

      def data(*fields)
        __xni_data_fields__ *fields
      end

      def data_reader(*fields)
        __xni_data_reader__ *fields
      end

      def data_accessor(*fields)
        __xni_data_accessor__ *fields
      end
    end
  end
end