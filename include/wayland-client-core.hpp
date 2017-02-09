/*
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

#ifndef WAYLAND_CLIENT_CORE_HPP
#define WAYLAND_CLIENT_CORE_HPP

/** \file */

#include <memory>
#include <string>
#include <vector>
#include <wayland-client-core.h>
#include <wayland-util.hpp>

//struct wl_event_queue;

namespace wayland {
/** \brief A queue for proxy_t object events.

    Event queues allows the events on a display to be handled in a
    thread-safe manner. See display_t for details.
*/
class event_queue_t {
  private:
	struct queue_ptr {
		wl_event_queue *queue;
		~queue_ptr();
	};

	std::shared_ptr<queue_ptr> queue;
	event_queue_t(wl_event_queue *q);

	friend class proxy_t;
	friend class display_t;

	// Get a pointer to the underlying C struct.
	wl_event_queue *c_ptr();
};

class display_t;

/** \brief Represents a protocol object on the client side.

    A proxy_t acts as a client side proxy to an object existing in the
    compositor. The proxy is responsible for converting requests made
    by the clients with proxy_t::marshal() or
    proxy_t::marshal_constructor() into Wayland's wire format. Events
    coming from the compositor are also handled by the proxy, which
    will in turn call the handler set with the on_XXX() functions of
    each interface class.

    With the exception of the function proxy_t::set_queue(), functions
    accessing a proxy_t are not normally used by client code. Clients
    should normally use the higher level interface classed generated
    by the scanner to interact with compositor objects.
*/
class proxy_t : public detail::object_t {

  protected:
	// base class for event listener storage.
	struct events_base_t {
		virtual ~events_base_t() { }
	};

	// Types
  private:
	// stored in the proxy user data
	struct proxy_data_t {
		std::shared_ptr<events_base_t> events;
		int destroy_opcode;
		unsigned int counter;

		proxy_data_t();
		proxy_data_t(std::shared_ptr<events_base_t> ev, int desop, unsigned int cnt);
	};


	// Member vars
  private:
	wl_proxy *proxy;
	proxy_data_t *data;
	bool display;
	bool dontdestroy;

  protected:
	// Interface desctiption filled in by the each interface class
	const wl_interface *interface;
	// constructor filled in by the each interface class
	std::function<proxy_t(proxy_t)> copy_constructor;


	// Friend declarations
  private:
	friend class detail::argument_t;

  protected:
	friend class registry_proxy_t;
	friend class egl_window_t;
	friend class cursor_theme_t;
	//friend EGLDisplay(::eglGetDisplay)(wayland::display_t &display);


	// Constructors and Destructors
  public:
	/** \brief Cronstruct a proxy_t from a wl_proxy pointer
	    \param p Pointer to a wl_proxy
	    \param is_display True, if p is a wl_display pointer
	*/
	proxy_t(wl_proxy *p, bool is_display = false, bool donotdestroy = false);

	/** \brief Copy Constructior
	    \param p A proxy_t object

	    For details see operator=()
	*/
	proxy_t(const proxy_t &p);

	/** \brief Assignment operator
	    \param p A proxy_t object

	    After an assignment, both proxy_t objects will point to the same proxy.
	*/
	proxy_t &operator=(const proxy_t &p);

	/** \brief Move Constructior
	    \param p A proxy_t object
	*/
	proxy_t(proxy_t &&p);

	/** \brief Move Asignment operator
	    \param p A proxy_t object
	*/
	proxy_t &operator=(proxy_t &&p);

	/** \brief Destructor

	    If this is the last copy of a paticular proxy, the proxy itself will
	    be detroyed and an destroy request will be marshaled.
	    If the proxy belongs to a wl_display object, the connection will be
	    closed.
	*/
	~proxy_t();

  protected:
	// Constructs NULL proxies.
	proxy_t();


	// Methods
  public:
	/** \brief Get the id of a proxy object.
	    \return The id the object associated with the proxy
	*/
	uint32_t get_id();

	/** \brief Get the interface name (class) of a proxy object.
	    \return The interface name of the object associated with the proxy
	*/
	std::string get_class();

	/** \brief Assign a proxy to an event queue.
	    \param queue The event queue that will handle this proxy

	    Assign proxy to event queue. Events coming from proxy will be queued in
	    queue instead of the display's main queue.

	    See also: display_t::dispatch_queue().
	*/
	void set_queue(event_queue_t queue);

	// Get a pointer to the underlying C struct.
	wl_proxy *c_ptr();

	const wl_interface *get_iface_ptr();

  private:
	// universal dispatcher
	static int c_dispatcher(const void *implementation, void *target,
	                        uint32_t opcode, const wl_message *message,
	                        wl_argument *args);

	// marshal request
	proxy_t marshal_single(uint32_t opcode, const wl_interface *interface,
	                       std::vector<detail::argument_t> v);
	proxy_t marshal_single(uint32_t opcode, const wl_interface *interface,
	                       std::list<detail::argument_t> v);

  protected:
	// marshal a request, that doesn't lead a new proxy
	// Valid types for args are:
	// - uint32_t
	// - int32_t
	// - proxy_t
	// - std::string
	// - std::vector<char>
	template <typename...T>
	void marshal(uint32_t opcode, T...args) {
		std::vector<detail::argument_t> v = { detail::argument_t(args)... };
		if (c_ptr())
			marshal_single(opcode, NULL, v);
	}

	// // dynamically marshal a request, that leads a new proxy, incomplete
	// template <typename...T>
	// proxy_t marshal_constructor_dynamic(uint32_t opcode, proxy_t &interface,
	//                             T...args) {
	// 	std::list<detail::argument_t> v = { detail::argument_t(args)... };
	// 	// find new_id in v, change to 2 args (name, version)
	// 	if (c_ptr())
	// 		return marshal_single(opcode, interface.interface, v);
	// 	return proxy_t();
	// }

	// marshal a request, that leads a new proxy
	template <typename...T>
	proxy_t marshal_constructor(uint32_t opcode, const wl_interface *interface,
	                            T...args) {
		std::vector<detail::argument_t> v = { detail::argument_t(args)... };
		if (c_ptr())
			return marshal_single(opcode, interface, v);
		return proxy_t();
	}

	// Set the opcode for destruction of the proxy (-1 unsets it)
	void set_destroy_opcode(int opcode);

	/*
	  Sets the dispatcher and its user data. User data must be an
	  instance of a class derived from events_base_t, allocated with
	  new. Will automatically be deleted upon destruction.
	*/
	void set_events(std::shared_ptr<events_base_t> events,
	                int(*dispatcher)(int, std::vector<detail::any>, std::shared_ptr<proxy_t::events_base_t>));

	// Retrieve the perviously set user data
	std::shared_ptr<events_base_t> get_events();
};

class callback_t;
class registry_t;

}

#endif
