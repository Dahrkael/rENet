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

#ifndef RUBY_ENET_CONNECTION
#define RUBY_ENET_CONNECTION

#include "renet.h"

typedef struct {
   ENetHost*    host;
   ENetPeer*    peer;
   ENetEvent*   event;
   ENetAddress* address;
   int          channels;
   int  		online;
} Connection;

void init_renet_connection();

VALUE renet_connection_allocate(VALUE self);
void renet_connection_deallocate(void* connection);

VALUE renet_connection_initialize(VALUE self, VALUE host, VALUE port, VALUE channels, VALUE download, VALUE upload);
VALUE renet_connection_connect(VALUE self, VALUE timeout);
VALUE renet_connection_disconnect(VALUE self, VALUE timeout);
VALUE renet_connection_send_packet(VALUE self, VALUE data, VALUE flag, VALUE channel);
VALUE renet_connection_send_queued_packets(VALUE self);
VALUE renet_connection_update(VALUE self, VALUE timeout);
VALUE renet_connection_use_compression(VALUE self, VALUE flag);

VALUE renet_connection_on_connection(VALUE self, VALUE method);
void renet_connection_execute_on_connection();

VALUE renet_connection_on_packet_receive(VALUE self, VALUE method);
/*VALUE renet_connection_on_packet_receive(int argc, VALUE *argv, VALUE self);*/
void renet_connection_execute_on_packet_receive(enet_uint8* data, enet_uint8 channelID);

VALUE renet_connection_on_disconnection(VALUE self, VALUE method);
void renet_connection_execute_on_disconnection();

VALUE renet_connection_online(VALUE self);

#endif
