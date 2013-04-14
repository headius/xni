require 'xni'

module DataObject
  extend XNI::Extension
  extension 'data_object'

  class FooBar < XNI::DataObject
    data :m_foo, :fixnum,
         :m_bar, :fixnum

    data_reader :m_bar
    data_accessor :m_foo

    native :initialize, [ :fixnum, :fixnum ], :void
    native :foo, [], :fixnum
    native :bar, [], :fixnum
    native :foo=, [ :fixnum ], :fixnum
    native :bar=, [ :fixnum ], :fixnum
  end
  
  class XOR < XNI::DataObject

    class << self
      [ :char, :uchar, :short, :ushort, :int, :uint, :long_long, :ulong_long ].each do |type|
        (2..6).each do |arity|
          params = (0...arity).map { |n| type }
          native "xor_#{type}_#{arity}".to_sym, params, type
        end
      end
    end


    [ :char, :uchar, :short, :ushort, :int, :uint, :long_long, :ulong_long ].each do |type|
      (2..6).each do |arity|
        params = (0...arity).map { |n| type }
        native "xor_#{type}_#{arity}".to_sym, params, type
      end
    end
  end
end