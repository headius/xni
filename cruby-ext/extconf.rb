if !defined?(RUBY_ENGINE) || RUBY_ENGINE == 'ruby'
  require 'mkmf'
  require 'rbconfig'
  dir_config 'cruby-ext'

  # recent versions of ruby add restrictive ansi and warning flags on a whim - kill them all
  $warnflags = ''
  $CFLAGS.gsub!(/[\s+]-ansi/, '')
  $CFLAGS.gsub!(/[\s+]-std=[^\s]+/, '')

  if ENV['RUBY_CC_VERSION'].nil? && (pkg_config("libffi") || have_header("ffi.h") || 
      find_header("ffi.h", "/usr/local/include", "/usr/include/ffi"))

    # We need at least ffi_call and ffi_prep_closure
    libffi_ok = have_library("ffi", "ffi_call", [ "ffi.h" ]) ||
        have_library("libffi", "ffi_call", [ "ffi.h" ])
    libffi_ok &&= have_func("ffi_prep_closure")
  end
  
  raise "no libffi on system" unless libffi_ok

  if RUBY_VERSION >= "2.0.0"
    have_func('rb_thread_call_with_gvl')
    have_func('rb_thread_call_without_gvl')
  end

  have_func('rb_thread_blocking_region')
  have_func('ffi_prep_cif_var')

  create_header

  $CFLAGS << " -mwin32 " if RbConfig::CONFIG['host_os'] =~ /cygwin/
  $LOCAL_LIBS << " ./libffi/.libs/libffi_convenience.lib" if RbConfig::CONFIG['host_os'] =~ /mswin/

  create_makefile 'xni_cruby'

else
  File.open("Makefile", "w") do |mf|
    mf.puts "# Dummy makefile for non-mri rubies"
    mf.puts "all install::\n"
  end
end