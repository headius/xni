

describe 'primitives' do
  before :all do
    system("cd #{File.dirname(__FILE__)}/ext && rake -I../../../../lib")
    require File.join(File.dirname(__FILE__), 'lib', 'primitive')
  end

  def iterate(arity, values)
    idx = (0...arity).map { 0 }
    (values.length**arity).times do
      yield idx.map { |i| values[i] } if block_given?
      idx.each_with_index do |v, i|
        if (idx[i] += 1) >= values.length
          idx[i] = 0
        else
          break
        end
      end
    end
  end
  
  def xor(values)
    values.inject(0) { |n, xor| xor ^ n }
  end
  
  ranges = { 
      :char => [ 0, 127, -128, -1 ],
      :uchar => [ 0, 0x7f, 0x80, 0xff ],
      :short => [ 0, 0x7fff, -0x8000, -1 ],
      :ushort => [ 0, 0x7fff, 0x8000, 0xffff ],
      :int => [ 0, 0x7fffffff, -0x80000000, -1 ],
      :uint => [ 0, 0x7fffffff, 0x80000000, 0xffffffff ],
      :long_long => [ 0, 0x7fffffffffffffff, -0x8000000000000000, -1 ],
      :ulong_long => [ 0, 0x7fffffffffffffff, 0x8000000000000000, 0xffffffffffffffff ]
  }
  
  describe 'signed char' do
    it 'returns correct value' do
      (-128..127).each { |n | Primitive.ret_char(n).should == n }
    end
  end
  
  it 'unsigned char' do
    (0..255).each { |n | Primitive.ret_uchar(n).should == n }
  end

  it 'signed short' do
    (-32768..32767).each { |n| Primitive.ret_short(n).should == n }
  end
  
  it 'unsigned short' do
    (0..65535).each { |n| Primitive.ret_ushort(n).should == n }
  end

  # Test that passing up to 6 integers works
  describe 'multiple integer parameters' do
    (2..6).each do |arity|
      [ :char, :uchar, :short, :ushort, :int, :uint, :long_long, :ulong_long ].each do |type|
        it "xor_#{type}_#{arity}" do
          iterate(arity, ranges[type]) do |params|
            Primitive.send("xor_#{type}_#{arity}", *params).should == xor(params)
          end
        end
      end
    end
  end
end