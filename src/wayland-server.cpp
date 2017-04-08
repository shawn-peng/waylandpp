/*
 * Copyright (c) 2016-2017, Yisu Peng
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

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <wayland-server.h>
#include <wayland-server.hpp>
//#include <wayland-server-protocol.hpp>

using namespace wayland;
using namespace wayland::detail;

display_server_t::display_server_t(std::string name) {
	//: display_resource_t(
	//		resource_t(
	//			reinterpret_cast<wl_resource*>(wl_display_create()),
	//			true)) {
	display = wl_display_create();
	if (name == "") {
		wl_display_add_socket(display, NULL);
	} else {
		wl_display_add_socket(display, name.c_str());
	}

	int pfds[2];
	int ret = pipe(pfds);
	if (ret < 0) {
		throw std::invalid_argument(strerror(errno));
	}
	wake_recv = pfds[0];
	wake_send = pfds[1];
	wl_event_loop *loop = wl_display_get_event_loop(display);
	auto src = wl_event_loop_add_fd(loop, wake_recv,
			WL_EVENT_READABLE,
			c_wake_callback, (void *)this);
}

wl_display *display_server_t::c_ptr() {
	return display;
}

void display_server_t::run() {
	wl_display_run(display);
}

void display_server_t::terminate() {
	wl_display_terminate(display);
}

void display_server_t::dispatch() {
	wl_display_flush_clients(display);
	wl_event_loop *loop = wl_display_get_event_loop(display);
	wl_event_loop_dispatch(loop, -1);
}

int display_server_t::c_wake_callback(int fd, uint32_t mask, void *data) {
	char buf[16];
	int ret = read(fd, buf, 16);
}

void display_server_t::wake_epoll() {
	char data = 0;
	int ret = write(wake_send, &data, sizeof(data));
}

//int display_server_t::init_shm() {
//	return wl_display_init_shm(display);
//}

client_t::client_t(wl_client *c) : client(c) {
	//wl_resource *disp = c->display_resource;
	//wl_display disp = wl_client_get_display(c);
	//display = display_resource_t(resource_t(disp));
}
//client_t client_t::from_c_ptr(wl_client *c) {
//	//client
//}


//void client_t::lock() {
//	mutlock.lock();
//}
//
//void client_t::unlock() {
//	mutlock.unlock();
//}

wl_client *client_t::c_ptr() {
	return client;
}

//display_resource_t client_t::get_display_resource() {
//	return display;
//}

// void global_bind(wl_client *client, void *data, uint32_t version, uint32_t id) {
// }


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
		// check implementation
		data = reinterpret_cast<resource_data_t*>(wl_resource_get_user_data(c_ptr()));
		if(!data) {
			data = new resource_data_t{std::shared_ptr<requests_base_t>(), 0};
			//cout << "malloc data struct for res(" << get_id() << "), counter = " << data->counter << endl;
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
		cout << "malloc data struct for res(" << get_id() << "), counter = " << data->counter << endl;
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
				cout << "destroy resource(" << get_id() << ")" << endl;
				wl_resource_destroy(resource);
			}
			delete data;
		}
	}
}

uint32_t resource_t::get_id() {
	return wl_resource_get_id(c_ptr());
}

client_t resource_t::get_client() {
	return client_t(wl_resource_get_client(resource));
	//wl_client *c = wl_resource_get_client(resource);
	//return client_t::from_c_ptr(c);
}

uint32_t resource_t::get_version() {
	return wl_resource_get_version(c_ptr());
}

std::string resource_t::get_class() {
	return wl_resource_get_class(c_ptr());
}

//void resource_t::set_queue(event_queue_t queue) {
//	wl_resource_set_queue(c_ptr(), queue.c_ptr());
//}

void resource_t::set_user_data(void *user_data) {
	if (data == NULL) {
		assert(0);
	} else {
		data->user_data = user_data;
	}
}

void *resource_t::get_user_data() {
	if (data == NULL) {
		return NULL;
	} else {
		return data->user_data;
	}
}

wl_resource *resource_t::c_ptr() {
	if(!resource)
		throw std::invalid_argument("resource is NULL");
	return resource;
}

void resource_t::post_error(uint32_t code, const char *msg, ...) {
	client_t client = get_client();
	char buffer[128];
	va_list ap;

	va_start(ap, msg);
	vsnprintf(buffer, sizeof buffer, msg, ap);
	va_end(ap);

	//disp.post_event(code, WL_DISPLAY_ERROR, *this, code, buffer);
	wl_resource_post_error(c_ptr(), code, buffer);
}

resource_t::resource_data_t::resource_data_t()
	: requests(NULL), user_data(NULL) {
}

resource_t::resource_data_t::resource_data_t(std::shared_ptr<requests_base_t> ev,
		unsigned int cnt)
	: requests(ev), counter(cnt), user_data(NULL) {
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
	resource_t res(reinterpret_cast<wl_resource*>(target), false);
	client_t client = res.get_client();
	unsigned int c = 0;
	for(char ch : signature) {
	//for (int i = 0; i < signature.size(); i++) {
	//	auto ch = signature[i];
		if(ch == '?' || isdigit(ch))
			continue;

		any a;
		switch(ch) {
			// int_32_t
			case 'i':
			case 'h':
				a = args[c].i;
				break;
			// uint32_t
			case 'u':
				a = args[c].u;
				break;
			// fixed
			case 'f':
			{
				fixed_t f;
				f.set_data(args[c].f);
				a = f;
				break;
			}
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
				//wl_resource *resource = reinterpret_cast<wl_resource*>(args[c].o);
				int id = args[c].u;
				if(id == 0) {
					a = resource_t();
					std::cerr << "New id is empty." << std::endl;
					return 1;
				}
				//wl_resource *resource = wl_client_get_object(client.c_ptr(), id);
				const interface_t *iface = message->types[c];
				wl_resource *resource = wl_resource_create(client.c_ptr(), iface, res.get_version(), id);
				wl_resource_set_user_data(resource, NULL); // Wayland leaves the user data uninitialized
				a = resource_t(resource);
				break;
			}
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
	dispatcher_func dispatcher = reinterpret_cast<dispatcher_func>(const_cast<void*>(implementation));
	//std::shared_ptr<requests_base_t> reqs = res.get_requests();
	return dispatcher(opcode, vargs, res.get_requests());
}

void resource_t::c_destroy(wl_resource *resource) {
}

void resource_t::marshal_vector(int opcode, std::vector<argument_t> args) {
	std::vector<wl_argument> v;
	for(auto &arg : args) {
		v.push_back(arg.argument);
	}

	//data->lock.lock();
	wl_resource_post_event_array(resource, opcode, v.data());
	//data->lock.unlock();
	return;
}

std::shared_ptr<resource_t::requests_base_t> resource_t::get_requests() {
	if(!display)
		return data->requests;
	return std::shared_ptr<requests_base_t>();
}

resource_t *resource_t::create(client_t &&client, const interface_t &interface,
		uint32_t version, uint32_t id) {
	//return client.creat_resource(const_cast<wl_interface *>(&interface), version, id));
	//client.lock();
	auto p =  new resource_t(wl_resource_create(client.c_ptr(), const_cast<wl_interface *>(&interface), version, id));
	//client.unlock();
	return p;
}


global_t::global_t(display_server_t &display,
		const interface_t &iface,
		uint32_t version,
		//void *data,
		global_t *data,
		resource_bind_func_t bind_func) :
	interface(const_cast<interface_t *>(&iface)),
	user_data(data)/*, bind(bind_func)*/ {
	global = wl_global_create(display.c_ptr(), interface, version, reinterpret_cast<void*>(data), c_bind);
	//global = wl_global_create(display.c_ptr(), interface, version, data, c_bind);
}

