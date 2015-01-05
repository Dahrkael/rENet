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

#include "renet.h"

VALUE mENet;

void Init_renet()
{
  mENet = rb_define_module("ENet");
  rb_define_singleton_method(mENet, "initialize", renet_main_initialize, 0);
  rb_define_singleton_method(mENet, "deinitialize", renet_main_deinitialize, 0);
  rb_cv_set(mENet, "@@initialized", Qfalse); 
  /*Init_Constants();*/
  init_renet_connection();
  init_renet_server();
  rb_define_const(mENet, "ENET_VERSION", rb_str_new2("1.3.0"));
  rb_define_const(mENet, "RENET_VERSION", rb_str_new2("0.1.13"));
}

VALUE renet_main_initialize(VALUE self)
{
	if (rb_cv_get(mENet, "@@initialized") == Qfalse)
	{
		if (enet_initialize() != 0)
		{
			return Qfalse;
		}
		rb_cv_set(mENet, "@@initialized", Qtrue); 
		return Qtrue;
	}
  return Qtrue;
}

VALUE renet_main_deinitialize(VALUE self)
{
	if (rb_cv_get(mENet, "@@initialized") == Qtrue)
	{
		enet_deinitialize();
	}
	rb_cv_set(mENet, "@@initialized", Qfalse); 
	return Qtrue;
}
/*
void Init_Constants()
{
	rb_define_const(mENet, "ENET_HOST_ANY", UINT2NUM(ENET_HOST_ANY));
	rb_define_const(mENet, "ENET_HOST_BROADCAST", UINT2NUM(ENET_HOST_BROADCAST));
	rb_define_const(mENet, "ENET_PORT_ANY", UINT2NUM(ENET_PORT_ANY));

	rb_define_const(mENet, "ENET_PACKET_FLAG_RELIABLE", UINT2NUM(ENET_PACKET_FLAG_RELIABLE));
	rb_define_const(mENet, "ENET_PACKET_FLAG_UNSEQUENCED", UINT2NUM(ENET_PACKET_FLAG_UNSEQUENCED));
	rb_define_const(mENet, "ENET_PACKET_FLAG_NO_ALLOCATE", UINT2NUM(ENET_PACKET_FLAG_NO_ALLOCATE));
}
*/
