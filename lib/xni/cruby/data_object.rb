require 'ffi'
require 'xni/loader'
require 'xni/util'
require 'xni/cruby/data_object_factory'
require 'xni/cruby/auto_release_pool'

module XNI
  class DataObject
    attr_reader :__xni_struct__
    
    def self.inherited(klass)
      class << klass
        attr_reader :__xni__, :__ffi__, :__xni_factory__
      end

      klass.singleton_class.define_singleton_method :native do |fn, params, rtype, options = {}|
        klass.__xni_define_singleton_method__ fn.to_s, Util.singleton_stub(klass, fn, params, rtype, options)
      end
      
      klass.extend FFI::DataConverter
      klass.native_type FFI::Type::POINTER

      def klass.to_native(value, ctx = nil)
        raise RuntimeError.new("object has been released") if value.__xni_struct__ == :released
        value.__xni_struct__.pointer
      end
      
      def klass.from_native(value, ctx = nil)
        raise RuntimeError.new("cannot convert native #{value} to instance of #{self}")
      end
      
      def klass.new(*args, &b)
        __xni_factory__.new(*args, &b)
      end

      if (extension = Util.extension(klass)) && self == DataObject
        klass.instance_variable_set(:@__xni__, extension)
        
        cname = "xni_#{klass.to_s.split('::')[0..-2].join('_')}_sizeof_#{klass.to_s.split('::')[-1]}".downcase
        address = extension.__xni_library__.find_function(cname)
        if address
          klass.instance_variable_set :@__xni_size__, size = FFI::Function.new(:int, [], address.ffi_pointer, :save_errno => false).call
          klass.instance_variable_set :@__xni_factory__,  Factory.new(klass, size)
        end
      end
      
      if self != DataObject
        klass.instance_variable_set(:@__ffi__, self.instance_variable_get(:@__ffi__))
        klass.instance_variable_set(:@__xni__, self.instance_variable_get(:@__xni__))
        klass.instance_variable_set(:@__xni_factory__, self.instance_variable_get(:@__xni_factory__))
      end
    end
    
    def self.autorelease
      __xni_factory__.autorelease
    end
    
    def self.retained
      __xni_factory__
    end
    
    def self.__xni_finalizer__(function)
      @__xni_finalizer__ = FFI::Function.new(function.result_type.ffi_type, [ FFI::Type::POINTER, FFI::Type::POINTER  ] + function.parameter_types.map(&:ffi_type), function.address.ffi_pointer)
      @__xni_factory__.finalizer = @__xni_finalizer__ if defined?(@__xni_factory__)
    end

    def self.__xni_define_method__(name, function)
      cname = Util.stub_cname(self, name)
      FFI::Function.new(function.result_type.ffi_type, [ FFI::Type::POINTER, FFI::Type::POINTER  ] + function.parameter_types.map(&:ffi_type), function.address.ffi_pointer).attach(__xni__.__ffi__, cname)
      
      rb_params = (0...function.parameter_types.length).map { |i| "a#{i}" }
      c_params = %w(self.class.__xni__.__xni_ext_data__ __xni_struct__) + rb_params
      
      class_eval <<-EVAL
        def #{name.to_s}(#{rb_params.join(', ')})
          self.class.__xni__.__ffi__.#{cname}(#{c_params.join(', ')})
        end
      EVAL
    end

    def self.__xni_define_singleton_method__(name, function)
      cname = Util.stub_cname(self, 's_' + name.to_s)
      FFI::Function.new(function.result_type.ffi_type, [ FFI::Type::POINTER  ] + function.parameter_types.map(&:ffi_type), function.address.ffi_pointer).attach(__xni__.__ffi__, cname)
    
      rb_params = (0...function.parameter_types.length).map { |i| "a#{i}" }
      c_params = %w(__xni__.__xni_ext_data__) + rb_params
    
      instance_eval <<-EVAL
        def #{name.to_s}(#{rb_params.join(', ')})
          __xni__.__ffi__.#{cname}(#{c_params.join(', ')})
        end
      EVAL
    end
    
    def self.__xni_data_fields__(*fields)
      raise RuntimeError.new("data fields already specified") if defined?(@__xni_struct_class__)
      fields = fields.each_slice(2).map { |f| [ f[0], f[1].ffi_type ] }.flatten
      @__xni_struct_class__ = Class.new(FFI::Struct) { |c| c.layout *fields }
      @__xni_factory__ = Factory.new(self, @__xni_struct_class__, defined?(@__xni_finalizer__) ? @__xni_finalizer__ : nil)
    end
    
    def self.__xni_data_accessors__(field_names, read, write)
      field_names.each do |f|
        raise TypeError.new("wrong type for '#{f}' (expected Symbol)") unless f.is_a?(Symbol)
        raise RuntimeError.new("invalid field: '#{f}'") unless @__xni_struct_class__.layout.members.include?(f)
        if read
          class_eval <<-EVAL
            def #{f.to_s}
              self.__xni_struct__[:#{f}]
            end
          EVAL

        end
        if write
          class_eval <<-EVAL
            def #{f.to_s}=(value)
              self.__xni_struct__[:#{f}] = value
            end
          EVAL

        end
      end
    end

    def self.__xni_data_reader__(*field_names)
      __xni_data_accessors__(field_names, true, false)
    end

    def self.__xni_data_accessor__(*field_names)
      __xni_data_accessors__(field_names, true, true)
    end
    
    def self.__xni_lifecycle__(how)
      
    end

    def autorelease
      AutoReleasePool.active.add(self)
    end
    
    def retain
      AutoReleasePool.active.remove(self)
    end
    
    def __xni_release__
      if @__xni_struct__ != :released
        struct = @__xni_struct__
        @__xni_struct__ = :released
        @__xni_finalizer__.call(self.class.__xni__.__xni_ext_data__, struct) unless @__xni_finalizer__.nil?
      end
    end
    
    def __xni_address__
      __xni_struct__.pointer.address
    end
  end
end