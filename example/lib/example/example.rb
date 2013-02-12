require 'xni'

module Example
  extend XNI::Extension
  extension 'example'

  # Attach xni_example_foo() as Example.foo
  native :foo, [], :ulong_long
   
  # A DataObject has native memory automatically allocated to hold the fields described by the 'data' directive
  class Foo < XNI::DataObject
    data :m_foo, :ulong_long,
         :m_bar, :ulong_long
    
    data_reader :m_bar
    data_accessor :m_foo

    # Attach a native function as Example::Foo#initialize
    native :initialize, [ :int ], :void
    
    # Attach xni_example_foo_foo() as Example::Foo#foo
    native :foo, [], :ulong_long
    
    native :pointer, [], :pointer
    
    # custom_finalizer indicates that before freeing the backing memory, it should call xni_#{class name}_finalize 
    custom_finalizer
    
    def inspect
      "#<#{self.class} m_foo=#{m_foo} m_bar=#{m_bar}>"
    end
  end
  
  class Bar < XNI::DataObject
    custom_finalizer

    # Attach a native function as Example::Bar#initialize
    native :initialize, [], :void

    # Attach xni_example_foo_foo() as Example::Bar#bar
    native :bar, [], :ulong_long    
  end
end


