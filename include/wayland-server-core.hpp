/*
 * Copyright (c) 2016-2017, Yisu Peng
 * Copyright (c) 2014-2015, Nils Christopher Brause
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

#ifndef WAYLAND_SERVER_CORE_HPP
#define WAYLAND_SERVER_CORE_HPP

/** \file */

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <wayland-server-core.h>
#include <wayland-util.hpp>

namespace wayland {

class resource_t;

/** \brief display class

 */
class display_server_t {
private:
	wl_display *display;

	//int wake_pipe[2];
	int wake_recv;
	int wake_send;

public:
	display_server_t(std::string name = "");

	wl_display *c_ptr();

	void run();

	void terminate();

	void dispatch();

	void wake_epoll();

	static int c_wake_callback(int fd, uint32_t mask, void *data);

	int init_shm();
};

/** \brief reference to the client

 */
class client_t {
private:
	wl_client *client;
	//display_resource_t display;
	//std::mutex mutlock;
public:
	client_t(int fd);
	client_t(wl_client *c);

	//void lock();
	//void unlock();

	//static wl_client from_c_ptr(wl_client c*);
 
	friend bool operator<(const client_t& l, const client_t& r) {
		return l.client < r.client;
	}
	bool operator ==(const client_t &c2) {
		return c2.client == client;
	}

	void flush();
	//void get_credentials(pid_t *pid, uid_t uid, gid_t gid);
	//int get_fd();
	//void add_destroy_listener(listener_t listener);
	//listener_t get_destroy_listener(notify_func_t notify);
	resource_t get_object(uint32_t id);
	//void 
	wl_client *c_ptr();
	//display_resource_t get_display_resource();
	
};

/** \brief reference to the resource

 */
class resource_t : public detail::object_t {
	// Types
protected:
	struct requests_base_t {
		virtual ~requests_base_t() { }
	};
	typedef int(*dispatcher_func)(int, std::vector<detail::any>, std::shared_ptr<resource_t::requests_base_t>);

	//struct user_data_t {
	//	virtual ~user_data_t() { }
	//};

private:
	struct resource_data_t {
		std::shared_ptr<requests_base_t> requests;
		unsigned int counter;
		std::mutex lock;
		//user_data_t *user_data;
		void *user_data;

		resource_data_t();
		resource_data_t(std::shared_ptr<requests_base_t> ev, unsigned int cnt);
	};

	// Member vars
private:
	wl_resource *resource;
	resource_data_t *data; // a reference to wl_resource.user_data
	bool display;
	bool dontdestroy;

protected:
	// Interface desctiption filled in by the each interface class
	const wl_interface *interface;
	// constructor filled in by the each interface class
	std::function<resource_t(resource_t)> copy_constructor;


	// Friend declarations
private:
	friend class detail::argument_t;


	// Constructors and Destructors
public:
	/** \brief Cronstruct a resource_t from a wl_resource pointer
	    \param p Pointer to a wl_resource
	    \param is_display True, if p is a wl_display pointer
	*/
	resource_t(wl_resource *p, bool is_display = false, bool donotdestroy = false);

	/** \brief Copy Constructior
	    \param p A resource_t object

	    For details see operator=()
	*/
	resource_t(const resource_t &p);

	/** \brief Assignment operator
	    \param p A resource_t object

	    After an assignment, both resource_t objects will point to the same resource.
	*/
	resource_t &operator=(const resource_t &p);

	/** \brief Move Constructior
	    \param p A resource_t object
	*/
	resource_t(resource_t &&p);

	/** \brief Move Asignment operator
	    \param p A resource_t object
	*/
	resource_t &operator=(resource_t &&p);

	/** \brief Destructor

	    If this is the last copy of a paticular resource, the resource itself will
	    be detroyed and an destroy request will be marshaled.
	    If the resource belongs to a wl_display object, the connection will be
	    closed.
	*/
	~resource_t();

protected:
	// Constructs NULL proxies.
	resource_t();


	// Methods
public:
	/** \brief Get the id of a resource object.
	    \return The id the object associated with the resource
	 */
	uint32_t get_id();

	/** \brief Get the client
		\return The pointer to the client
	 */
	client_t get_client();

	/** \brief Get the version
		\return version
	 */
	uint32_t get_version();

	/** \brief Get the interface name (class) of a resource object.
	    \return The interface name of the object associated with the resource
	 */
	std::string get_class();

	// /** \brief Assign a resource to an event queue.
	//     \param queue The event queue that will handle this resource

	//     Assign resource to event queue. Events coming from resource will be queued in
	//     queue instead of the display's main queue.

	//     See also: display_server_t::dispatch_queue().
	// */
	// void set_queue(event_queue_t queue);

	// Get a pointer to the underlying C struct.
	wl_resource *c_ptr();

	const wl_interface *get_iface_ptr();

	//void bind();

	void set_user_data(void *data);
	void *get_user_data();

	static resource_t *create(client_t &&client, const interface_t &interface,
			uint32_t version, uint32_t id);

	void post_error(uint32_t code, const char *msg, ...);

protected:

	/*
	  Sets the dispatcher and its user data. User data must be an
	  instance of a class derived from requests_base_t, allocated with
	  new. Will automatically be deleted upon destruction.
	*/
	void set_requests(std::shared_ptr<requests_base_t> requests,
	                int(*dispatcher)(int, std::vector<detail::any>, std::shared_ptr<resource_t::requests_base_t>));

	// Retrieve the perviously set user data
	std::shared_ptr<requests_base_t> get_requests();

	template <typename...T>
	void post_event(int opcode, T...args);

	void marshal_vector(int opcode, std::vector<detail::argument_t> args);

private:
	static int c_dispatcher(const void *implementation, void *target,
	                        uint32_t opcode, const wl_message *message,
	                        wl_argument *args);
	static void c_destroy(wl_resource *resource);
};


typedef void (*resource_bind_func_t)(void *data);//, uint32_t version, uint32_t id);
//typedef void (resource_t::*resource_bind_func_t)();//, uint32_t version, uint32_t id);

class global_t {
public:
	struct global_data_t {
		global_data_t();
	};
// 	// bind func type
// 	typedef 
private:
	wl_global *global;
	interface_t *interface;
	void *user_data;
	//resource_bind_func_t bind;

public:
	global_t(display_server_t &display, const interface_t &iface,
			uint32_t version, global_t *data,//void *data,
			resource_bind_func_t bind_func);

	virtual ~global_t();

	virtual void bind(resource_t res, void *data);

protected:
	static void c_bind(struct wl_client *client, void *data,
			uint32_t version, uint32_t id);

};


class listener_t;
typedef void (*notify_func_t)(listener_t *, void *);

class listener_t {
private:
	wl_listener listener;
	notify_func_t callback;


public:
	listener_t(notify_func_t func);
	listener_t(const listener_t &o);

protected:
	listener_t();


public:
	void notify(void *data);
	wl_listener *c_ptr();

	static listener_t &get_object(void *obj);

protected:
	static void c_notify(wl_listener *p, void *data);
};



class signal_t {
private:
	wl_signal signal;

public:
	signal_t();
	// signal_t(const signal_t &o);

	void add(listener_t &listener);
	//listener_t &get(notify_func_t cb_func);
	void emit(void *data);
};


///** \brief contains a reference to the client and the resource
//
// */
//class service_t {
//private:
//	client_t client;
//	resource_t res;
//};


// Implementations
template <typename...T>
void resource_t::post_event(int opcode, T...args) {
	std::vector<detail::argument_t> v = { detail::argument_t(args)... };
	if (c_ptr()) {
		marshal_vector(opcode, v);
	}
}


}

#endif
