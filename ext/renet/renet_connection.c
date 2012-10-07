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

#include "renet_connection.h"

void init_renet_connection()
{
  VALUE cENetConnection = rb_define_class_under(mENet, "Connection", rb_cObject);
  rb_define_alloc_func(cENetConnection, renet_connection_allocate);
  
  rb_define_method(cENetConnection, "initialize",          renet_connection_initialize, 5);
  rb_define_method(cENetConnection, "connect",             renet_connection_connect, 1);
  rb_define_method(cENetConnection, "disconnect",          renet_connection_disconnect, 1);
  rb_define_method(cENetConnection, "send_packet",         renet_connection_send_packet, 3);
  rb_define_method(cENetConnection, "send_queued_packets", renet_connection_send_queued_packets, 0);
  rb_define_method(cENetConnection, "flush",               renet_connection_send_queued_packets, 0);
  rb_define_method(cENetConnection, "update",              renet_connection_update, 1);
  rb_define_method(cENetConnection, "use_compression",     renet_connection_use_compression, 1);
  
  rb_define_method(cENetConnection, "on_connection",     renet_connection_on_connection, 1);
  rb_define_method(cENetConnection, "on_packet_receive", renet_connection_on_packet_receive, 1);
  rb_define_method(cENetConnection, "on_disconnection",  renet_connection_on_disconnection, 1);
  
  rb_define_method(cENetConnection, "online?",    renet_connection_online, 0);
  rb_define_method(cENetConnection, "connected?", renet_connection_online, 0);

  rb_define_attr(cENetConnection, "total_sent_data", 1, 1);
  rb_define_attr(cENetConnection, "total_received_data", 1, 1);
  rb_define_attr(cENetConnection, "total_sent_packets", 1, 1);
  rb_define_attr(cENetConnection, "total_received_packets", 1, 1);
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

  /* Now that we're releasing the GIL while waiting for enet_host_service 
     we need a lock to prevent potential segfaults if another thread tries
     to do an operation on same connection  */
  VALUE lock = rb_mutex_new();
  rb_iv_set(self, "@lock", lock); 
  rb_mutex_lock(lock);

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
  rb_iv_set(self, "@total_sent_data", INT2FIX(0)); 
  rb_iv_set(self, "@total_received_data", INT2FIX(0));
  rb_iv_set(self, "@total_sent_packets", INT2FIX(0));
  rb_iv_set(self, "@total_received_packets", INT2FIX(0));

  rb_mutex_unlock(lock);
  return self;
}



/* These let us release the global interpreter lock if while we're waiting for
   enet_host_service to finish:

   CallbackData
   do_service
   service
*/
typedef struct
{
  Connection * connection;
  enet_uint32 timeout;
} CallbackData;

static VALUE do_service(void *data)
{
  CallbackData* temp_data = data;
  return enet_host_service(temp_data->connection->host, temp_data->connection->event, temp_data->timeout);
}

static VALUE service(VALUE self, Connection* connection, enet_uint32 timeout)
{
  CallbackData data = {connection, timeout};
  if (timeout > 0)
  {
    return rb_thread_blocking_region(do_service, &data, RUBY_UBF_IO, 0);
  }
  else
  {
    return do_service(&data);
  }
}


/* The core API functions for a connection: 
  renet_connection_connect
  renet_connection_disconnect
  renet_connection_send_packet
  renet_connection_send_queued_packets
  renet_connection_update
  renet_connection_use_compression
  renet_connection_online
*/
VALUE renet_connection_connect(VALUE self, VALUE timeout)
{
  Connection* connection;
  Data_Get_Struct(self, Connection, connection);
  VALUE lock = rb_iv_get(self, "@lock");
  rb_mutex_lock(lock);

  VALUE rv = Qfalse;

  if (connection->online == 0)
  {
    connection->peer = enet_host_connect(connection->host, connection->address, connection->channels, 0);    
      
    if (connection->peer == NULL)
    {
      rb_raise(rb_eStandardError, "Cannot connect to remote host");
    }
    
    if (service(self, connection, NUM2UINT(timeout)) > 0 && connection->event->type == ENET_EVENT_TYPE_CONNECT)
    {
      connection->online = 1;
      renet_connection_execute_on_connection(self);
      rv = Qtrue;
    }
    else
    {
      enet_peer_reset(connection->peer);
    }
  }

  rb_mutex_unlock(lock);
  return rv;
}

