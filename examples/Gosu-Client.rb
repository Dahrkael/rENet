require 'gosu'
require './../bin/renet.so'

class GameWindow < Gosu::Window
	def initialize
		super(640, 480, false)
		self.caption = "Gosu & ENet Test"
		@font = Gosu::Font.new(self, Gosu::default_font_name, 20)
		@socket = ENet::Connection.new("localhost", 8000, 2, 0, 0)
		@socket.on_packet_receive(method(:on_packet))
		@packets = []
	end

	def update
		@socket.update(0)
		self.caption = "Gosu & ENet Test  -  [FPS: #{Gosu::fps.to_s}]"
	end

	def draw
		@font.draw("Connected: #{@socket.online?}", 10, 10, 0, 1.0, 1.0, 0xffffff00)
		@font.draw("Press C to connect   Press P to send a packet", 10, 440, 0, 1.0, 1.0, 0xffffff00)
		
		@font.draw("Packets Sent: #{@socket.total_sent_packets}", 450, 380, 0, 1.0, 1.0, 0xffffff00)
		@font.draw("Packets Recv: #{@socket.total_received_packets}", 450, 400, 0, 1.0, 1.0, 0xffffff00)
		@font.draw("Bytes Sent: #{@socket.total_sent_data}", 450, 420, 0, 1.0, 1.0, 0xffffff00)
		@font.draw("Bytes Recv: #{@socket.total_received_data}", 450, 440, 0, 1.0, 1.0, 0xffffff00)
		
		if !@packets.empty?
			@packets.each_index do |i|
				@font.draw("[PACKET] From channel #{@packets[i][1]}, containing \"#{@packets[i][0]}\"", 10, 30*(i+1), 0, 1.0, 1.0, @packets[i][2])
			end
		end
	end
  
	def button_down(id)
		case id
			when Gosu::KbC
				@socket.connect(2000) 
			when Gosu::KbP
				@socket.send_packet("test packet", true, 0)
		end
	end

	def on_packet(data, channel)
		@packets.slice!(0) if @packets.size >= 11
		@packets << [data, channel, Gosu::Color.argb(255, rand(100)+155, rand(100)+155, rand(100)+155)]
	end
end


window = GameWindow.new
window.show