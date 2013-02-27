package xni;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 *
 */
final class Layout {
    private final Map<String, Field> fieldMap;
    private final int size, alignment;
    
    static final class Field {
        final String name;
        final Type type;
        final int offset;

        Field(String name, Type type, int offset) {
            this.name = name;
            this.type = type;
            this.offset = offset;
        }
    }

    Layout(Collection<Field> fields) {
        Map<String, Field> map = new HashMap<String, Field>();
        int size = 0, alignment = 1;
        for (Field f : fields) {
            map.put(f.name, f);
            size = Math.max(size, f.offset + f.type.size());
            alignment = Math.max(alignment, f.type.alignment());
        }
        this.fieldMap = Collections.unmodifiableMap(new HashMap<String, Field>(map));
        this.size = Util.align(size, alignment);
        this.alignment = alignment;
    }
    
    Layout(int size) {
        this.fieldMap = Collections.emptyMap();
        this.size = Util.align(size, 8);
        this.alignment = 8;
    }
    
    Field getField(String name) {
        return fieldMap.get(name);
    }
    
    int size() {
        return size;
    }
}
