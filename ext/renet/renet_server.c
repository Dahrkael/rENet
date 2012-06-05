/*
 * Copyright (c) 2011 Dahrkael <dark.wolf.warrior at gmail.com>
 * Permission is hereby granted, free of charge, 
 * to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, 
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/

#include "renet_server.h"

VALUE cENetServer;

void init_renet_server()
{
  cENetServer = rb_define_class_under(mENet, "Server", rb_cObject);
  rb_define_alloc_func(cENetServer, renet_server_allocate);
  
  rb_define_method(cENetServer, "initialize", renet_server_initialize, 5);
  rb_define_method(cENetServer, "disconnect_client", renet_server_disconnect_client, 1);
  rb_define_method(cENetServer, "send_packet", renet_server_send_packet, 4);
  rb_define_method(cENetServer, "broadcast_packet", renet_server_broadcast_packet, 3);
  rb_define_method(cENetServer, "send_queued_packets", renet_server_send_queued_packets, 0);
  rb_define_method(cENetServer, "flush", renet_server_send_queued_packets, 0);
  rb_define_method(cENetServer, "update", renet_server_update, 1);
  rb_define_method(cENetServer, "use_compression", renet_server_use_compression, 1);
  
  rb_define_method(cENetServer, "on_connection", renet_server_on_connection, 1);
  rb_define_method(cENetServer, "on_packet_receive", renet_server_on_packet_receive, 1);
  rb_define_method(cENetServer, "on_disconnection", renet_server_on_disconnection, 1);
  
  rb_define_method(cENetServer, "max_clients", renet_server_max_clients, 0);
  rb_define_method(cENetServer, "clients_count", renet_server_clients_count, 0);
}

VALUE renet_server_allocate(VALUE self)
{
  Server* server = malloc(sizeof(Server));
  server->event = malloc(sizeof(ENetEvent));
  server->address = malloc(sizeof(ENetAddress));
  server->conn_ip = malloc(sizeof(char)*30);

  return Data_Wrap_Struct(self, NULL, renet_server_deallocate, server);
}

void renet_server_deallocate(void* server)
{
	free(((Server*)server)->event);
	free(((Server*)server)->address);
	enet_host_destroy(((Server*)server)->host);	
	free((Server*)server);
}

VALUE renet_server_initialize(VALUE self, VALUE port, VALUE n_peers, VALUE channels, VALUE download, VALUE upload)
{
	rb_funcall(mENet, rb_intern("initialize"), 0);
	Server* server;
	Data_Get_Struct(self, Server, server);
	
	server->address->host = ENET_HOST_ANY;
	server->address->port = NUM2UINT(port);
	server->channels = NUM2UINT(channels);
	server->host = enet_host_create(server->address, NUM2UINT(n_peers), server->channels, NUM2UINT(download), NUM2UINT(upload));
	if (server->host == NULL)
    {
		rb_raise(rb_eStandardError, "Cannot create server");
	}
	rb_iv_set(self, "@total_sent_data", INT2FIX(0)); 
	rb_iv_set(self, "@total_received_data", INT2FIX(0));
	rb_iv_set(self, "@total_sent_packets", INT2FIX(0));
	rb_iv_set(self, "@total_received_packets", INT2FIX(0)); 
	
	rb_define_attr(cENetServer, "total_sent_data", 1, 1);
	rb_define_attr(cENetServer, "total_received_data", 1, 1);
	rb_define_attr(cENetServer, "total_sent_packets", 1, 1);
	rb_define_attr(cENetServer, "total_received_packets", 1, 1);
	
	return self;
}

VALUE renet_server_disconnect_client(VALUE self, VALUE peer_id)
{
	Server* server;
	Data_Get_Struct(self, Server, server);
	enet_peer_disconnect_now(&(server->host->peers[NUM2UINT(peer_id)]), 0);
	renet_server_execute_on_disconnection(peer_id);
	return Qtrue;
}

VALUE renet_server_send_packet(VALUE self, VALUE peer_id, VALUE data, VALUE flag, VALUE channel)
{
	Server* server;
	Data_Get_Struct(self, Server, server);
	Check_Type(data, T_STRING);
	char* cdata = StringValuePtr(data);
	ENetPacket* packet;
	if (flag == Qtrue)
	{
		packet = enet_packet_create(cdata, RSTRING_LEN(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	}
	else
	{
		packet = enet_packet_create(cdata, RSTRING_LEN(data) + 1, 0);
	}
    enet_peer_send(&(server->host->peers[NUM2UINT(peer_id)]), NUM2UINT(channel), packet);
   return Qnil;
}

VALUE renet_server_broadcast_packet(VALUE self, VALUE data, VALUE flag, VALUE channel)
{
	Server* server;
	Data_Get_Struct(self, Server, server);
	Check_Type(data, T_STRING);
	char* cdata = StringValuePtr(data);
	ENetPacket* packet;
	if (flag == Qtrue)
	{
		packet = enet_packet_create(cdata, RSTRING_LEN(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	}
	else
	{
		packet = enet_packet_create(cdata, RSTRING_LEN(data) + 1, 0);
	}
    enet_host_broadcast(server->host, NUM2UINT(channel), packet);
   return Qnil;
}

VALUE renet_server_send_queued_packets(VALUE self)
{
	Server* server;
	Data_Get_Struct(self, Server, server);
    enet_host_flush(server->host);
    return Qnil;
}

VALUE renet_server_update(VALUE self, VALUE timeout)
{
	Server* server;
	Data_Get_Struct(self, Server, server);
	int peer_id;
	
    while (enet_host_service(server->host, server->event, NUM2UINT(timeout)) > 0)
    {
        switch (server->event->type)
        {
			case ENET_EVENT_TYPE_NONE:
			
			break;
			
			case ENET_EVENT_TYPE_CONNECT:
				server->n_clients += 1;
				enet_address_get_host_ip(&(server->event->peer->address), server->conn_ip, 20);
				peer_id = (int)(server->event->peer - server->host->peers);
				renet_server_execute_on_connection(INT2NUM(peer_id), rb_str_new2(server->conn_ip));
            break;

			case ENET_EVENT_TYPE_RECEIVE:
				peer_id = (int)(server->event->peer - server->host->peers);
				renet_server_execute_on_packet_receive(INT2NUM(peer_id), server->event->packet->dataLength, server->event->packet->data, server->event->channelID);
				enet_packet_destroy(server->event->packet);
            break;
           
			case ENET_EVENT_TYPE_DISCONNECT:
				server->n_clients -= 1;
				peer_id = (int)(server->event->peer - server->host->peers);
				renet_server_execute_on_disconnection(INT2NUM(peer_id));
        }
    }

	int tmp;
	tmp = NUM2INT(rb_iv_get(self, "@total_sent_data"));
	tmp = tmp + server->host->totalSentData;
	server->host->totalSentData = 0;
	rb_iv_set(self, "@total_sent_data", UINT2NUM(tmp)); 
	
	tmp = NUM2INT(rb_iv_get(self, "@total_received_data"));
	tmp = tmp + server->host->totalReceivedData;
	server->host->totalReceivedData = 0;
	rb_iv_set(self, "@total_received_data", UINT2NUM(tmp)); 
	
	tmp = NUM2INT(rb_iv_get(self, "@total_sent_packets"));
	tmp = tmp + server->host->totalSentPackets;
	server->host->totalSentPackets = 0;
	rb_iv_set(self, "@total_sent_packets", UINT2NUM(tmp)); 
	
	tmp = NUM2INT(rb_iv_get(self, "@total_received_packets"));
	tmp = tmp + server->host->totalReceivedPackets;
	server->host->totalReceivedPackets = 0;
	rb_iv_set(self, "@total_received_packets", UINT2NUM(tmp)); 
	
	return Qtrue;
}

VALUE renet_server_use_compression(VALUE self, VALUE flag)
{
	Server* server;
	Data_Get_Struct(self, Server, server);
	if (flag == Qtrue)
	{
		enet_host_compress_with_range_coder(server->host);
	}
	else
	{
		enet_host_compress(server->host, NULL);
	}
	return Qnil;
}

VALUE renet_server_on_connection(VALUE self, VALUE method)
{
	/*VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);*/
	rb_iv_set(cENetServer, "@on_connection", method);
	return Qnil;
}

