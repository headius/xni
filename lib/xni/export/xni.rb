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
      :float => 'float',
      :double => 'double',
      :pointer => 'void *',
      :string => 'const char *',
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

        @structs.each_pair do |struct_class, fields|
          if fields.empty?
            mod_name = struct_class.to_s.split('::')[0..-2].join('_').downcase
            struct_name = struct_class.to_s.split('::')[-1].downcase
            f.puts "XNI_EXPORT int xni_#{mod_name}_sizeof_#{struct_name}(void);"
          end
        end
        
        @functions.each do |fn|
          param_string = fn[:params].empty? ? 'void' : fn[:params].map(&:cdecl).join(', ')
          f.puts "XNI_EXPORT #{fn[:result_type].cdecl} #{fn[:cname]}(#{param_string});"
        end
        f.puts <<-EPILOG

#endif /* #{guard} */
        EPILOG
      end
    end
    
    def stubs(stubs_file, header_file)
      File.open(stubs_file, 'w') do |f|
        f.puts "#include <xni.h>"
        f.puts "#include \"#{File.basename(header_file)}\""
        f.puts <<-API
        
static void* ext_data(RubyEnv *);
static struct RubyInterface_ xni_funcs = {
  ext_data
};

struct RubyEnvImpl: public RubyEnv_ {
    void* ext_data;
    RubyEnvImpl(void* ext_data_): RubyEnv_(&xni_funcs), ext_data(ext_data_) {}    
};

static void* 
ext_data(RubyEnv* rb)
{
    return reinterpret_cast<RubyEnvImpl *>(rb)->ext_data;
}
        API

        @functions.each do |fn|
          
          param_names = (0...fn[:params].drop(1).length).each_with_object([]) { |i, ary| ary << "a#{i}" }
          params_with_names = fn[:params].drop(1).each_with_object([]) { |p, ary| ary << "#{p.cdecl('a' + ary.length.to_s)}"}
          params_with_names.unshift('void* ext_data') 
          f.puts <<-STUB
extern "C" #{fn[:result_type].cdecl} __#{fn[:cname]}(#{params_with_names.join(', ')});

extern "C" #{fn[:result_type].cdecl} 
__#{fn[:cname]}(#{params_with_names.join(', ')}) 
{
    RubyEnvImpl rb(ext_data);
    #{fn[:result_type].cdecl != 'void' ? 'return ' : ''}#{fn[:cname]}(#{param_names.unshift('&rb').join(', ')});
} 

          STUB
        end

        mod_name = @mod.to_s.gsub('::', '_').downcase
        f.puts <<-LOAD
extern "C" void* __xni_#{mod_name}_load(void* (*load)(void));
extern "C" void* __xni_#{mod_name}_load(void* (*load)(void)) 
{
    return (*load)();
}

extern "C" void __xni_#{mod_name}_unload(void (*unload)(void *), void* ext_data);
extern "C" void __xni_#{mod_name}_unload(void (*unload)(void *), void* ext_data) 
{
    (*unload)(ext_data);
}

        LOAD
        
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
      cname = 'xni_' + mod_name + '_' + name.to_s
      XNI.exporter.attach(name, cname, find_type(rtype), params.map { |t| find_type(t) }.unshift(RubyEnv))
    end

    def extension(name)
    end
    
    def callback(params, rtype)
      Callback.new(find_type(rtype), params.map { |t| find_type(t) })
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
      XNI.exporter.add_struct(klass)
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
      XNI.exporter.attach(name, "xni_#{class_name}_finalize", find_type(:void), [ RubyEnv, find_type(self) ])
    end
    
    def self.native(name, params, rtype)
      mod_name = self.to_s.gsub('::', '_').downcase
      cname = 'xni_' + mod_name + '_' + name.to_s.sub(/\?$/, '_p')
      XNI.exporter.attach(name, cname, find_type(rtype), params.map { |t| find_type(t) }.unshift(find_type(self)).unshift(RubyEnv) )
    end
    
    def self.lifecycle(*args)
      
    end
  end
end