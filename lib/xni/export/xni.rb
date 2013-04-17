module XNI

  def self.exporter=(exporter)
    @@exporter = exporter
  end

  def self.exporter
    @@exporter
  end

  class Type
    def initialize(cdecl)
      @cdecl = cdecl
    end
    
    def cdecl(name = nil)
      if name
        "#{@cdecl} #{name}"
      else
        @cdecl
      end
    end
    
    def to_s
      cdecl
    end
  end
  
  class Callback < Type
    def initialize(rtype, params)
      @rtype = rtype.dup
      @params = params.dup
    end
    
    def cdecl(name = nil)
      if name
        "#@rtype (*#{name})(#{@params.join(', ')})"
      else
        "#@rtype (*)(#{@params.join(', ')})"
      end
    end
  end

  RubyEnv = Type.new('RubyEnv *')

  class StructByReference < Type
    def initialize(struct_class)
      super("struct #{struct_class.to_s.gsub('::', '_')} *")
    end
  end
  
  class ArrayType < Type
    attr_reader :element_type, :direction
    def initialize(element_type, direction)
      super("#{direction == :in ? 'const ' : ''}#{element_type} *")
    end
    
    ALLOWED_TYPES = [ :bool, :fixnum, :double ]
  end

  PrimitiveTypes = {
      :void => 'void',
      :bool => 'bool',
      :char => 'char',
      :uchar => 'unsigned char',
      :short => 'short',
      :ushort => 'unsigned short',
      :int => 'int',
      :uint => 'unsigned int',
      :long => 'long',
      :ulong => 'unsigned long',
      :long_long => 'long long',
      :ulong_long => 'unsigned long long',
      :fixnum => 'fixnum',
      :float => 'float',
      :double => 'double',
      :pointer => 'void *',
      :cstring => 'const char *',
  }

  module TypeMapCache
    TypeMap = {}
    def find_type(type)
      TypeMap[type] ||= XNI.find_type(type)
    end
  end
  
  TypeMap = {}
  def self.find_type(type)
    return type if type.is_a?(Type)

    t = TypeMap[type]
    return t unless t.nil?

    if PrimitiveTypes.has_key?(type)
      TypeMap[type] = Type.new(PrimitiveTypes[type])
    
    elsif type.is_a?(Class) && (type < Struct || type < DataObject)
      TypeMap[type] = StructByReference.new(type)
    
    else
      raise TypeError.new("cannot resolve type #{type}")
    end    
  end


  def self.carray(type, direction = :in)
    raise ArgumentError.new("#{type} not allowed") unless ArrayType::ALLOWED_TYPES.include?(type)
    raise ArgumentError.new("invalid direction, #{direction}") unless [ :in, :out, :inout ].include?(direction)
    ArrayType.new(find_type(type), direction)
  end

  class Exporter
    attr_reader :mod, :functions

    def initialize(mod)
      @mod = mod
      @functions = []
      @structs = {}
    end

    def attach(mname, cname, result_type, param_types)
      @functions << { mname: mname, cname: cname, result_type: result_type, params: param_types.dup }
    end

    def add_struct(struct_class)
      @structs[struct_class] = [] 
    end

    def struct_layout(struct_class, fields)
      @structs[struct_class] = fields.dup
    end

    def output(stubs_file, header_file)
      header(header_file)
      stubs(stubs_file, header_file)
    end

    def header(out_file)
      File.open(out_file, 'w') do |f|
        guard = File.basename(out_file).upcase.gsub('.', '_').gsub('/', '_')
        f.puts <<-HEADER
#ifndef #{guard}
#define #{guard} 1

#include <xni.h>

        HEADER

        @structs.each_pair do |struct_class, fields|
          struct_name = struct_class.to_s.gsub('::', '_')
          f.puts "struct #{struct_name};"
          
          unless fields.empty?
            f.puts "struct #{struct_name} {"
            fields.each do |field|
              f.puts "#{' ' * 4}#{field[:type].cdecl} #{field[:name].to_s};"
            end
            f.puts '};'
            f.puts
          end
        end

        f.puts <<-HIDDEN
#ifdef __GNUC__
#pragma GCC visibility push(hidden)
#endif
        HIDDEN

        @structs.each_pair do |struct_class, fields|
          if fields.empty?
            mod_name = struct_class.to_s.split('::')[0..-2].join('_').downcase
            struct_name = struct_class.to_s.split('::')[-1].downcase
            f.puts "XNI_EXPORT int #{mod_name}_sizeof_#{struct_name}(void);"
          end
        end

        @functions.each do |fn|
          param_string = fn[:params].empty? ? 'void' : fn[:params].map(&:cdecl).join(', ')
          f.puts "XNI_EXPORT #{fn[:result_type].cdecl} #{fn[:cname]}(#{param_string});"
        end

        f.puts <<-HIDDEN
#ifdef __GNUC__
#pragma GCC visibility pop
#endif
        HIDDEN

        mod_name = @mod.to_s.gsub('::', '_').downcase
        f.puts <<-LOAD
XNI_EXPORT int xni_#{mod_name}_load(RubyVM *,void **);
XNI_EXPORT void xni_#{mod_name}_unload(RubyVM *, void *);
        LOAD
        
        f.puts <<-EPILOG

