require 'xni'

module Primitive
  extend XNI::Extension
  extension 'primitive'

  native :ret_char, [ :char ], :char
  native :ret_uchar, [ :uchar ], :uchar
  native :ret_short, [ :short ], :short
  native :ret_ushort, [ :ushort ], :ushort
  native :ret_int, [ :int ], :int
  native :ret_uint, [ :uint ], :uint
  native :ret_long_long, [ :long_long ], :long_long
  native :ret_ulong_long, [ :ulong_long ], :ulong_long
  
  [ :char, :uchar, :short, :ushort, :int, :uint, :long_long, :ulong_long ].each do |type|
    (2..6).each do |arity|
      params = (0...arity).map { |n| type } 
      native "xor_#{type}_#{arity}".to_sym, params, type
    end
  end
end