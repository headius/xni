module XNI
  module Types
    ALLOWED_TYPES = [
        :fixnum, :double,
        :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong,
        :long_long, :ulong_long, :float, :bool, :string, :pointer
    ].freeze
  end
end