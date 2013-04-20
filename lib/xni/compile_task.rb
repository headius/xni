require 'rake'
require 'rake/tasklib'
require 'rake/clean'
require 'tmpdir'
require 'rbconfig'
require 'xni'

module XNI
  class CompileTask < Rake::TaskLib
    DEFAULT_CFLAGS = %w(-fexceptions -O2 -fno-omit-frame-pointer -fno-strict-aliasing)
    DEFAULT_LDFLAGS = %w(-fexceptions)
    XNI_INCDIR = File.expand_path('../../include', File.dirname(__FILE__))

    attr_reader :cflags, :cxxflags, :ldflags, :libs, :platform

    def initialize(name)
      @name = File.basename(name)
      @ext_dir = File.dirname(name)
      @defines = []
      @include_paths = [ XNI_INCDIR, File.expand_path('../../../runtime') ]
      @library_paths = []
      @libraries = []
      @headers = []
      @functions = []
      @cflags = DEFAULT_CFLAGS.dup
      @cxxflags = []
      @ldflags = DEFAULT_LDFLAGS.dup
      @libs = []
      @platform = Platform.system
      @exports = []

      yield self if block_given?
      define_task!
    end

    def have_func?(func)
      main = <<-C_FILE
        extern void #{func}();
        int main(int argc, char **argv) { #{func}(); return 0; }
      C_FILE

      if try_compile(main)
        @functions << func
        return true
      end
      false
    end

    def have_header?(header, *paths)
      try_header(header, @include_paths) || try_header(header, paths)
    end

    def have_library?(libname, *paths)
      try_library(libname, :paths => @library_paths) || try_library(libname, :paths => paths)
    end

    def have_library(lib, func = nil, headers = nil, &b)
      try_library(lib, :function => func, :headers => headers, :paths => @library_paths)
    end

    def find_library(lib, func, *paths)
      try_library(lib, :function => func, :paths => @library_paths) || try_library(libname, :function => func, :paths => paths)
    end

    def export(rb_file)
      @exports << { 
          :rb_file => rb_file 
      }
    end

    private
    def compile(src_file, obj_file, c)
      extra_opts = src_file =~ /__xni_/ ? ' -fomit-frame-pointer ' : ''
      desc "compile #{src_file}"
      if src_file =~ /\.c$/
        file obj_file => [ src_file, File.dirname(obj_file) ] do |t|
          sh "#{c[:cc]} #{c[:cflags]}#{extra_opts} -o #{t.name} -c #{t.prerequisites[0]}"
        end

      else
        file obj_file => [ src_file, File.dirname(obj_file) ] do |t|
          sh "#{c[:cxx]} #{c[:cxxflags]}#{extra_opts} -o #{t.name} -c #{t.prerequisites[0]}"
        end
      end
    end

    def define_task!
      pic_flags = %w(-fPIC)
      
      out_dir = @platform.name
      @include_paths << out_dir

      if @ext_dir != '.'
        out_dir = File.join(@ext_dir, out_dir)
      end

      directory(out_dir)
      CLOBBER.include(out_dir)

      lib_name = File.join(out_dir, @platform.map_library_name(@name))
      so_flags = []

      if @platform.mac?
        pic_flags = []
        so_flags << '-bundle'

      elsif @platform.name =~ /linux/
        so_flags << "-shared -Wl,-soname,#{lib_name}"

      else
        so_flags << '-shared'
      end
      so_flags = so_flags.join(' ')

      iflags = @include_paths.uniq.map { |p| "-I#{p}" }
      defines = @functions.uniq.map { |f| "-DHAVE_#{f.upcase}=1" }
      defines << @headers.uniq.map { |h| "-DHAVE_#{h.upcase.sub(/\./, '_')}=1" }

      cflags = (@cflags + pic_flags + iflags + defines).join(' ')
      cxxflags = (@cxxflags + @cflags + pic_flags + iflags + defines).join(' ')
      ld_flags = (@library_paths.map { |path| "-L#{path}" } + @ldflags).join(' ')
      libs = (@libraries.map { |l| "-l#{l}" } + @libs).join(' ')
      ld = cxx
      
      obj_files = []
      
      FileList["#{@ext_dir}/**/*.{c,cpp}"].exclude(/#{out_dir}/).each do |src_file|
        obj_file = File.join(out_dir, src_file.sub(/\.(c|cpp)$/, '.o').sub(/^#{@ext_dir}\//, ''))
        compile(src_file, obj_file, cc: cc, cxx: cxx, cflags: cflags, cxxflags: cxxflags)
        obj_files << obj_file
      end

      @exports.each do |e|
        header = File.join(out_dir, File.basename(e[:rb_file]).sub(/\.rb$/, '.h'))
        stub = File.join(out_dir, '__xni_' + File.basename(e[:rb_file]).sub(/\.rb$/, '.cpp'))
        
        file header => [ e[:rb_file], out_dir ] do |t|
          ruby "-I#{File.join(File.dirname(__FILE__), 'export')} #{File.expand_path('../export/exporter.rb', __FILE__)} #{t.prerequisites[0]} #{stub} #{header}"
        end
        
        desc "Export #{e[:rb_file]}"
        namespace :export do
          task File.basename(e[:rb_file]) => header
        end
        
        file stub => [ header ]
        obj_file = stub.sub(/\.(c|cpp)$/, '.o')
        compile(stub, obj_file, cxx: cxx, cxxflags: cxxflags)
        obj_files << obj_file

        CLOBBER.include(header)
        CLOBBER.include(stub)

        obj_files.each { |o| file o => [ header ] }
      end

      # Add in runtime files
      runtime_srcs = Dir.glob(File.expand_path('../../../runtime/*.cpp', __FILE__))
      runtime_srcs.each do |src|
        obj_file = File.join(out_dir, '__xni_rt_' + File.basename(src).sub(/\.(c|cpp)$/, '.o'))
        compile(src, obj_file, cxx: cxx, cxxflags: cxxflags)
        obj_files << obj_file
      end


      CLEAN.include(obj_files)
      
      # create all the directories for the output files 
      obj_files.map { |f| File.dirname(f) }.sort.uniq.map { |d| directory d }

      desc 'Build dynamic library'
      file lib_name => obj_files do |t|
        sh "#{ld} #{so_flags} -o #{t.name} #{t.prerequisites.join(' ')} #{ld_flags} #{libs}"
      end
      CLEAN.include(lib_name)

      
      task :default => [lib_name]
      task :package => [:api_headers]
    end

    def try_header(header, paths)
      main = <<-C_FILE
          #include <#{header}>
          int main(int argc, char **argv) { return 0; }
      C_FILE

      if paths.empty? && try_compile(main)
        @headers << header
        return true
      end

      paths.each do |path|
        if try_compile(main, "-I#{path}")
          @include_paths << path
          @headers << header
          return true
        end
      end
      false
    end


    def try_library(libname, options = {})
      func = options[:function] || 'main'
      paths = options[:paths] || ''
      main = <<-C_FILE
        #{(options[:headers] || []).map { |h| "#include <#{h}>" }.join('\n')}
        extern int #{func}();
        int main() { return #{func}(); }
      C_FILE

      if paths.empty? && try_compile(main)
        @libraries << libname
        return true
      end

      paths.each do |path|
        if try_compile(main, "-L#{path}", "-l#{libname}")
          @library_paths << path
          @libraries << libname
        end
      end
    end

    def try_compile(src, *opts)
      Dir.mktmpdir do |dir|
        path = File.join(dir, 'xni-test.c')
        File.open(path, 'w') do |f|
          f << src
        end
        begin
          return system "#{cc} #{opts.join(' ')} -o #{File.join(dir, 'xni-test')} #{path} >& /dev/null"
        rescue
          return false
        end
      end
    end

    def cc
      @cc ||= (ENV['CC'] || RbConfig::CONFIG['CC'] || 'cc')
    end

    def cxx
      @cxx ||= (ENV['CXX'] || RbConfig::CONFIG['CXX'] || 'c++')
    end
  end
end
