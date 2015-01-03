if File.exists?("#{File.dirname(__FILE__)}/renet.bundle")
  require "#{File.dirname(__FILE__)}/renet.bundle"
elsif File.exists?("#{File.dirname(__FILE__)}/renet.so")
  require "#{File.dirname(__FILE__)}/renet.so"
elsif File.exists?("#{File.dirname(__FILE__)}/renet.1.8.so")
  if defined? RUBY_VERSION and RUBY_VERSION[0..2] == '1.9' then
    version = '1.9'
  else
    version = '1.8'
  end
  require "#{File.dirname(__FILE__)}/renet.#{version}.so"
end
