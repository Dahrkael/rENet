require 'mkmf'

extension_name = 'renet'
have_library('enet')
dir_config(extension_name)
create_makefile(extension_name)

system("make")

#if `uname`.chomp == 'Darwin'
#	`ln -s renet.bundle ../../lib/renet.bundle`
#else
#	`ln -s renet.so ../../lib/renet.so`
#end