global_t::~global_t() {
}

void global_t::bind(resource_t res, void *data) {
	cout << "Calling base bind method. Do nothing." << endl;
}

void global_t::c_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	global_t *g = reinterpret_cast<global_t *>(data);
	//g = g->user_data;

	wl_resource *r = wl_resource_create(client, g->interface, version, id);
	if (r == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	
	//(*g->bind)(g->user_data);
	//resource_t res(r);
	g->bind(resource_t(r), g->user_data);
}



//listener_t::listener_t() : callback(NULL) {
//}

listener_t::listener_t(notify_func_t func) : callback(func) {
	listener.notify = c_notify;
}

void listener_t::notify(void *data) {
	(*callback)(this, data);
}

wl_listener *listener_t::c_ptr() {
	return &listener;
}

listener_t &listener_t::get_object(void *obj) {
	return *static_cast<listener_t *>(obj);
}

void listener_t::c_notify(wl_listener *p, void *data) {
	get_object(p).notify(data);
}


signal_t::signal_t() {
	wl_signal_init(&signal);
}

void signal_t::add(listener_t &listener) {
	wl_signal_add(&signal, listener.c_ptr());
}

//listener_t &signal_t::get(notify_func_t cb_func) {
//	return listener_t::get_object(wl_signal_get(&signal, cb_func));
//}

void signal_t::emit(void *data) {
	wl_signal_emit(&signal, data);
}



