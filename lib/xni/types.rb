module XNI
  module Types
    ALLOWED_TYPES = [
        :char, :uchar, :short, :ushort, :int, :uint, :long, :ulong,
        :long_long, :ulong_long, :float, :double, :bool, :string, :pointer
    ].freeze
  end
end