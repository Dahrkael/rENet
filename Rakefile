require 'rubygems'
require 'rubygems/package_task'

project_name = "renet"

def apply_gemspec_defaults(s)
	s.name = "renet"
	s.version = "0.1.15"
	s.date = Time.now.strftime '%Y-%m-%d'
	s.author = "Dahrkael"
	s.email = "dark.wolf.warrior@gmail.com"
	s.homepage = "https://github.com/Dahrkael/rENet"
	s.summary = "Ruby library for games networking. Uses ENet as backend"
	s.description = s.summary
	s.require_path = "lib"
	s.has_rdoc = false
	s.files = Dir["README", "lib/**/*.rb", "examples/*.rb", "test/*.rb"]
end

 
namespace :win do
	 WINDOWS_SPEC = Gem::Specification.new do |s|
		apply_gemspec_defaults(s)
		s.platform = 'i386-mingw32'
		s.files += FileList["lib/**/*.so"]
	end
	Gem::PackageTask.new(WINDOWS_SPEC) do |t|
		t.need_zip = false
		t.need_tar = false
	end
 end
 
namespace :mac do
	 MAC_SPEC = Gem::Specification.new do |s|
		apply_gemspec_defaults(s)
		s.platform = 'universal-darwin'
		s.files += FileList["ext/**/*.h", "ext/**/*.c", "ext/#{project_name}/extconf.rb"]
		s.extensions = ["ext/#{project_name}/extconf.rb"]
	end
	Gem::PackageTask.new(MAC_SPEC) do |t|
		t.need_zip = false
		t.need_tar = false
	end
 end
 
namespace :linux do
	 LINUX_SPEC = Gem::Specification.new do |s|
		apply_gemspec_defaults(s)
		s.platform = 'ruby'
		s.files += FileList["ext/**/*.h", "ext/**/*.c", "ext/#{project_name}/extconf.rb"]
		s.extensions = ["ext/#{project_name}/extconf.rb"]
		s.require_path = 'lib'
	end
  
	Gem::PackageTask.new(LINUX_SPEC) do |t|
		t.need_zip = false
		t.need_tar = false 
	end
end

task :release => [:'win:gem', :'mac:gem', :'linux:gem']