#endif /* #{guard} */
        EPILOG
      end
    end
    
    def stubs(stubs_file, header_file)
      File.open(stubs_file, 'w') do |f|
        f.puts "#include <xni.h>"
        f.puts "#include \"#{File.basename(header_file)}\""
        f.puts "#include \"#{File.expand_path('../../../../runtime/xni_runtime.h', __FILE__)}\""

        @functions.each do |fn|
          
          param_names = (0...fn[:params].drop(1).length).each_with_object([]) { |i, ary| ary << "a#{i}" }
          params_with_names = fn[:params].drop(1).each_with_object([]) { |p, ary| ary << "#{p.cdecl('a' + ary.length.to_s)}"}
          params_with_names.unshift('void* ext_data') 
          
          f.puts <<-STUB
extern "C" #{fn[:result_type].cdecl} xni_#{fn[:cname]}(#{params_with_names.join(', ')});

extern "C" #{fn[:result_type].cdecl} 
xni_#{fn[:cname]}(#{params_with_names.join(', ')}) 
{
    RubyEnv_ rb(&xni::ruby_functions, (ExtensionData *) ext_data);
    try {
        #{fn[:result_type].cdecl != 'void' ? 'return ' : ''}#{fn[:cname]}(#{param_names.unshift('&rb').join(', ')});
    
    } catch (xni::exception& xni_exc) { 
      xni::handle_exception((ExtensionData *) ext_data, xni_exc);
    
    } catch (std::exception& std_exc) { 
      xni::handle_exception((ExtensionData *) ext_data, std_exc); 
    }
} 

          STUB
        end
        
        @structs.each_pair do |struct_class, fields|
          if fields.empty?
            mod_name = struct_class.to_s.split('::')[0..-2].join('_').downcase
            struct_name = struct_class.to_s.split('::')[-1].downcase
            f.puts <<-STUB
extern "C" int xni_#{mod_name}_sizeof_#{struct_name}(void);
extern "C" int xni_#{mod_name}_sizeof_#{struct_name}(void)
{
    return #{mod_name}_sizeof_#{struct_name}();
}
            STUB
          end
        end

      end
    end
  end

  module Extension
    include TypeMapCache

    def self.extended(mod)
      XNI.exporter = Exporter.new(mod)
    end

    def native(name, params, rtype)
      mod_name = self.to_s.gsub('::', '_').downcase
      cname = mod_name + '_' + name.to_s
      XNI.exporter.attach(name, cname, find_type(rtype), params.map { |t| find_type(t) }.unshift(RubyEnv))
    end

    def extension(name)
    end

    def carray(type, direction)
      XNI.carray(type, direction)
    end
  end

  class Struct
    
    def self.layout(*args)
      fields = []
      arg.each_slice(2) do |(name, type)|
        fields << { name: name, type: find_type(type) }
      end
      XNI.exporter.struct(self.to_s, fields)
    end

    TypeMap = {}
    def self.find_type(type)
      t = TypeMap[type]
      return t unless t.nil?

      if type.is_a?(Class) && type < Struct
        return TypeMap[type] = StructByValue.new(type)
      end

      TypeMap[type] = XNI.find_type(type)
    end
  end

  class DataObject
    extend TypeMapCache

    def self.inherited(klass)
      XNI.exporter.add_struct(klass) if self == DataObject
      
      klass.singleton_class.define_singleton_method(:native) do |name, params, rtype|
        mod_name = klass.to_s.gsub('::', '_').downcase
        cname = mod_name + '_s_' + name.to_s.sub(/\?$/, '_p')
        XNI.exporter.attach(name, cname, DataObject.find_type(rtype), params.map { |t| DataObject.find_type(t) }.unshift(RubyEnv) )
      end
    end
    
    def self.data(*fields)
      layout = []
      fields.each_slice(2) do |(name, type)|
        layout << { name: name, type: find_type(type) }
      end

      XNI.exporter.struct_layout(self, layout)
    end
    
    def self.data_reader(*field_names); end
    def self.data_accessor(*field_names); end

    def self.custom_finalizer
      class_name = self.to_s.gsub('::', '_').downcase
      XNI.exporter.attach(name, "#{class_name}_finalize", find_type(:void), [ RubyEnv, find_type(self) ])
    end
    
    def self.native(name, params, rtype)
      mod_name = self.to_s.gsub('::', '_').downcase
      cname = mod_name + '_' + __mangle_function_name__(name)
      XNI.exporter.attach(name, cname, find_type(rtype), params.map { |t| find_type(t) }.unshift(find_type(self)).unshift(RubyEnv) )
    end
    
    def self.carray(type, direction)
      XNI.carray(type, direction)
    end
    
    def self.lifecycle(*args)
      
    end
    
    def self.__mangle_function_name__(name)
      mangled = name.to_s.dup
      
      if mangled =~ /([A-Za-z0-9]+)=$/
        mangled = 'set_' + $1 
      
      elsif mangled =~ /\?$/
        mangled.sub!(/\?$/, '_p')
      end
      
      mangled
    end
  end
end