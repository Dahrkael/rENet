require 'mkmf'
# $CFLAGS += ' -Wall -Werror -Wno-declaration-after-statement'
case RUBY_PLATFORM
  when /(mingw|mswin|windows)/i
    $LDFLAGS += ' -lwinmm '
end
create_makefile('renet/renet')
