/* compositor.hpp
 *
 * Copyright (c) 2016-2017 Yisu Peng
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

#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <queue>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <thread>

#include <wayland-util.hpp>
#include <wayland-shm.hpp>

#include <wayland-server.hpp>

#include <pixman-1/pixman.h>

#include "wrapper.hpp"

class example_compositor;
class example_view;

class example_surface {
protected:
	struct state {
		//shared_ptr<shm_buffer_t> buffer;
		int newly_attached;
		wayland::shm_buffer_t *buffer;
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

		state() : newly_attached(false), buffer(NULL), sx(0), sy(0) {
			pixman_region32_init(&damage_buffer);
			pixman_region32_init(&damage_surface);
			pixman_region32_init(&opaque);
			pixman_region32_init(&input);
		}
	};

	wayland::surface_resource_t resource;

	example_compositor *compositor;
	example_view *view;
	std::queue<wayland::callback_resource_t> frame_queue;

	/** Damage in local coordinates from the client, for tex upload. */
	pixman_region32_t damage;                                           

	pixman_region32_t opaque;        /* part of geometry, see below */
	pixman_region32_t input;
	int32_t width, height;
	int32_t ref_count;

	state pending;
	state current;

	gl_shader *shader;

public:
	example_surface(example_compositor *c);

	int bind(wayland::surface_resource_t surf);

	wayland::surface_resource_t &get_resource();

	bool bind_view(example_view *v);
	example_view *get_view() {
		return view;
	}

	void draw();

	void frame_done();

	//void notify_motion(int x, int y) {
	//	
	//}

	//void attach() {
	//}

	void commit_state();

	wayland::shm_buffer_t *get_buffer();
	std::vector<int> to_window_space(std::vector<float> v);
	std::vector<float> to_screen_space(std::vector<int> v);
};

class example_pointer;
class example_seat;

class example_pointer {
private:
	example_seat *seat;
	wayland::client_t *client;
	wayland::pointer_resource_t resource;

	//std::unordered_map<

public:
	example_pointer(example_seat *seat)
		: seat(seat)
	{
	}
	void bind(wayland::pointer_resource_t res) {
		resource = res;
	}

	void notify_motion(uint32_t time, int x, int y) {
		wayland::fixed_t fx(x), fy(y);
		resource.send_motion(time, fx, fy);
	}
	void notify_button(uint32_t serial, uint32_t time, uint32_t button,
			wayland::pointer_button_state state) {
		resource.send_button(serial, time, button, state);
	}

};

class example_view {
private:
	example_surface *surface;
	example_pointer *pointer;
	int32_t x, y;
	int32_t width, height;
	pixman_region32_t bounding_box;

public:
	example_view(example_surface *surf)
		: surface(surf),
		x(0), y(0),
		width(0), height(0),
		pointer(NULL)
   	{
		pixman_region32_init(&bounding_box);
	}
	example_view(example_surface *surf, int x, int y,
			int width, int height)
		: surface(surf), x(x), y(y),
		width(width), height(height)
	{
		pixman_region32_init_rect(&bounding_box, x, y, width, height);
	}
	void set_geometry(int x, int y, int width, int height) {
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
		pixman_region32_init_rect(&bounding_box, x, y, width, height);
	}
	int get_left() {
		return x;
	}
	int get_top() {
		return y;
	}
	void move(int dx, int dy) {
		x += dx;
		y += dy;
		pixman_region32_init_rect(&bounding_box, x, y, width, height);
	}
	
	bool contain_point(int x, int y) {
		return pixman_region32_contains_point(&bounding_box, x, y, NULL);
	}
	example_surface *get_surface() {
		return surface;
	}
	void notify_motion(uint32_t time, int x, int y) {
		assert(pointer);
		pointer->notify_motion(time, x - this->x, y - this->y);
	}
	void notify_button(uint32_t serial, uint32_t time, uint32_t button,
			wayland::pointer_button_state state) {
		assert(pointer);
		pointer->notify_button(serial, time, button, state);
	}
	bool bind_pointer(example_pointer *p) {
		if (pointer) {
			cerr << "the view has already got a pointer." << endl;
			return false;
		}
		pointer = p;
		return true;
	}
};

//class example_shell_surface : public shell_surface_resource_t {
class example_shell_surface {
protected:
	//struct shell_surf_user_data_t : public user_data_t {
	//	surface_resource_t surface;
	//};
private:
	wayland::shell_surface_resource_t res;
	example_compositor *compositor;
	wayland::surface_resource_t surf_res;
	bool surface_grabbing;

public:
	//example_shell_surface() {
	//}

	example_shell_surface(example_compositor *c)
		: compositor(c),
		surface_grabbing(false)
	{
	}

	void bind(wayland::shell_surface_resource_t surf);

	void bind_surface(wayland::surface_resource_t surf) {
		//auto data = new shell_surf_user_data_t();
		//data->surface = surf;
		//set_user_data(data);
		surf_res = surf;
	}
};

/**
 * 		GLOBALS
 */

