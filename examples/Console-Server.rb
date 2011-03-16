require './../bin/renet.so'

def connection(id, ip)
	puts "[ID #{id}] connected from #{ip}"
end
def packet(id, data, channel)
	puts "packet from [peer #{id}] -> [#{data}] - [#{channel}]" 
	@server.send_packet(id, data, true, channel)
end

@server = ENet::Server.new(8000, 32, 2, 0, 0)

@server.on_connection(method(:connection))
@server.on_packet_receive(method(:packet))

loop do
	@server.update(1000)
end
