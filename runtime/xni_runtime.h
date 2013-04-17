#ifndef XNI_RUNTIME_H
#define XNI_RUNTIME_H

namespace xni {

    enum {
        Exception = 1,
        RuntimeError = 2,
        ArgError = 3,
        IndexError = 4,
    };

    void handle_exception(ExtensionData*, std::exception &ex);
    void handle_exception(ExtensionData*, xni::exception &ex);
    extern RubyInterface_ ruby_functions;
};

#endif /* XNI_RUNTIME_H */