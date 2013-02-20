xni [![Build Status](https://travis-ci.org/wmeissner/xni.png)](https://travis-ci.org/wmeissner/xni)
======

[XNI](https://github.com/wmeissner/xni) is a ruby library for building extensions that run on JRuby and MRI

##### Example usage
###### example.rb

    module Example
	  extend XNI::Extension
	  extension 'example'
	
	  # Attach xni_example_foo() as Example.foo
	  native :foo, [], :fixnum
	   
	  class Foo < XNI::DataObject
	    data :m_foo, :fixnum,
	         :m_bar, :fixnum
	    
	    data_reader :m_bar
	    data_accessor :m_foo
	
	    # Attach a native function as Example::Foo#initialize
	    native :initialize, [ :fixnum ], :void
	    
	    # Attach xni_example_foo_foo() as Example::Foo#foo
	    native :foo, [], :fixnum
	  end
    end
    
###### example.cpp

    XNI_EXPORT fixnum
    xni_example_foo(RubyEnv* rb)
    {
        return 0xfee1deadcafebabeLL;
    }

    XNI_EXPORT void 
    xni_example_foo_initialize(RubyEnv* rb, struct Example_Foo* foo, fixnum foo_value)
    {
        foo->m_bar = 0xbabef00dLL;
        foo->m_foo = foo_value;
    }
    
    XNI_EXPORT fixnum
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
    :fixnum   - 64 bit signed integer
    :double   - 64 bit floating point
    :pointer  - void *    
    :cstring  - const char *
    :void     - void
    :bool     - bool
