
module XNI
  class AutoReleasePool
    def initialize(&b)
      @objects = {}
      thread = Thread.current
      prev = thread[:autoreleasepool]
      thread[:autoreleasepool] = self
      
      begin
        yield self
      ensure
        begin
          @objects.each_key { |obj| obj.__xni_release__ }
        ensure
          thread[:autoreleasepool] = prev
        end
      end
    end
    
    def add(obj)
      @objects[obj] = true
    end
    
    def remove(obj)
      @objects.delete(obj)
    end
    
    def self.active
      pool = Thread.current[:autoreleasepool]
      raise RuntimeError.new("no active auto release pool") if pool.nil?
      pool
    end
  end
  
  
end