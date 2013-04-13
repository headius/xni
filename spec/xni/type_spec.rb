require 'xni'

describe XNI::Type do
  it { XNI::Type::SCHAR.size.should == 1 }
  it { XNI::Type::UCHAR.size.should == 1 }
  it { XNI::Type::SSHORT.size.should == 2 }
  it { XNI::Type::USHORT.size.should == 2 }
  it { XNI::Type::SLONG_LONG.size.should == 8 }
  it { XNI::Type::ULONG_LONG.size.should == 8 }
  it { XNI::Type::FLOAT.size.should == 4 }
  it { XNI::Type::DOUBLE.size.should == 8 }
  it { XNI::Type::POINTER.size.should == 8 }
end