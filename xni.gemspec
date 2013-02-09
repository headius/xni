Gem::Specification.new do |s|
  s.name = 'xni'
  s.version = '0.0.1'
  s.author = 'Wayne Meissner'
  s.email = 'wmeissner@gmail.com'
  s.homepage = 'http://wiki.github.com/ffi/ffi'
  s.summary = 'X-platform Native Interface'
  s.description = 'Native C access for Ruby VMs'
  s.files = %w(xni.gemspec LICENSE README.md Rakefile) + Dir.glob('ext/**/*.[ch]') + Dir.glob("{gen,spec,libtest}/**/*") 
  s.files << Dir.glob('lib/**/*').reject { |f| f =~ /lib\/1\.[89]/}
#  s.extensions << 'ext/ffi_c/extconf.rb'
  s.has_rdoc = false
  s.license = 'Apache 2.0'
#  s.require_paths << 'ext/ffi_c'
#  s.required_ruby_version = '>= 1.8.7'
  s.add_dependency 'rake', '>= 10.0.0'
  s.add_development_dependency 'rake'
  s.add_development_dependency 'rake-compiler', '>=0.6.0'
  s.add_development_dependency 'rspec'
end
