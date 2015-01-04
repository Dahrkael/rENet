
Gem::Specification.new do |s|
  project_name = "renet"
  s.name = project_name
  s.version = "0.2.0"
  s.date = Time.now.strftime '%Y-%m-%d'
  s.author = "Dahrkael"
  s.email = "dark.wolf.warrior@gmail.com"
  s.homepage = "https://github.com/Dahrkael/rENet"
  s.summary = "Ruby library for games networking. Uses ENet as backend"
  s.description = s.summary
  s.require_path = "lib"
  s.has_rdoc = false
  s.files = Dir["README", "lib/**/*.rb", "examples/*.rb", "test/*.rb",
                "ext/**/*.h", "ext/**/*.c", "ext/#{project_name}/extconf.rb"]
  s.extensions = ["ext/#{project_name}/extconf.rb"]

  case RUBY_PLATFORM
    when /darwin/i
      s.platform = 'universal-darwin'
    else # linux and many others
      s.platform = 'ruby'
  end
end