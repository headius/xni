if !defined?(JRUBY_VERSION) || JRUBY_VERSION < "1.7.0"
  require 'xni/ffi-impl/pointer'
end

module XNI
  class Pointer
    def to_s
      "#<#{self.class}:address=#{address.to_s(16)}>"
    end
  end
end