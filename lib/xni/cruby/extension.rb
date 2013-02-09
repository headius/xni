require 'xni/cruby/extension_data'

module XNI
  module Extension
    def __xni_ext_data__(data)
      class << self
        attr_reader :__xni_ext_data__
      end
      @__xni_ext_data__ = data
    end

    def __xni_define_method__(name, function, param_count)
      cname = Util.stub_cname(self, name)
      function.attach(__ffi__, cname)
      rb_params = (0...param_count).map { |i| "a#{i}" }
      c_params = %w(self.__xni_ext_data__) + rb_params

      instance_eval <<-EVAL
        def #{name.to_s}(#{rb_params.join(', ')})
          self.__ffi__.#{cname}(#{c_params.join(', ')})
        end
      EVAL
    end
  end
end