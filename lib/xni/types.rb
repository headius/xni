module XNI
  module Types
    ALLOWED_TYPES = [
        :fixnum, :double,
        :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong,
        :long_long, :ulong_long, :float, :bool, :string, :pointer
    ].freeze
    
    
  end
  
  CARRAY_DIRECTIONS = { 
      :in => Type::CArray::IN, 
      :out => Type::CArray::OUT, 
      :inout => Type::CArray::IN | Type::CArray::OUT 
  }

  CARRAY_TYPES = [ :bool, :fixnum, :double ]

  def self.carray(type, direction = :in)
    raise ArgumentError.new("#{type} not allowed") unless CARRAY_TYPES.include?(type)
    raise ArgumentError.new("invalid direction, #{direction}") unless CARRAY_DIRECTIONS.has_key?(direction)

    Type::CArray.new(XNI::TypeMap[type], 0, CARRAY_DIRECTIONS[direction])
  end
end