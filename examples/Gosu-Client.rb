# require gosu and renet libs!
require 'rubygems'
require 'gosu'
require 'renet'

# define a new window class
class GameWindow < Gosu::Window
	def initialize
		super(640, 480, false)
		# set a descriptive caption
		self.caption = "Gosu & ENet Test"
		# create a font to draw the information
		@font = Gosu::Font.new(self, Gosu::default_font_name, 20)
		# create a new connection object (remote host address, remote host port, channels, download bandwidth, upload bandwidth)
		@socket = ENet::Connection.new("localhost", 8000, 2, 0, 0)
		# set the handler for the packets
		@socket.on_packet_receive(method(:on_packet))
		# an array to store the incoming packets
		@packets = []
	end

	def update
		# send queued packets and receive incoming ones (0 = non-blocking)
		@socket.update(0)
		# show our client FPS
		self.caption = "Gosu & ENet Test  -  [FPS: #{Gosu::fps.to_s}]"
	end

	def draw
		# show if we are connected to the server or not (@socket.connected? is valid too)
		@font.draw("Connected: #{@socket.online?}", 10, 10, 0, 1.0, 1.0, 0xffffff00)
		# show the controls
		@font.draw("Press C to connect   Press P to send a packet", 10, 440, 0, 1.0, 1.0, 0xffffff00)
		# Show the socket information
		@font.draw("Packets Sent: #{@socket.total_sent_packets}", 450, 380, 0, 1.0, 1.0, 0xffffff00)
		@font.draw("Packets Recv: #{@socket.total_received_packets}", 450, 400, 0, 1.0, 1.0, 0xffffff00)
		@font.draw("Bytes Sent: #{@socket.total_sent_data}", 450, 420, 0, 1.0, 1.0, 0xffffff00)
		@font.draw("Bytes Recv: #{@socket.total_received_data}", 450, 440, 0, 1.0, 1.0, 0xffffff00)
		
		# show the storaged packets
		if !@packets.empty?
			@packets.each_index do |i|
				@font.draw("[PACKET] From channel #{@packets[i][1]}, containing \"#{@packets[i][0]}\"", 10, 30*(i+1), 0, 1.0, 1.0, @packets[i][2])
			end
		end
	end
  
	def button_down(id)
		case id
			# connect to the server if we press C
			when Gosu::KbC
				# only connects if we are not already connected
				@socket.connect(2000) 
			# send a packet if we press P
			when Gosu::KbP
				# send a packet to the server (data, reliable or not, channel ID)
				@socket.send_packet("test packet", true, 0)
		end
	end
	
	# define a custom handler to manage the packets
	def on_packet(data, channel)
		# delete the old packets if we get more than 10
		@packets.slice!(0) if @packets.size >= 11
		#storage the packet data, channel and a representative color
		@packets << [data, channel, Gosu::Color.argb(255, rand(100)+155, rand(100)+155, rand(100)+155)]
	end
end

# show the window
window = GameWindow.new
window.show