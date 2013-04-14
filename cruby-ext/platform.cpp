/*
 * Copyright (C) 2008-2013 Wayne Meissner
 *
 * This file is part of the XNI project.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ruby.h>
#include <ffi.h>

#include "xni.h"
#include "pointer.h"

#ifdef RUBY_EXTCONF_H
# include RUBY_EXTCONF_H
#endif

using namespace xni;


#ifdef __GNUC__
# pragma GCC visibility push(hidden)
#endif

VALUE xni_cPlatform;

/*
 * Determine the cpu type at compile time - useful for MacOSX where the the
 * system installed ruby incorrectly reports 'host_cpu' as 'powerpc' when running
 * on intel.
 */
#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64) || defined(_M_X64) || defined(_M_AMD64)
# define CPU "x86_64"

#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
# define CPU "i386"

#elif defined(__ppc64__) || defined(__powerpc64__) || defined(_M_PPC)
# define CPU "ppc64"

#elif defined(__ppc__) || defined(__powerpc__) || defined(__powerpc)
# define CPU "ppc"

/* Need to check for __sparcv9 first, because __sparc will be defined either way. */
#elif defined(__sparcv9__) || defined(__sparcv9)
# define CPU "sparcv9"

#elif defined(__sparc__) || defined(__sparc)
# define CPU "sparc"

#elif defined(__arm__) || defined(__arm)
# define CPU "arm"

#elif defined(__mips__) || defined(__mips)
# define CPU "mips"

#elif defined(__s390__)
# define CPU "s390"

#else
# define CPU "unknown"
#endif

static VALUE
platform_cpu(VALUE self)
{
    return rb_str_new_cstr(CPU);
}

void
xni::platform_init(VALUE xniModule)
{
    xni_cPlatform = rb_define_class_under(xniModule, "Platform", rb_cObject);
    rb_define_method(xni_cPlatform , "cpu", RUBY_METHOD_FUNC(platform_cpu), 0);
    rb_define_alias(xni_cPlatform, "arch", "cpu");
}

#ifdef __GNUC__
# pragma GCC visibility pop
#endif
