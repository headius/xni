require 'pathname'
require 'ffi'
require 'xni/platform'

module XNI
  module Loader
    def self.find(name, start_path = nil)
      platform = Platform.system
      library = platform.map_library_name(name)
      root = false
      Pathname.new(start_path || File.dirname(caller[0].split(/:/)[0])).ascend do |path|
        Dir.glob("#{path}/**/{#{platform.arch}-#{platform.os}/#{library},#{library}}") do |f|
          return f
        end

        break if root

        # Next iteration will be the root of the gem if this is the lib/ dir - stop after that
        root = File.basename(path) == 'lib'
      end
      raise LoadError.new("cannot find '#{name}' library")
    end
  end
end