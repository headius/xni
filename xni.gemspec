Gem::Specification.new do |s|
  s.name = 'xni'
  s.version = '0.1.0.dev'
  s.author = 'Wayne Meissner'
  s.email = 'wmeissner@gmail.com'
  s.homepage = 'http://wiki.github.com/wmeissner/xni'
  s.summary = 'X Native Interface'
  s.description = 'Native C access for Ruby VMs'
  s.files = %w(xni.gemspec LICENSE README.md Rakefile) 
  s.files << Dir.glob('ext/**/*.[ch]')
  s.files << Dir.glob('{spec,libtest}/**/*')
  s.files << Dir.glob('include/**/*.h')
  s.files << Dir.glob('lib/**/*.rb')
  s.has_rdoc = false
  s.license = 'Apache 2.0'
  s.required_ruby_version = '>= 1.9.3'
  s.add_dependency 'rake', '>= 10.0.0'
  s.add_dependency 'ffi', '>= 1.3.0'
  s.add_development_dependency 'rspec'
end
