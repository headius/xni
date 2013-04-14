
describe 'data_object' do
  before :all do
    load_path = $:.map{ |p| "-I#{p}" }.join(' ')
    system("cd #{File.dirname(__FILE__)}/ext && rake #{load_path}")
    require File.join(File.dirname(__FILE__), 'lib', 'data_object')
  end

  describe 'attributes' do
    subject(:foobar) { DataObject::FooBar.new(0xdeadbeef, 0xfee1dead) }
    it { foobar.m_foo.should == 0xdeadbeef }
    it { foobar.m_bar.should == 0xfee1dead }
  end
  
  describe 'instance methods' do 
    describe 'setter' do
      subject(:foobar) { DataObject::FooBar.new(0, 0) }
      it 'foo=' do
        foobar.foo = 0xdeadbeef
        foobar.m_foo.should == 0xdeadbeef
      end
  
      it 'bar=' do
        foobar.bar = 0xdeadbeef
        foobar.m_bar.should == 0xdeadbeef
      end
    end

    describe 'getter' do
      subject(:foobar) { DataObject::FooBar.new(0xdeadbeef, 0xfee1dead) }
      it { foobar.foo.should == 0xdeadbeef }
      it { foobar.bar.should == 0xfee1dead }
    end
    
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

  describe 'multiple integer parameters to singleton function' do
    let(:x) { DataObject::XOR }
    (2..6).each do |arity|
      [ :char, :uchar, :short, :ushort, :int, :uint, :long_long, :ulong_long ].each do |type|
        it "xor_#{type}_#{arity}" do
          iterate(arity, ranges[type]) do |params|
            x.send("xor_#{type}_#{arity}", *params).should == xor(params)
          end
        end
      end
    end
  end

  describe 'multiple integer parameters to instance method' do
    subject(:x) { DataObject::XOR.new }
    (2..6).each do |arity|
      [ :char, :uchar, :short, :ushort, :int, :uint, :long_long, :ulong_long ].each do |type|
        it "xor_#{type}_#{arity}" do
          iterate(arity, ranges[type]) do |params|
            x.send("xor_#{type}_#{arity}", *params).should == xor(params)
          end
        end
      end
    end
  end
end