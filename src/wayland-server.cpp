/*
 * Copyright (c) 2016-2017, Yisu Peng
 * Copyright (c) 2014-2016, Nils Christopher Brause
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <wayland-server.hpp>
//#include <wayland-server-protocol.hpp>

using namespace wayland;
using namespace wayland::detail;

display_t::display_t() {
	display = wl_display_create();
}

void global_bind(wl_client *client, void *data, uint32_t version, uint32_t id) {
}


// event_queue_t::queue_ptr::~queue_ptr() {
// 	if(queue)
// 		wl_event_queue_destroy(queue);
// }
// 
// event_queue_t::event_queue_t(wl_event_queue *q)
// 	: queue(new queue_ptr({ q })) {
// }
// 
// wl_event_queue *event_queue_t::c_ptr() {
// 	if(!queue)
// 		throw std::invalid_argument("event_queue is NULL");
// 	return queue->queue;
// };

resource_t::resource_t()
	: object_t(NULL), resource(NULL), data(NULL), display(false), interface(NULL) {
}

resource_t::resource_t(wl_resource *p, bool is_display, bool donotdestroy)
	: object_t((wl_object *)p), resource(p), data(NULL), display(is_display), dontdestroy(donotdestroy), interface(NULL) {
	if(!display) {
		data = reinterpret_cast<resource_data_t*>(wl_resource_get_user_data(c_ptr()));
		if(!data) {
			data = new resource_data_t{std::shared_ptr<requests_base_t>(), 0};
			wl_resource_set_user_data(resource, data);
		}
		data->counter++;
	}
}

resource_t::resource_t(const resource_t &p) : object_t(p) {
	operator=(p);
}

resource_t &resource_t::operator=(const resource_t& p) {
	//object_t::operator=(object_t((wl_object *)p.resource));
	//object_t::operator=((wl_object *)p.resource);
	object_t::operator=(p);
	resource = p.resource;
	data = p.data;
	interface = p.interface;
	copy_constructor = p.copy_constructor;
	display = p.display;
	dontdestroy = p.dontdestroy;

	if(!data) {
		std::cerr << "Found resource_t without meta data." << std::endl;
		data = new resource_data_t{std::shared_ptr<requests_base_t>(), 0};
		wl_resource_set_user_data(resource, data);
	}
	data->counter++;

	return *this;
}

resource_t::resource_t(resource_t &&p)
	: /*object_t(std::move(p))*/object_t(NULL), resource(NULL), data(NULL), display(false), dontdestroy(false), interface(NULL) {
	operator=(std::move(p));
}

resource_t &resource_t::operator=(resource_t &&p) {
	object_t::operator=(std::move(p));
	std::swap(resource, p.resource);
	std::swap(data, p.data);
	std::swap(display, p.display);
	std::swap(dontdestroy, p.dontdestroy);
	std::swap(interface, p.interface);
	std::swap(copy_constructor, p.copy_constructor);
	return *this;
}

resource_t::~resource_t() {
	if(resource && !display) {
		data->counter--;
		if(data->counter == 0) {
			if(!dontdestroy) {
				//if(data->destroy_opcode >= 0) {
				//	wl_resource_marshal(resource, data->destroy_opcode);
				//}
				wl_resource_destroy(resource);
			}
			delete data;
		}
	}
}

uint32_t resource_t::get_id() {
	return wl_resource_get_id(c_ptr());
}

std::string resource_t::get_class() {
	return wl_resource_get_class(c_ptr());
}

//void resource_t::set_queue(event_queue_t queue) {
//	wl_resource_set_queue(c_ptr(), queue.c_ptr());
//}

wl_resource *resource_t::c_ptr() {
	if(!resource)
		throw std::invalid_argument("resource is NULL");
	return resource;
}

resource_t::resource_data_t::resource_data_t() : requests(NULL) {
}

resource_t::resource_data_t::resource_data_t(std::shared_ptr<requests_base_t> ev,
		unsigned int cnt)
	: requests(ev), counter(cnt) {
}


void resource_t::set_requests(std::shared_ptr<requests_base_t> requests,
		int(*dispatcher)(int, std::vector<any>, std::shared_ptr<resource_t::requests_base_t>)) {
	if(!display && !data->requests) {
		data->requests = requests;
		// the dispatcher gets 'implemetation'
		wl_resource_set_dispatcher(resource, c_dispatcher, reinterpret_cast<void *>(dispatcher), data, c_destroy);
	}
}

int resource_t::c_dispatcher(const void *implementation, void *target, uint32_t opcode, const wl_message *message, wl_argument *args) {
	if(!implementation)
		throw std::invalid_argument("resource dispatcher: implementation is NULL.");
	if(!target)
		throw std::invalid_argument("resource dispatcher: target is NULL.");
	if(!message)
		throw std::invalid_argument("resource dispatcher: message is NULL.");
	if(!args)
		throw std::invalid_argument("resource dispatcher: args is NULL.");

	std::string signature(message->signature);
	std::vector<any> vargs;
	unsigned int c = 0;
	for(char ch : signature) {
		if(ch == '?' || isdigit(ch))
			continue;

		any a;
		switch(ch) {
			// int_32_t
			case 'i':
			case 'h':
			case 'f':
				a = args[c].i;
				break;
			// uint32_t
			case 'u':
				a = args[c].u;
				break;
			// string
			case 's':
				if(args[c].s)
					a = std::string(args[c].s);
				else
					a = std::string("");
				break;
			// resource
			case 'o':
				if(args[c].o)
					a = resource_t(reinterpret_cast<wl_resource*>(args[c].o));
				else
					a = resource_t();
				break;
			// new id
			case 'n': {
				if(args[c].o) {
					wl_resource *resource = reinterpret_cast<wl_resource*>(args[c].o);
					wl_resource_set_user_data(resource, NULL); // Wayland leaves the user data uninitialized
					a = resource_t(resource);
				} else {
					a = resource_t();
					std::cerr << "New id is empty." << std::endl;
				}
			}
			break;
			// array
			case 'a':
				if(args[c].a)
					a = array_t(args[c].a);
				else
					a = array_t();
				break;
			default:
				a = 0;
				break;
		}
		vargs.push_back(a);
		c++;
	}
	resource_t res(reinterpret_cast<wl_resource*>(target), false);
	dispatcher_func dispatcher = reinterpret_cast<dispatcher_func>(const_cast<void*>(implementation));
	return dispatcher(opcode, vargs, res.get_requests());
}

void resource_t::c_destroy(wl_resource *resource) {
}




