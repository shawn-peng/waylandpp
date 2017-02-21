/* compositor.cpp
 *
 * Copyright (c) 2016 Yisu Peng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <iostream>
#include <queue>
#include <vector>
#include <list>

#include <wayland-util.h>

#include <wayland-server.hpp>

#include <EGL/eglext.h>
#include <pixman-1/pixman.h>


using namespace wayland;
using namespace wayland::detail;

class example_compositor;

/**
 * 		IMPLEMENTATIONS
 */
class example_buffer {
private:
	buffer_resource_t res;
public:
	example_buffer(buffer_resource_t buf) : res(buf) {
	}
};
//class example_surface : public surface_resource_t {
class example_surface {
protected:
	//struct shell_surf_user_data_t : public user_data_t {
	//}
	struct state {
		shared_ptr<example_buffer> buffer;
		int32_t sx;
		int32_t sy;
		/* wl_surface.damage */
		pixman_region32_t damage_surface;
		/* wl_surface.damage_buffer */
		pixman_region32_t damage_buffer;

		/* wl_surface.set_opaque_region */
		pixman_region32_t opaque;

		/* wl_surface.set_input_region */
		pixman_region32_t input;
	};

	surface_resource_t res;

	example_compositor *compositor;
	std::queue<callback_resource_t> frame_queue;

	/** Damage in local coordinates from the client, for tex upload. */
	pixman_region32_t damage;                                           

	pixman_region32_t opaque;        /* part of geometry, see below */
	pixman_region32_t input;
	int32_t width, height;
	int32_t ref_count;

	//shared_ptr<example_buffer> pending_buffer;
	//int32_t pending_sx;
	//int32_t pending_sy;
	state pending;

public:
	example_surface(example_compositor *c) : compositor(c) {
	}

	int bind(surface_resource_t surf) {
   		//surface_resource_t(surf) {
		res = surf;

		// lambda functions with members captured
		surf.on_destroy() = [&]() {
		};

		surf.on_attach() = [&](buffer_resource_t buffer, int x, int y) {
			pending.buffer = make_shared<example_buffer>(buffer);
			pending.sx = x;
			pending.sy = y;
		};

		surf.on_frame() = [&](callback_resource_t c) {
			cout << "frame" << endl;
			frame_queue.push(c);
		};

		surf.on_damage() = [&](int x, int y, int width, int height) {
			cout << "damage: (" << x << ", " << y
				<< ", " << width << ", " << height << ")" << endl;
			pixman_region32_union_rect(&pending.damage_surface,
					&pending.damage_surface,
					x, y, width, height);
		};

		surf.on_commit() = [&]() {
		};
	}

	//void attach() {
	//}

	void commit_state();
};

//class example_shell_surface : public shell_surface_resource_t {
class example_shell_surface {
protected:
	//struct shell_surf_user_data_t : public user_data_t {
	//	surface_resource_t surface;
	//};
private:
	surface_resource_t res;

public:
	//example_shell_surface() {
	//}

	example_shell_surface(shell_surface_resource_t surf) {
		bind(surf);
	}

	void bind(shell_surface_resource_t surf) {
		surf.on_pong() = [&](uint32_t serial) {
			printf("get pong (%d).\n", serial);
		};

		surf.on_set_title() = [&](std::string title) {
			cout << "set title: " << title << endl;
		};

		surf.on_set_toplevel() = [&](void) {
		};
	}

	void bind_surface(surface_resource_t surf) {
		//auto data = new shell_surf_user_data_t();
		//data->surface = surf;
		//set_user_data(data);
		res = surf;
	}
};

/**
 * 		GLOBALS
 */
class example_compositor : public global_t {
private:
	display_t display;

	signal_t destroy_signal;

	/* surface signals */
	signal_t create_surface_signal;
	signal_t activate_signal;
	signal_t transform_signal;

	signal_t kill_signal;
	signal_t idle_signal;
	signal_t wake_signal;

	signal_t show_input_panel_signal;
	signal_t hide_input_panel_signal;
	signal_t update_input_panel_signal;

	signal_t seat_created_signal;
	signal_t output_created_signal;
	signal_t output_destroyed_signal;
	signal_t output_moved_signal;
	signal_t output_resized_signal; /* callback argument: resized output */

	signal_t session_signal;
	int session_active;

	//struct weston_layer fade_layer;
	//struct weston_layer cursor_layer;

	//struct wl_list output_list;
	//struct wl_list seat_list;
	//struct wl_list layer_list;
	//struct wl_list view_list;	/* struct weston_view::link */
	//struct wl_list plane_list;
	//struct wl_list key_binding_list;
	//struct wl_list modifier_binding_list;
	//struct wl_list button_binding_list;
	//struct wl_list touch_binding_list;
	//struct wl_list axis_binding_list;
	//struct wl_list debug_binding_list;

	//uint32_t state;
	//struct wl_event_source *idle_source;
	//uint32_t idle_inhibit;
	//int idle_time;			/* timeout, s */

	//const struct weston_pointer_grab_interface *default_pointer_grab;

