require 'rake/clean'
require 'rubygems'
require 'rubygems/tasks'


def gem_spec
  @gem_spec ||= Gem::Specification.load('xni.gemspec')
end

if defined?(JRUBY_VERSION)
  require 'ant'
  jar_file = 'lib/xni/xni_ext.jar'
  CLEAN.include jar_file

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
end

Gem::Tasks.new do |t|
  t.scm.tag.format = '%s'
end