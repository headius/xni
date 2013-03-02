module XNI
  class DataObject
    class Factory

      def initialize(klass, size, finalizer = nil)
        if size.is_a?(Integer)
          # Create a struct layout from 64 bit integers to fill the amount of memory requested
          fields = (0...((size + 7) / 8)).map { |i| ["pad#{i}".to_sym, FFI::Type::INT64] }.flatten
          @struct_class = Class.new(FFI::Struct) { |c| c.layout *fields }
        else
          @struct_class = size
        end

        @klass = klass
        @finalizer = finalizer
      end

      def allocate
        obj = @klass.allocate
        obj.instance_variable_set :@__xni_struct__, struct = @struct_class.new

        if @finalizer
          obj.instance_variable_set :@__xni_finalizer__, @finalizer
          ObjectSpace.define_finalizer(obj, create_finalizer(struct))
        end
        obj
      end
      
      def new(*args, &b)
        obj = allocate
        obj.send :initialize, *args, &b
        obj
      end

      def finalizer=(finalizer)
        @finalizer = finalizer
      end

      def create_finalizer(ptr)
        lambda { @finalizer.call(ptr) }
      end
      
      def autorelease
        @autorelease ||= AutoReleaseFactory.new(@klass, @struct_class, @finalizer)
      end
    end
    
    class AutoReleaseFactory < Factory
      def new(*args, &b)
        obj = super
        AutoReleasePool.active.add(obj)
        obj
      end

      def autorelease
        self
      end
    end
  end
end