class example_shell : public wayland::global_t {
	wayland::display_server_t display;
	example_compositor *compositor;
public:
	example_shell(wayland::display_server_t disp,
			example_compositor *c)
		: global_t(disp, wayland::detail::shell_interface, 1, this, NULL),
		display(disp),
		compositor(c)
	{
	}

	virtual void bind(wayland::resource_t res, void *data) {
		std::cout << "client bind example_shell" << std::endl;

		auto r = new wayland::shell_resource_t(res);

		r->on_get_shell_surface() = [&] (wayland::shell_surface_resource_t shell_surf, wayland::surface_resource_t surf) {
			auto new_shell_surf = new example_shell_surface(compositor);
			new_shell_surf->bind(shell_surf);
			//example_shell_surface new_shell_surf;
			new_shell_surf->bind_surface(surf);
		};
	}
};

class example_pointer;

class example_seat : public wayland::global_t {
	wayland::display_server_t display;
	example_compositor *compositor;
	std::list<wayland::resource_t> res_list;
	wayland::signal_t selection_signal;
	wayland::signal_t destroy_signal;
	wayland::signal_t upd_caps_signal;

	wayland::seat_capability caps;

	example_pointer *pointer_state;
	//example_keyboard *keyboard_state;
	//example_touch *touch_state;

	example_surface *keyboard_focus;

public:
	example_seat(wayland::display_server_t disp, example_compositor *comp)
		: global_t(disp, wayland::detail::seat_interface, 1, this, NULL),
		display(disp), compositor(comp),
		caps(0)
	{
		caps = wayland::seat_capability::pointer |
			wayland::seat_capability::keyboard |
			wayland::seat_capability::touch;
	}

	virtual void bind(wayland::resource_t res, void *data);
};

class example_compositor : public wayland::global_t {
private:
	wayland::display_server_t display;

	display_wrapper_t wrapper;

	example_shell shell;
	example_seat seat;
	wayland::shm_t shm;

	gl_shader *shader;

	bool running;

	wayland::signal_t destroy_signal;

	/* surface signals */
	wayland::signal_t create_surface_signal;
	wayland::signal_t activate_signal;
	wayland::signal_t transform_signal;

	wayland::signal_t kill_signal;
	wayland::signal_t idle_signal;
	wayland::signal_t wake_signal;

	wayland::signal_t show_input_panel_signal;
	wayland::signal_t hide_input_panel_signal;
	wayland::signal_t update_input_panel_signal;

	wayland::signal_t seat_created_signal;
	wayland::signal_t output_created_signal;
	wayland::signal_t output_destroyed_signal;
	wayland::signal_t output_moved_signal;
	wayland::signal_t output_resized_signal; /* callback argument: resized output */

	wayland::signal_t session_signal;
	int session_active;

	//struct weston_layer fade_layer;
	//struct weston_layer cursor_layer;

	example_surface *focus;
	bool surface_grabbing;

	//previous pointer location
	int32_t prev_pnt_x;
	int32_t prev_pnt_y;

	std::list<example_surface *> surface_list;
	//struct wl_list output_list;
	//struct wl_list seat_list;
	//struct wl_list layer_list;
	//struct wl_list view_list;	/* struct weston_view::link */
	std::list<example_view *> view_list;
	std::map<wayland::client_t, example_view*> view_client_dict;
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
	example_compositor(wayland::display_server_t disp);

	void bind(wayland::resource_t res, void *data);

	static void c_bind(void *data) {
		example_compositor *c = static_cast<example_compositor *>(data);
		//c->bind();
	}

	//static void c_frame(void *owner, void *data) {
	//	auto p = static_cast<example_compositor *>(owner);
	//	p->frame(data);
	//}

	//static void c_quit(void *owner, void *data) {
	//	auto p = static_cast<example_compositor *>(owner);
	//	p->quit();
	//}


	int get_width();

	int get_height() {
		return wrapper.get_height();
	}

	void frame() {
		// compose windows
		// for window list
		for (auto s : surface_list) {
			s->draw();
		}

		for (auto s : surface_list) {
			s->frame_done();
		}

		display.wake_epoll();
	}

	void quit() {
		cout << "quiting..." << endl;
		running = false;
		display.terminate();
	}

	void pointer_enter(int32_t x, int32_t y) {
		cout << "pointer enters (" << x << ", " << y << ")" << endl;
	}

	void pointer_motion(uint32_t time, int32_t x, int32_t y);

	void pointer_button(uint32_t serial, uint32_t time,
			uint32_t button, wayland::pointer_button_state state,
			function<void()> parent_handler);

	void attach(shared_ptr<wayland::shm_buffer_t> buf) {
	}

	gl_shader *get_shader() {
		return shader;
	}

	example_view *find_view(wayland::client_t c) {
		return view_client_dict[c];
	}

	void start_grabbing_surface() {
		surface_grabbing = true;
	}

	void run() {
		running = true;
		//while (running) {
		//	display.dispatch();
		//	wrapper.dispatch();
		//}
		//std::thread wrapper_run_thread([&]() {
		//		wrapper.run();
		//	});
		wrapper.start();

		shader = wrapper.get_shader();

		display.run();
		wrapper.stop();
		wrapper.join();
	}
};




