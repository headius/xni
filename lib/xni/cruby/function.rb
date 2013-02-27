module XNI
  class Function
    attr_reader :address, :result_type, :parameter_types

    def initialize(address, result_type, parameter_types)
      @address = address
      @result_type = result_type
      @parameter_types = parameter_types.dup.freeze
    end
  end
end