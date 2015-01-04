require 'mkmf'

extension_name = 'renet'

create_makefile(extension_name)

system("make")

