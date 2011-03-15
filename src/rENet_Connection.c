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

#include "rENet_Connection.h"

VALUE cENetConnection;

void Init_rENet_Connection()
{
  cENetConnection = rb_define_class_under(mENet, "Connection", rb_cObject);
  rb_define_alloc_func(cENetConnection, renet_connection_allocate);
  
  rb_define_method(cENetConnection, "initialize", renet_connection_initialize, 5);
  rb_define_method(cENetConnection, "connect", renet_connection_connect, 1);
  rb_define_method(cENetConnection, "disconnect", renet_connection_disconnect, 1);
  rb_define_method(cENetConnection, "send_packet", renet_connection_send_packet, 3);
  rb_define_method(cENetConnection, "update", renet_connection_update, 1);
  
  rb_define_method(cENetConnection, "on_connection", renet_connection_on_connection, 1);
  rb_define_method(cENetConnection, "on_packet_receive", renet_connection_on_packet_receive, 1);
  rb_define_method(cENetConnection, "on_disconnection", renet_connection_on_disconnection, 1);
  
  rb_define_method(cENetConnection, "online", renet_connection_online, 0);
}

VALUE renet_connection_allocate(VALUE self)
{
  Connection* connection = malloc(sizeof(Connection));
  connection->event = malloc(sizeof(ENetEvent));
  connection->address = malloc(sizeof(ENetAddress));

  return Data_Wrap_Struct(self, NULL, renet_connection_deallocate, connection);
}

void renet_connection_deallocate(void* connection)
{
	free(((Connection*)connection)->event);
	free(((Connection*)connection)->address);
	enet_host_destroy(((Connection*)connection)->host);	
	free((Connection*)connection);
}

VALUE renet_connection_initialize(VALUE self, VALUE host, VALUE port, VALUE channels, VALUE download, VALUE upload)
{
	rb_funcall(mENet, rb_intern("initialize"), 0);
	Connection* connection;
	Data_Get_Struct(self, Connection, connection);
	Check_Type(host, T_STRING);
	if (enet_address_set_host(connection->address, StringValuePtr(host)) != 0)
	{
		rb_raise(rb_eStandardError, "Cannot set address");
	}
	connection->address->port = NUM2UINT(port);
	connection->channels = NUM2UINT(channels);
	connection->online = 0;
	connection->host = enet_host_create(NULL, 1, connection->channels, NUM2UINT(download), NUM2UINT(upload));
	if (connection->host == NULL)
    {
		rb_raise(rb_eStandardError, "Cannot create host");
	}
	
	return self;
}

VALUE renet_connection_connect(VALUE self, VALUE timeout)
{
	Connection* connection;
	Data_Get_Struct(self, Connection, connection);
	
	connection->peer = enet_host_connect(connection->host, connection->address, connection->channels, 0);    
    
    if (connection->peer == NULL)
    {
		rb_raise(rb_eStandardError, "Cannot connect to remote host");
    }
    
    if (enet_host_service(connection->host, connection->event, NUM2UINT(timeout)) > 0 && connection->event->type == ENET_EVENT_TYPE_CONNECT)
    {
		connection->online = 1;
		renet_connection_execute_on_connection();
        return Qtrue;
    }
    else
    {
        enet_peer_reset(connection->peer);
        return Qfalse;
    }
	
}

VALUE renet_connection_disconnect(VALUE self, VALUE timeout)
{
	Connection* connection;
	Data_Get_Struct(self, Connection, connection);
	
	enet_peer_disconnect(connection->peer, 0);

    while (enet_host_service(connection->host, connection->event, NUM2UINT(timeout)) > 0)
    {
        switch (connection->event->type)
        {
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy (connection->event->packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				connection->online = 0;
				return Qtrue;
        }
    }
    enet_peer_reset(connection->peer);
	connection->online = 0;
	return Qfalse;
}

VALUE renet_connection_send_packet(VALUE self, VALUE data, VALUE flag, VALUE channel)
{
	Connection* connection;
	Data_Get_Struct(self, Connection, connection);
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
    enet_peer_send(connection->peer, NUM2UINT(channel), packet);
   //enet_host_flush(connection->host);
   return Qnil;
}

VALUE renet_connection_update(VALUE self, VALUE timeout)
{
	Connection* connection;
	Data_Get_Struct(self, Connection, connection);
	if (connection->online == 0)
	{
		return Qfalse;
	}
    while (enet_host_service(connection->host, connection->event, NUM2UINT(timeout)) > 0)
    {
        switch (connection->event->type)
        {
			case ENET_EVENT_TYPE_NONE:
			
			break;
			
			case ENET_EVENT_TYPE_CONNECT:
				
            break;

			case ENET_EVENT_TYPE_RECEIVE:
					renet_connection_execute_on_packet_receive(connection->event->packet->data, connection->event->channelID);
					enet_packet_destroy(connection->event->packet);
            break;
           
			case ENET_EVENT_TYPE_DISCONNECT:
				connection->online = 0;
				renet_connection_execute_on_disconnection();
        }
    }
	return Qtrue;
}

VALUE renet_connection_on_connection(VALUE self, VALUE symbol)
{
	VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);
	rb_iv_set(cENetConnection, "@on_connection", method);
	return Qnil;
}

void renet_connection_execute_on_connection()
{
	VALUE method = rb_iv_get(cENetConnection, "@on_connection");
	if (method != Qnil)
	{
		rb_funcall(method, rb_intern("call"), 0);
	}
}

VALUE renet_connection_on_packet_receive(VALUE self, VALUE method)
{
	
	/*VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);*/
	rb_iv_set(cENetConnection, "@on_packet_receive", method);
	return Qnil;
}

/*VALUE renet_connection_on_packet_receive(int argc, VALUE *argv, VALUE self) 
{
	VALUE block = Qnil;

	rb_scan_args(argc, argv, "0&", &block);

    if (RTEST(block))
	{
		rb_iv_set(cENetConnection, "@on_packet_receive", block);
	}
    else
	{
		rb_raise(rb_eArgError, "a block is required");
	}
	return Qnil;
}*/

void renet_connection_execute_on_packet_receive(enet_uint8* data, enet_uint8 channelID)
{
	VALUE method = rb_iv_get(cENetConnection, "@on_packet_receive");
	if (method != Qnil)
	{
		rb_funcall(method, rb_intern("call"), 2, rb_str_new2(data), UINT2NUM(channelID));
	}
}

VALUE renet_connection_on_disconnection(VALUE self, VALUE symbol)
{
	VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);
	rb_iv_set(cENetConnection, "@on_disconnection", method);
	return Qnil;
}

void renet_connection_execute_on_disconnection()
{
	VALUE method = rb_iv_get(cENetConnection, "@on_disconnection");
	if (method != Qnil)
	{
		rb_funcall(method, rb_intern("call"), 0);
	}
}

VALUE renet_connection_online(VALUE self)
{
	Connection* connection;
	Data_Get_Struct(self, Connection, connection);
	if (connection->online == 1)
	{
		return Qtrue;
	}
	else
	{
		return Qfalse;
	}
}
