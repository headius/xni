
package xni;

import com.kenai.jffi.Library;
import org.jruby.*;
import org.jruby.anno.JRubyConstant;
import org.jruby.anno.JRubyMethod;
import org.jruby.runtime.ObjectAllocator;
import org.jruby.runtime.ThreadContext;
import org.jruby.runtime.builtin.IRubyObject;

public class DynamicLibrary extends RubyObject {
    
    @JRubyConstant
    public static final int RTLD_LAZY   = 0x00001;
    @JRubyConstant
    public static final int RTLD_NOW    = 0x00002;
    @JRubyConstant
    public static final int RTLD_LOCAL  = 0x00004;
    @JRubyConstant
    public static final int RTLD_GLOBAL = 0x00008;
    
    private final Library library;
    private final String name;
    
    public static RubyClass createDynamicLibraryClass(Ruby runtime, RubyModule module) {
        RubyClass dynamicLibraryClass = module.defineClassUnder("DynamicLibrary", runtime.getObject(),
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        dynamicLibraryClass.defineAnnotatedMethods(DynamicLibrary.class);
        dynamicLibraryClass.defineAnnotatedConstants(DynamicLibrary.class);
        
        RubyClass symbolClass = dynamicLibraryClass.defineClassUnder("Symbol", runtime.getObject(), 
                ObjectAllocator.NOT_ALLOCATABLE_ALLOCATOR);
        
        symbolClass.defineAnnotatedMethods(Symbol.class);
        symbolClass.defineAnnotatedConstants(Symbol.class);

        return dynamicLibraryClass;
    }
    
    private static final int getNativeLibraryFlags(IRubyObject rbFlags) {
        int f = 0, flags = RubyNumeric.fix2int(rbFlags);
        f |= (flags & RTLD_LAZY) != 0 ? Library.LAZY : 0;
        f |= (flags & RTLD_NOW) != 0 ? Library.NOW : 0;
        f |= (flags & RTLD_LOCAL) != 0 ? Library.LOCAL : 0;
        f |= (flags & RTLD_GLOBAL) != 0 ? Library.GLOBAL : 0;
        return f;
    }
    
    public DynamicLibrary(Ruby runtime, RubyClass klass, String name, Library library) {
        super(runtime, klass);
        this.name = name;
        this.library = library;
    }
    
    final Library getNativeLibrary() {
        return library;
    }
    
    @JRubyMethod(name = {  "open" }, meta = true)
    public static final IRubyObject open(ThreadContext context, IRubyObject recv, IRubyObject libraryName, IRubyObject libraryFlags) {
        final String libName = libraryName.isNil() ? null : libraryName.toString();
        try {
            Library library = Library.getCachedInstance(libName, getNativeLibraryFlags(libraryFlags));
            if (library == null) {
                throw new UnsatisfiedLinkError(Library.getLastError());
            }
            return new DynamicLibrary(context.runtime, (RubyClass) recv,
                    libName, library);
        } catch (UnsatisfiedLinkError ex) {
            throw context.runtime.newLoadError(String.format("Could not open library '%s' : %s",
                    libName != null ? libName : "current process", ex.getMessage()));
        }
    }
    
    @JRubyMethod(name = {  "find_variable", "find_symbol" })
    public IRubyObject findVariable(ThreadContext context, IRubyObject symbolName) {
        final String sym = symbolName.toString();
        final long address = library.getSymbolAddress(sym);
        if (address == 0L) {
            return context.runtime.getNil();
        }

        return new Symbol(context.runtime, this, sym, address);
    }

    @JRubyMethod(name = {  "find_function" })
    public IRubyObject findFunction(ThreadContext context, IRubyObject symbolName) {
        final String sym = symbolName.toString();
        final long address = library.getSymbolAddress(sym);
        if (address == 0L) {
            return context.runtime.getNil();
        }
        return new Symbol(context.runtime, this, sym, address);
    }
    
    @JRubyMethod(name = "name")
    public IRubyObject name(ThreadContext context) {
        return name != null ? RubyString.newString(context.runtime, name) : context.runtime.getNil();
    }
    
    public static final class Symbol extends RubyObject {
        private final DynamicLibrary library;
        private final String name;
        private final long address;
        
        public Symbol(Ruby runtime, DynamicLibrary library, String name, long address) {
            super(runtime, runtime.getModule("XNI").getClass("DynamicLibrary").getClass("Symbol"));
            this.library = library;
            this.name = name;
            this.address = address;
        }

        @JRubyMethod(name = "library")
        public IRubyObject library(ThreadContext context) {
            return library;
        }

        @JRubyMethod(name = "address")
        public IRubyObject address(ThreadContext context) {
            return context.getRuntime().newFixnum(address);
        }
        
        @JRubyMethod(name = "inspect")
        public IRubyObject inspect(ThreadContext context) {
            return RubyString.newString(context.runtime,
                    String.format("#<%s library=%s symbol=%s address=%#x>",
                            getMetaClass().getName(), library.name, name(), address()));
        }

        @JRubyMethod(name = "to_s", optional = 1)
        public IRubyObject to_s(ThreadContext context, IRubyObject[] args) {
            return RubyString.newString(context.runtime, name);
        }

        @Override
        public final String toString() {
            return name;
        }

        public final String name() {
            return name;
        }
        
        public final long address() {
            return address;
        }
    }
}
