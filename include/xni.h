#ifndef XNI_XNI_H
#define XNI_XNI_H


#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
#  define XNI_EXPORT extern "C"
#else
#  define XNI_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct RubyInterface_;

#ifdef __cplusplus
  struct RubyEnv_;
  typedef struct RubyEnv_ RubyEnv;

#else
  typedef const struct RubyInterface_* RubyEnv;
#endif

struct RubyInterface_ {
    void* (*ext_data)(RubyEnv *rb);
};

#ifdef __cplusplus
} /* extern "C" { */
#endif

#ifdef __cplusplus
struct RubyEnv_ {
protected:
    const RubyInterface_* ruby;

public:
    RubyEnv_(RubyInterface_* funcs): ruby(funcs) {}
    
    inline void* ext_data() {
        return ruby->ext_data(this); 
    }
};

#include <exception>
#include <stdexcept>

namespace xni {
    class Exception: public std::runtime_error {
    private:
        std::string message;
    
    public:
        Exception(RubyEnv *env_, std::string& message_);
        ~Exception() throw();
    };
}

#endif /* __cplusplus */




#endif /* XNI_XNI_H */