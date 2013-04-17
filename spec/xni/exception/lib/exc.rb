require 'xni'

module Exc
  extend XNI::Extension
  extension 'exc'
  
  native :throw_xni_runtime_error, [ :cstring ], :void
  native :throw_xni_arg_error, [ :cstring ], :void
  native :throw_std_runtime_error, [ :cstring ], :void
  native :throw_std_invalid_argument, [ :cstring ], :void
  native :throw_std_out_of_range, [ :cstring ], :void
end