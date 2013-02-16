require 'ffi'

module XNI
  class Platform
    LIBSUFFIX = FFI::Platform.mac? ? 'bundle' : FFI::Platform::LIBSUFFIX
    LIBPREFIX = FFI::Platform.mac? ? '' : FFI::Platform::LIBPREFIX

    def self.system
      @@system ||= Platform.new
    end

    def map_library_name(name)
      "#{LIBPREFIX}#{name}.#{LIBSUFFIX}"
    end

    def arch
      FFI::Platform::ARCH
    end

    def os
      FFI::Platform::OS
    end

    def name
      FFI::Platform.name
    end

    def mac?
      FFI::Platform.mac?
    end
  end
end