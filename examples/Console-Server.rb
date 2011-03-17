require './../bin/renet.so'

def connections_handler(id, ip)
	puts "[ID #{id}] connected from #{ip}"
	@clients.delete_if {|key, value| key == id }
	@clients[id] = { :ip => ip }
	puts "Clients connected: #{@server.clients_count} of #{@server.max_clients}"
end

def packets_handler(id, data, channel)
	puts "packet from [peer #{id}] -> [#{data}] - [#{channel}]" 
	@server.send_packet(id, data, true, channel)
end

def disconnections_handler(id)
	puts "#{@clients[id][:ip]} with ID #{id} disconnected"
	@clients.delete_if {|key, value| key == id }
	@server.broadcast_packet("#{id} disconnected", true, 1)
end

@clients = {}
@server = ENet::Server.new(8000, 32, 2, 0, 0)

@server.on_connection(method(:connections_handler))
@server.on_packet_receive(method(:packets_handler))
@server.on_disconnection(method(:disconnections_handler))

loop do
	@server.update(1000)
end
