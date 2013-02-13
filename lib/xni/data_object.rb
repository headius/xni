require 'xni/util'

if defined?(JRUBY_VERSION) && JRUBY_VERSION >= "1.7.0"
  require 'xni/jruby/data_object'
else
  require 'xni/cruby/data_object'
end

module XNI
  ALLOWED_DATA_TYPES = [
      :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong, :long_long, :ulong_long,
      :float, :double, :pointer
  ]
  
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
        fields.each_slice(2) do |name, type| 
          raise TypeError.new("unsupported data type, #{type} for field #{name}") unless ALLOWED_DATA_TYPES.include?(type)
        end
        @__xni_fields__ = Hash[*fields]
        __xni_data_fields__ *fields
      end

      def data_reader(*fields)
        __xni_data_reader__ *fields
      end

      def data_accessor(*fields)
        __xni_data_accessor__ *fields
      end
      
      public
      def __xni_fields__
        @__xni_fields__ ||= (superclass < DataObject) ? superclass.__xni_fields__ : {}
      end
    end
    
    def inspect
      fields_string = '' 
      unless (fields = self.class.__xni_fields__).empty?
        fields_string << ' ' << fields.each_key.map { |name| name.to_s + '=' + self.send(name).inspect }.join(', ')
      end

      "#<#{self.class}:#{__xni_struct__.pointer.address.to_s(16)}#{fields_string}>"
    end
    
  end
end