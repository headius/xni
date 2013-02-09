require 'xni'

load ARGV[0]
XNI.exporter.output(File.join(File.dirname(ARGV[1]), "__xni_#{File.basename(ARGV[1], '.h')}.cpp"), ARGV[1])
