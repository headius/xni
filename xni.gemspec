require File.expand_path("../lib/#{File.basename(__FILE__, '.gemspec')}/version", __FILE__)

Gem::Specification.new do |s|
  s.name = 'xni'
  s.version = XNI::VERSION
  s.author = 'Wayne Meissner'
  s.email = 'wmeissner@gmail.com'
  s.homepage = 'http://wiki.github.com/wmeissner/xni'
  s.summary = 'X Native Interface'
  s.description = 'Native C access for JRuby'
  s.files = %w(xni.gemspec LICENSE README.md Rakefile) 
  s.files += Dir['lib/**/*.rb', 'ext/**/*.[ch]', 'include/**/*.h', '{spec,libtest}/**/*']
  s.files << 'lib/xni/xni_ext.jar' if defined?(JRUBY_VERSION)
  s.has_rdoc = false
  s.license = 'Apache 2.0'
  s.required_ruby_version = '>= 1.9.3'
  s.add_dependency 'rake', '>= 10.0.0'
  s.add_development_dependency 'rspec'
  s.add_development_dependency 'rubygems-tasks'
end