	///* Repaint state. */
	//struct weston_plane primary_plane;
	//uint32_t capabilities; /* combination of enum weston_capability */

	//struct weston_renderer *renderer;

	//pixman_format_code_t read_format;

	//struct weston_backend *backend;

	//struct weston_launcher *launcher;

	//struct wl_list plugin_api_list; /* struct weston_plugin_api::link */

	//uint32_t output_id_pool;

	//struct xkb_rule_names xkb_names;
	//struct xkb_context *xkb_context;
	//struct weston_xkb_info *xkb_info;

	///* Raw keyboard processing (no libxkbcommon initialization or handling) */
	//int use_xkbcommon;

	//int32_t kb_repeat_rate;
	//int32_t kb_repeat_delay;

	//bool vt_switching;

	//clockid_t presentation_clock;
	//int32_t repaint_msec;

	//unsigned int activate_serial;

	//struct wl_global *pointer_constraints;

	//int exit_code;

	//void *user_data;
	//void (*exit)(struct weston_compositor *c);

public:
	example_compositor(display_t disp)
		: global_t(disp, compositor_interface, 4, this, NULL),
		display(disp), session_active(true) {
		//new global_t(display, compositor_interface, 4, this, &c_bind);
		//new global_t(display, shell_interface, 1, this, &c_bind);
		//new global_t(display, seat_interface, 1, this, &c_bind);
		//new global_t(display, shm_interface, 1, this, &c_bind);
	}

	virtual void bind(resource_t res, void *data) {
		std::cout << "client bind example_compositor" << std::endl;

		compositor_resource_t compositor(res);
		compositor.on_create_surface() = [&](surface_resource_t surface) {
			//surface_resource_t surface(*resource_t::create(res.get_client(), surface_interface, res.get_version(), id));
			//new example_surface(surface);
			auto s = new example_surface(this);
			s->bind(surface);

			//surface.on_frame() = [&](callback_resource_t c) {
			//	cout << "frame" << endl;
			//};

			//surface.on_damage() = [&](int x, int y, int width, int height) {
			//	cout << "damage: (" << x << ", " << y
			//		<< ", " << width << ", " << height << ")" << endl;
			//};
		};
	}

	static void c_bind(void *data) {
		example_compositor *c = static_cast<example_compositor *>(data);
		//c->bind();
	}


	void attach(shared_ptr<example_buffer> buf) {
	}
};

class example_shell : public global_t {
	display_t display;
public:
	example_shell(display_t disp)
		: global_t(disp, shell_interface, 1, this, NULL),
		display(disp) {
	}

	virtual void bind(resource_t res, void *data) {
		std::cout << "client bind example_shell" << std::endl;

		shell_resource_t r(res);

		r.on_get_shell_surface() = [&] (shell_surface_resource_t shell_surf, surface_resource_t surf) {
			example_shell_surface new_shell_surf(shell_surf);
			//example_shell_surface new_shell_surf;
			new_shell_surf.bind_surface(surf);
		};
	}
};

class example_seat : public global_t {
	display_t display;
	std::list<resource_t> res_list;
	signal_t selection_signal;
	signal_t destroy_signal;
	signal_t upd_caps_signal;

	seat_capability caps;

public:
	example_seat(display_t disp)
		: global_t(disp, seat_interface, 1, this, NULL),
		display(disp),
		caps(0)
	{
		caps = seat_capability::pointer |
			seat_capability::keyboard |
			seat_capability::touch;
	}

	virtual void bind(resource_t res, void *data) {
		std::cout << "client bind example_seat" << std::endl;
		res_list.push_back(res);
		
		seat_resource_t r(res);

		r.on_get_pointer() = [&](pointer_resource_t res) {
		};

		r.on_get_keyboard() = [&](keyboard_resource_t res) {
		};

		r.on_get_touch() = [&](touch_resource_t res) {
		};

		r.send_capabilities(caps);
	}
};

class example_shm : public global_t {
	display_t display;
public:
	example_shm(display_t disp)
		: global_t(disp, shm_interface, 1, this, NULL),
		display(disp) {
	}

	virtual void bind(resource_t res, void *data) {
		std::cout << "client bind example_shm" << std::endl;
		//shm_resource_t r(res);

		//r.on_create_pool() = [&] (shm_pool_resource_t pool, int fd, int32_t size) {
		//	//create_pool(fd, size)
		//	//pool -> refer pool
		//};
	}
};

void example_surface::commit_state() {
	//compositor->attach(pending.buffer);
	//pending.buffer = NULL;

}


// No weston version
int main(int argc, char *argv[]) {
	//weston_t weston;
	//weston.init(argc, argv);

	//display_t display("waylandpp-example");
	display_t display;

	example_compositor compositor(display);
	//compositor.set_weston(weston.ec);

	example_shell shell(display);
	example_seat seat(display);
	//example_shm shm(display);
	display.init_shm();

	//gl_renderer glr(EGL_PLATFORM_WAYLAND_KHR, display.c_ptr(),
	//		NULL, //gl_renderer->alpha_attribs, NULL, 0);

	display.run();

	return 0;
}


