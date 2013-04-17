#ifndef XNI_XNI_H
#define XNI_XNI_H


#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
# include <exception>
# include <stdexcept>
#endif

#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

#ifdef __cplusplus
#  define XNI_EXPORT extern "C"
#else
#  define XNI_EXPORT
#endif

#define XNI_HIDDEN __attribute__((visibility("hidden")))
#define XNI_VISIBLE __attribute__((visibility("default")))

/* return XNI_OK or XNI_ERR from _load */
#define XNI_OK (0)
#define XNI_ERR (-1)

typedef signed long long fixnum;

#ifdef __cplusplus
extern "C" {
#endif

struct RubyInterface_;
typedef struct RubyVM_ RubyVM;

#ifdef __cplusplus
  struct RubyEnv_;
  typedef struct RubyEnv_ RubyEnv;

#else
  typedef const struct RubyInterface_* RubyEnv;
#endif

typedef struct RubyException_ *RubyException;

extern RubyException xni_eException;
extern RubyException xni_eRuntimeError;
extern RubyException xni_eArgError;
extern RubyException xni_eIndexError;

struct RubyInterface_ {
    void* (*ext_data)(RubyEnv *);
    void (*raise_exception)(RubyEnv*, RubyException, const char *, va_list);
};

#ifdef __cplusplus
} /* extern "C" { */
#endif

#ifdef __cplusplus

struct VMInterface {
    void (*vm_raise)(void* vm_data, RubyException ex, const char* message);
};

struct ExtensionData {
    void* ext_data;
    void* vm_data;
    VMInterface* vm;
};

struct RubyEnv_ {
private:
    const RubyInterface_* m_ruby;
    const ExtensionData* m_ed;

public:
    RubyEnv_(RubyInterface_* ruby, ExtensionData* ed): m_ruby(ruby), m_ed(ed) {}
    inline const RubyInterface_* operator->() const { 
        return m_ruby; 
    }
    
    inline void* ext_data() {
        return m_ed->ext_data; 
    }
    
    inline void raise(RubyException ex, const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        (*m_ruby->raise_exception)(this, ex, fmt, ap);
        va_end(ap);
    }
};

namespace xni {
    class exception: public std::exception {
    private:
        const RubyException m_exc;
        const std::string m_message;
    
    protected:
        exception(RubyException ex_, const std::string& message_): m_exc(ex_), m_message(message_) {}
        
    public:
        virtual ~exception() throw();
        const char* what() throw();
        
        inline const RubyException exc() const {
            return m_exc;
        }
         
        inline std::string message() const {
            return m_message;
        }
    };
    
    class runtime_error: public exception {
    public:
        runtime_error(const std::string& message_): exception(xni_eRuntimeError, message_) {}
    };
    
    class arg_error: public exception {
    public:
        arg_error(const std::string& message_): exception(xni_eArgError, message_) {}
    };
    
    class index_error: public exception {
    public:
        index_error(const std::string& message_): exception(xni_eIndexError, message_) {}
    };
}

#endif /* __cplusplus */

inline void
xni_raise(RubyEnv* env, RubyException ex, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (*env)->raise_exception(env, ex, fmt, ap);
    va_end(ap);
}

inline void* 
xni_data(RubyEnv* env) {
    return (*env)->ext_data(env);
}
 
#ifdef __GNUC__
# pragma GCC visibility pop
#endif

#endif /* XNI_XNI_H */