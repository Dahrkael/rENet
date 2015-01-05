require 'mkmf'
$CFLAGS += ' -std=c99 '
case RUBY_PLATFORM
  when /(mingw|mswin|windows)/i
    $LDFLAGS += ' -lwinmm '
end
create_makefile('renet/renet')
