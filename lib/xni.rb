module XNI
  if defined?(JRUBY_VERSION) && JRUBY_VERSION >= "1.7.0"
    IMPL_DIR = File.join(File.dirname(__FILE__), 'xni', 'jruby')
  
  else
    IMPL_DIR = File.join(File.dirname(__FILE__), 'xni', 'cruby')
  end
end

require File.join(XNI::IMPL_DIR, 'xni.rb')
require 'xni/extension'
require 'xni/data_object'
