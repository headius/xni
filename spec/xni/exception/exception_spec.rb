

describe 'exceptions' do
  before :all do
    load_path = $:.map{ |p| "-I#{p}" }.join(' ')
    system("cd #{File.dirname(__FILE__)}/ext && rake #{load_path}")
    require File.join(File.dirname(__FILE__), 'lib', 'exc')
  end
  
  it 'xni::runtime_error' do
    lambda { Exc.throw_xni_runtime_error 'foo bar' }.should raise_error(RuntimeError)
  end
  
  it 'xni::arg_error' do
    lambda { Exc.throw_xni_arg_error 'foo bar' }.should raise_error(ArgumentError)
  end
  
  it 'std::runtime_error' do
    lambda { Exc.throw_std_runtime_error 'foo bar' }.should raise_error(RuntimeError)
  end
  
  it 'std::invalid_argument' do
    lambda { Exc.throw_std_invalid_argument 'foo bar' }.should raise_error(ArgumentError)
  end
  
  it 'std::out_of_range' do
    lambda { Exc.throw_std_out_of_range 'foo bar' }.should raise_error(IndexError)
  end
end