xni [![Build Status](https://travis-ci.org/wmeissner/xni.png)](https://travis-ci.org/wmeissner/xni)
======

[XNI](https://github.com/wmeissner/xni) is a ruby library for building extensions that run on JRuby and MRI

##### Example usage
###### example.rb

    module Example
	  extend XNI::Extension
	  extension 'example'
	
	  # Attach xni_example_foo() as Example.foo
	  native :foo, [], :ulong_long
	   
	  class Foo < XNI::DataObject
	    data :m_foo, :ulong_long,
	         :m_bar, :ulong_long
	    
	    data_reader :m_bar
	    data_accessor :m_foo
	
	    # Attach a native function as Example::Foo#initialize
	    native :initialize, [ :int ], :void
	    
	    # Attach xni_example_foo_foo() as Example::Foo#foo
	    native :foo, [], :ulong_long
	  end
    end
    
###### example.cpp

    XNI_EXPORT void 
    xni_example_foo_initialize(RubyEnv* rb, struct Example_Foo* foo, int foo_value)
    {
        foo->m_bar = 0xbabef00dLL;
        foo->m_foo = foo_value;
    }
    
    XNI_EXPORT unsigned long long
    xni_example_foo(RubyEnv* rb)
    {
        return 0xfee1deadcafebabeLL;
    }
    
    XNI_EXPORT unsigned long long
    xni_example_foo_foo(RubyEnv* rb, struct Example_Foo* foo)
    {
        return foo->m_foo;
    }
    
###### Using it

    jruby-1.7.3.dev :002 > foo = Example::Foo.new(1234)
    => #<Example::Foo m_foo=1234 m_bar=3133075469> 
    jruby-1.7.3.dev :003 > foo.foo
     => 1234     

##### Supported types
    :char, :uchar           - signed/unsigned char
    :short, :ushort         - signed/unsigned short
    :int, :uint             - signed/unsigned int
    :long, :ulong           - signed/unsigned long
    :long_long, :ulong_long - signed/unsigned long long
    :float                  - float
    :double                 - double
    :pointer                - void *    
    :cstring                - const char *
    :void                   - void