module XNI
  class ExtensionData
    def initialize(ptr)
      @ptr = ptr
    end

    def to_ptr
      @ptr
    end
  end
end