void renet_server_execute_on_connection(VALUE peer_id, VALUE ip)
{
	VALUE method = rb_iv_get(cENetServer, "@on_connection");
	if (method != Qnil)
	{
		rb_funcall(method, rb_intern("call"), 2, peer_id, ip);
	}
}

VALUE renet_server_on_packet_receive(VALUE self, VALUE method)
{
	
	/*VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);*/
	rb_iv_set(cENetServer, "@on_packet_receive", method);
	return Qnil;
}

void renet_server_execute_on_packet_receive(VALUE peer_id, size_t data_length, enet_uint8* data, enet_uint8 channelID)
{
	VALUE method = rb_iv_get(cENetServer, "@on_packet_receive");
	if (method != Qnil)
	{
		rb_funcall(method, rb_intern("call"), 3, peer_id, rb_str_new(data, data_length), UINT2NUM(channelID));
	}
}

VALUE renet_server_on_disconnection(VALUE self, VALUE method)
{
	/*VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);*/
	rb_iv_set(cENetServer, "@on_disconnection", method);
	return Qnil;
}

void renet_server_execute_on_disconnection(VALUE peer_id)
{
	VALUE method = rb_iv_get(cENetServer, "@on_disconnection");
	if (method != Qnil)
	{
		rb_funcall(method, rb_intern("call"), 1, peer_id);
	}
}

VALUE renet_server_max_clients(VALUE self)
{
	Server* server;
	Data_Get_Struct(self, Server, server);
	return UINT2NUM(server->host->peerCount);
}

VALUE renet_server_clients_count(VALUE self)
{
	Server* server;
	Data_Get_Struct(self, Server, server);
	return UINT2NUM(server->n_clients);
}

