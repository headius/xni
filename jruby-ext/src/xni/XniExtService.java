package xni;

import java.io.IOException;
import java.lang.RuntimeException;

import org.jruby.Ruby;
import org.jruby.runtime.load.BasicLibraryService;


public class XniExtService implements BasicLibraryService {
    public boolean basicLoad(final Ruby runtime) throws IOException {
        new XNILibrary().load(runtime, false);
        return true;
    }
}