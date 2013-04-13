require 'rake/clean'
require 'rubygems'
require 'rubygems/tasks'
require 'rbconfig'


def gem_spec
  @gem_spec ||= Gem::Specification.load('xni.gemspec')
end
GMAKE = system('which gmake >/dev/null') ? 'gmake' : 'make'
BUILD_DIR = "build/#{RbConfig::CONFIG['arch']}"
TEST_DEPS = []

if defined?(JRUBY_VERSION)
  require 'ant'
  jar_file = 'lib/xni/xni_ext.jar'
  CLEAN.include jar_file
  TEST_DEPS << jar_file

  gem_spec.files << jar_file

  directory 'pkg/classes'
  CLEAN.include 'pkg/classes'

  desc 'Compile the JRuby extension'
  task :compile => FileList['pkg/classes', 'jruby-ext/src/**/*.java'] do |t|
    ant.javac :srcdir => 'jruby-ext/src', :destdir => t.prerequisites.first,
              :source => '1.6', :target => '1.6', :debug => true, :includeantruntime => false,
              :classpath => '${java.class.path}:${sun.boot.class.path}'
  end

  desc 'Build the jar'
  file jar_file => :compile do |t|
    ant.jar :basedir => 'pkg/classes', :destfile => t.name, :includes => '**/*.class'
  end
  
  task :jar => jar_file

else
  build_dir = BUILD_DIR
  cext_src = File.join(File.dirname(__FILE__), 'cruby-ext')
  cext_file = "#{build_dir}/xni_cruby.#{RbConfig::CONFIG['DLEXT']}"

  directory build_dir
  CLEAN.include cext_file
  TEST_DEPS << cext_file
  CLEAN.include FileList["#{build_dir}/*.o"]
  CLOBBER.include build_dir
  
  desc 'Compile C extension'
  task :compile => cext_file
  
  file "#{build_dir}/Makefile" => [ build_dir ] do
    sh %{cd #{build_dir} && #{Gem.ruby} #{File.join(cext_src, 'extconf.rb')}}
  end

  file cext_file => %W(#{build_dir}/Makefile) + FileList["#{cext_src}/*.cpp"] + FileList["#{cext_src}/*.h"] do
    sh %{cd #{build_dir} && #{GMAKE}}
  end
end

desc "Run all specs"

task :specs => TEST_DEPS do
  sh %{#{Gem.ruby} -w -S rspec -Ilib -I#{BUILD_DIR} spec/xni}
end

Gem::Tasks.new do |t|
  t.scm.tag.format = '%s'
end
