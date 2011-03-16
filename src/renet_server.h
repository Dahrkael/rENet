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
/* Peer_Index = (int)(event.peer - server->peers); */
 
#ifndef RUBY_ENET_SERVER
#define RUBY_ENET_SERVER

#include "renet.h"

typedef struct {
   ENetHost*    host;
   ENetEvent*   event;
   ENetAddress* address;
   int          channels;
   char* 	    conn_ip;
} Server;

void init_renet_server();

VALUE renet_server_allocate(VALUE self);
void renet_server_deallocate(void* server);

VALUE renet_server_initialize(VALUE self, VALUE port, VALUE n_peers, VALUE channels, VALUE download, VALUE upload);
VALUE renet_server_disconnect_client(VALUE self, VALUE peer_id);
VALUE renet_server_send_packet(VALUE self, VALUE peer_id, VALUE data, VALUE flag, VALUE channel);
VALUE renet_server_broadcast_packet(VALUE self, VALUE data, VALUE flag, VALUE channel);
VALUE renet_server_send_queued_packets(VALUE self);
VALUE renet_server_update(VALUE self, VALUE timeout);

VALUE renet_server_on_connection(VALUE self, VALUE method);
void renet_server_execute_on_connection(VALUE peer_id, VALUE ip);

VALUE renet_server_on_packet_receive(VALUE self, VALUE method);
void renet_server_execute_on_packet_receive(VALUE peer_id, enet_uint8* data, enet_uint8 channelID);

VALUE renet_server_on_disconnection(VALUE self, VALUE method);
void renet_server_execute_on_disconnection(VALUE peer_id);

#endif
