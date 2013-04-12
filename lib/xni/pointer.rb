require File.join(XNI::IMPL_DIR, File.basename(__FILE__))

module XNI
  class Pointer
    def to_s
      "#<#{self.class}:address=#{address.to_s(16)}>"
    end
  end
end