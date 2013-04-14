module XNI
  class Platform
    def self.system
      @system ||= Platform.new
    end

    def os
      @os ||= case RbConfig::CONFIG['host_os'].downcase
                when /linux/
                  'linux'
                when /darwin/
                  'darwin'
                when /freebsd/
                  'freebsd'
                when /openbsd/
                  'openbsd'
                when /sunos|solaris/
                  'solaris'
                when /mingw|mswin/
                  'windows'
                else
                  RbConfig::CONFIG['host_os'].downcase
              end
    end

    def name
      "#{cpu}-#{os}"
    end

    def mac?
      os == 'darwin'
    end

    def map_library_name(lib_name)
      if mac?
        "#{lib_name}.bundle"

      elsif os == 'windows'
        lib_name + '.dll'

      else
        "#{lib_name}.so"
      end
    end
  end
end