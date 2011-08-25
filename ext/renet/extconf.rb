require 'mkmf'

extension_name = 'renet'

if !have_library('enet', 'enet_initialize')
	puts "ENet library required to build the gem, please install it"
end

dir_config('enet', '/usr/include/enet', 'usr/lib')
create_makefile(extension_name)

system("make")

#if `uname`.chomp == 'Darwin'
#	`ln -s renet.bundle ../../lib/renet.bundle`
#else
#	`ln -s renet.so ../../lib/renet.so`
#end