VALUE renet_connection_disconnect(VALUE self, VALUE timeout)
{
  Connection* connection;
  Data_Get_Struct(self, Connection, connection);
  VALUE lock = rb_iv_get(self, "@lock");
  rb_mutex_lock(lock);

  VALUE rv = Qfalse;

  if (connection->online == 0)
  {
    rv = Qtrue;
  }
  else
  {      
    connection->online = 0;
    enet_peer_disconnect(connection->peer, 0);

    while (service(self, connection, NUM2UINT(timeout)) > 0)
    {
      switch (connection->event->type)
      {
      case ENET_EVENT_TYPE_NONE:
        break;
      case ENET_EVENT_TYPE_CONNECT:
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        enet_packet_destroy (connection->event->packet);
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        rv = Qtrue;
        break;
      }
    }
    if (rv != Qtrue) { enet_peer_disconnect_now(connection->peer, 0); }
  }
  rb_mutex_unlock(lock);
  return rv;
}

VALUE renet_connection_send_packet(VALUE self, VALUE data, VALUE flag, VALUE channel)
{
  Connection* connection;
  Data_Get_Struct(self, Connection, connection);
  VALUE lock = rb_iv_get(self, "@lock");
  rb_mutex_lock(lock);

  Check_Type(data, T_STRING);
  char* cdata = StringValuePtr(data);
  ENetPacket* packet;
  if (connection->online != 0)
  {
    if (flag == Qtrue)
    {
      packet = enet_packet_create(cdata, RSTRING_LEN(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    }
    else
    {
      packet = enet_packet_create(cdata, RSTRING_LEN(data) + 1, 0);
    }
    enet_peer_send(connection->peer, NUM2UINT(channel), packet);
  }

  rb_mutex_unlock(lock);
  return Qnil;
}

VALUE renet_connection_send_queued_packets(VALUE self)
{
  Connection* connection;
  Data_Get_Struct(self, Connection, connection);
  VALUE lock = rb_iv_get(self, "@lock");
  rb_mutex_lock(lock);

  if (connection->online != 0)
  {
    enet_host_flush(connection->host);
  }

  rb_mutex_unlock(lock);
  return Qnil;
}

VALUE renet_connection_update(VALUE self, VALUE timeout)
{
  Connection* connection;
  Data_Get_Struct(self, Connection, connection);
  VALUE lock = rb_iv_get(self, "@lock");
  rb_mutex_lock(lock);

  VALUE rv = Qfalse;

  if (connection->online != 0)
  {
    /* wait up to timeout milliseconds for a packet */
    if (service(self, connection, NUM2UINT(timeout)) > 0)
    {
      do
      {
        switch (connection->event->type)
        {
        case ENET_EVENT_TYPE_NONE:
          break;
        case ENET_EVENT_TYPE_CONNECT:
          break;
        case ENET_EVENT_TYPE_RECEIVE:
          renet_connection_execute_on_packet_receive(self, connection->event->packet, connection->event->channelID);
          break;
        case ENET_EVENT_TYPE_DISCONNECT:
          connection->online = 0;
          renet_connection_execute_on_disconnection(self);
          break;
        }
      }
      /* Do not use a timeout for subsequent services or we could get stuck 
         here forever */
      while ((connection->online != 0) && (service(self, connection, 0) > 0));
    }

    rv = Qtrue;
  }

  /* we are unlocking now because it's important to unlock before going 
    back into ruby land (which rb_funcall will do). If we don't then an
    exception can leave the locks in an inconsistent state */
  rb_mutex_unlock(lock);

  if (rv == Qtrue)
  {
    {
      VALUE total = rb_iv_get(self, "@total_sent_data");
      VALUE result = rb_funcall( total
                               , rb_intern("+")
                               , 1
                               , UINT2NUM(connection->host->totalSentData));
      rb_iv_set(self, "@total_sent_data", result); 
      connection->host->totalSentData = 0;
    }

    {
      VALUE total = rb_iv_get(self, "@total_received_data");
      VALUE result = rb_funcall( total
                               , rb_intern("+")
                               , 1
                               , UINT2NUM(connection->host->totalReceivedData));
      rb_iv_set(self, "@total_received_data", result);
      connection->host->totalReceivedData = 0;
    }

    {
      VALUE total = rb_iv_get(self, "@total_sent_packets");
      VALUE result = rb_funcall( total
                               , rb_intern("+")
                               , 1
                               , UINT2NUM(connection->host->totalSentPackets));
      rb_iv_set(self, "@total_sent_packets", result);
      connection->host->totalSentPackets = 0;
    }

    {
      VALUE total = rb_iv_get(self, "@total_received_packets");
      VALUE result = rb_funcall( total
                               , rb_intern("+")
                               , 1
                               , UINT2NUM(connection->host->totalReceivedPackets));
      rb_iv_set(self, "@total_received_packets", result);
      connection->host->totalReceivedPackets = 0;
    }
  }
  
  return rv;
}

VALUE renet_connection_use_compression(VALUE self, VALUE flag)
{
  Connection* connection;
  Data_Get_Struct(self, Connection, connection);
  VALUE lock = rb_iv_get(self, "@lock");
  rb_mutex_lock(lock);

  if (flag == Qtrue)
  {
    enet_host_compress_with_range_coder(connection->host);
  }
  else
  {
    enet_host_compress(connection->host, NULL);
  }

  rb_mutex_unlock(lock);
  return Qnil;
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

/* These call our callbacks and take us back into ruby land.
   They also unlock the mutex for the duration of our stay. we have to be
   careful to make this safe. For example if we didn't set connection->online
   = 1 before calling renet_connection_execute_on_connection we could possibly
   segfault if we called connect from inside on_connection 

  renet_connection_execute_on_connection
  renet_connection_execute_on_disconnection
  renet_connection_execute_on_packet_receive
*/

void renet_connection_execute_on_connection(VALUE self)
{
  VALUE method = rb_iv_get(self, "@on_connection");
  if (method != Qnil)
  {
    VALUE lock = rb_iv_get(self, "@lock");
    rb_mutex_unlock(lock);
    rb_funcall(method, rb_intern("call"), 0);
    rb_mutex_lock(lock);
  }
}

void renet_connection_execute_on_disconnection(VALUE self)
{
  VALUE method = rb_iv_get(self, "@on_disconnection");
  if (method != Qnil)
  {
    VALUE lock = rb_iv_get(self, "@lock");
    rb_mutex_unlock(lock);
    rb_funcall(method, rb_intern("call"), 0);
    rb_mutex_lock(lock);
  }
}

void renet_connection_execute_on_packet_receive(VALUE self, ENetPacket * const packet, enet_uint8 channelID)
{
  VALUE method  = rb_iv_get(self, "@on_packet_receive");
  VALUE data    = rb_str_new((char const *)packet->data, packet->dataLength);
  /* marshal data and then destroy packet 
     if we don't do this now the packet might become invalid before we get
     back and we'd get a segfault when we attempt to destroy */
  enet_packet_destroy(packet); 

  if (method != Qnil)
  {
    VALUE lock = rb_iv_get(self, "@lock");
    rb_mutex_unlock(lock);
    rb_funcall(method, rb_intern("call"), 2, data, UINT2NUM(channelID));
    rb_mutex_lock(lock);
  }
}


/* These set the callbacks to call on important events:
  renet_connection_on_connection
  renet_connection_on_packet_receive
  renet_connection_on_disconnection
*/

VALUE renet_connection_on_connection(VALUE self, VALUE method)
{
  /*VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);*/
  rb_iv_set(self, "@on_connection", method);
  return Qnil;
}


VALUE renet_connection_on_packet_receive(VALUE self, VALUE method)
{
  
  /*VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);*/
  rb_iv_set(self, "@on_packet_receive", method);
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

VALUE renet_connection_on_disconnection(VALUE self, VALUE method)
{
  /*VALUE method = rb_funcall(rb_cObject, rb_intern("method"), 1, symbol);*/
  rb_iv_set(self, "@on_disconnection", method);
  return Qnil;
}




