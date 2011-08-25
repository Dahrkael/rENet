# require the lib!
require 'rubygems'
require 'renet'

# define a custom handler for new client connections, it will be automatically called
def connections_handler(id, ip)
	# show who connected
	puts "[ID #{id}] connected from #{ip}"
	# delete the hash entry if it already exists
	@clients.delete_if {|key, value| key == id }
	# add the new entry to the clients hash
	@clients[id] = { :ip => ip }
	#show how many clients we have
	puts "Clients connected: #{@server.clients_count} of #{@server.max_clients}"
end

# define a custom handler for incoming data packets, it will be automatically called
def packets_handler(id, data, channel)
	# show who sent the packet, the data inside and the channel used
	puts "packet from [peer #{id}] -> [#{data}] - [#{channel}]" 
	# answer sending the same data to the client (client ID, data, reliable or not, channel ID)
	@server.send_packet(id, data, true, channel)
end

# define a custom handler for client disconnections, it will be automatically called
def disconnections_handler(id)
	# show who left
	puts "#{@clients[id][:ip]} with ID #{id} disconnected"
	# delete its entry in the clients hash
	@clients.delete_if {|key, value| key == id }
	# sends a packet to all clients informing about the disconnection (data, reliable or not, channel ID)
	@server.broadcast_packet("#{id} disconnected", true, 1)
end
# create a hash to store clients information
@clients = {}
# create a new server object ( port, clients allowed, number of channels, download bandwidth, upload badnwidth)
@server = ENet::Server.new(8000, 32, 2, 0, 0)

# Set our handlers to the server
@server.on_connection(method(:connections_handler))
@server.on_packet_receive(method(:packets_handler))
@server.on_disconnection(method(:disconnections_handler))

# loop forever
loop do
	# send queued packets and wait for new packets 1000 milliseconds
	@server.update(1000)
end
