/* compositor.cpp
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

#include <iostream>
#include <queue>
#include <vector>
#include <list>
#include <thread>

#include <wayland-util.hpp>
#include <wayland-shm.hpp>

#include <wayland-server.hpp>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
//#include <GL/gl.h>
//#include <GL/glext.h>
//#include <GL/glew.h>

#include <EGL/eglext.h>
#include <pixman-1/pixman.h>

#include "helper.hpp"
#include "wrapper.hpp"

using namespace wayland;
using namespace wayland::detail;

class example_compositor;

void *read_tga(const char *filename, int *width, int *height);

/**
 * 		IMPLEMENTATIONS
 */

static GLuint make_buffer(
		GLenum target,
		const void *buffer_data,
		GLsizei buffer_size)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(target, buffer);
	glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
	return buffer;
}

class example_surface {
protected:
	struct state {
		//shared_ptr<shm_buffer_t> buffer;
		shm_buffer_t *buffer;
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

		state() : buffer(NULL), sx(0), sy(0) {
			pixman_region32_init(&damage_buffer);
			pixman_region32_init(&damage_surface);
			pixman_region32_init(&opaque);
			pixman_region32_init(&input);
		}
	};

	surface_resource_t resource;

	example_compositor *compositor;
	std::queue<callback_resource_t> frame_queue;

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

	int bind(surface_resource_t surf) {
   		//surface_resource_t(surf) {
		resource = surf;

		// lambda functions with members captured
		surf.on_destroy() = [&]() {
		};

		surf.on_attach() = [&](buffer_resource_t buf_res, int x, int y) {
			cout << "attach buffer(" << buf_res.get_id() << ") to: x(" << x << "), y(" << y << ")" << endl;
			if (pending.buffer) {
				pending.buffer->release();
			}
			shm_buffer_t *buffer = shm_buffer_t::from_resource(buf_res);
			//pending.buffer.reset(buffer);
			pending.buffer = buffer;
			pending.sx = x;
			pending.sy = y;
			width = buffer->get_width();
			height = buffer->get_height();
		};

		surf.on_frame() = [&](callback_resource_t c) {
			cout << "frame" << endl;
			frame_queue.push(c);
		};

		surf.on_damage() = [&](int x, int y, int width, int height) {
			//cout << "damage: (" << x << ", " << y
			//	<< ", " << width << ", " << height << ")" << endl;
			pixman_region32_union_rect(&pending.damage_surface,
					&pending.damage_surface,
					x, y, width, height);
		};

		surf.on_commit() = [&]() {
			//cout << "commit" << endl;
			//swap(pending, current);
		};
	}

	shm_buffer_t *get_buffer() {
		//return current.buffer.get();
		return pending.buffer;
	}

	vector<int> to_window_space(vector<float> v)
	{
	}

	vector<float> to_screen_space(vector<int> v)
	{
	}

	void draw();

	void frame_done() {
		if (frame_queue.empty()) {
			return;
		}
		timeval tv;
		gettimeofday(&tv, 0);
		uint32_t x = tv.tv_sec * 1000 + tv.tv_usec / 1000;

		frame_queue.front().send_done(x);
		frame_queue.pop();
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
	shell_surface_resource_t res;
	surface_resource_t surf_res;

public:
	//example_shell_surface() {
	//}

	example_shell_surface(shell_surface_resource_t surf) {
		bind(surf);
	}

	void bind(shell_surface_resource_t surf) {
		res = surf;
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
		surf_res = surf;
	}
};

/**
 * 		GLOBALS
 */
class example_compositor : public global_t {
private:
	display_server_t display;

	display_wrapper_t wrapper;

	gl_shader *shader;

	bool running;

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

	std::list<example_surface *> surface_list;
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
	example_compositor(display_server_t disp)
		: global_t(disp, compositor_interface, 4, this, NULL),
		display(disp), session_active(true) {
		//new global_t(display, compositor_interface, 4, this, &c_bind);
		//new global_t(display, shell_interface, 1, this, &c_bind);
		//new global_t(display, seat_interface, 1, this, &c_bind);
		//new global_t(display, shm_interface, 1, this, &c_bind);

		wrapper.set_owner((void *)this);
		//wrapper.on_frame() = c_frame;
		//wrapper.register_callback("frame", c_frame);
		//wrapper.register_callback("quit", c_quit);
		//wrapper.register_callback("frame",
		//		bind_mem_fn(&example_compositor::frame, this));
		//wrapper.register_callback("quit",
		//		bind_mem_fn(&example_compositor::quit, this));
		wrapper.on_frame() =
				bind_mem_fn(&example_compositor::frame, this);
		wrapper.on_quit() =
				bind_mem_fn(&example_compositor::quit, this);
		wrapper.on_pointer_enter() =
				bind_mem_fn(&example_compositor::pointer_enter, this);
		wrapper.on_pointer_motion() =
				bind_mem_fn(&example_compositor::pointer_motion, this);
	}

	virtual void bind(resource_t res, void *data) {
		std::cout << "client bind example_compositor" << std::endl;

		auto compositor = new compositor_resource_t(res);
		compositor->on_create_surface() = [&](surface_resource_t surf_res) {
			//surface_resource_t surf_res(*resource_t::create(res.get_client(), surface_interface, res.get_version(), id));
			//new example_surface(surf_res);
			auto s = new example_surface(this);
			s->bind(surf_res);
			surface_list.push_back(s);
		};
	}

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

		display.wake();
	}

	void quit() {
		cout << "quiting..." << endl;
		running = false;
		display.terminate();
	}

	void pointer_enter(int32_t x, int32_t y) {
		cout << "pointer enters (" << x << ", " << y << ")" << endl;
	}

	void pointer_motion(uint32_t time, int32_t x, int32_t y) {
		//cout << "pointer motion (" << x << ", " << y << ")@"
		//	<< time << endl;
		for (auto s : surface_list) {
			//if (s->bounding(x, y)) {
			//}
		}
	}

	void attach(shared_ptr<shm_buffer_t> buf) {
	}

	gl_shader *get_shader() {
		return shader;
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

example_surface::example_surface(example_compositor *c) : compositor(c) {
	shader = c->get_shader();
}
void example_surface::draw() {
	//GLfloat verts[4 * 2];
	//verts[0] = 0;
	//verts[1] = 0;
	//verts[2] = 0;
	//verts[3] = height;
	//verts[4] = width;
	//verts[5] = height;
	//verts[6] = width;
	//verts[7] = 0;
	//static const GLfloat verts[4 * 2] = { 
	//	0.0f, 0.0f,
	//	1.0f, 0.0f,
	//	1.0f, 1.0f,
	//	0.0f, 1.0f
	//};

	glUseProgram(shader->program);

	//struct {
	//	GLuint vertex_buffer, element_buffer;
	//	GLuint textures[2];

	//	/* fields for shader objects ... */
	//} g_resources;
	static const GLfloat verts[] = { 
		-1.0f,  1.0f,
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f,  1.0f,
	};
	//static const GLushort elements[] = { 0, 1, 2, 3 };


	if (!pending.buffer) {
		return;
	}

	if (shader == NULL) {
		cerr << "No valid shader." << endl;
		return;
	}

	shm_buffer_t &buf = *pending.buffer;

	//cout << "drawing surface(" << resource.get_id() << ")"
	//	<< "with attached buffer(" << buf.get_resource().get_id() << ")"
	//	<< endl;

	//cout << pending.sx << endl
	//	<< pending.sy << endl
	//	<< width << endl
	//	<< height << endl;
	int port_x = pending.sx;
	int port_y = (compositor->get_height()-height-pending.sy);
	glViewport(port_x, port_y, width, height);
	//glMatrixMode(GL_PROJECTION);

	GLint uniform_tex
		= glGetUniformLocation(shader->program, "tex");

	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0,
			GL_RGBA, 
			buf.get_width(),
			buf.get_height(),
			0,
			GL_RGBA, GL_UNSIGNED_BYTE,
			buf.get_data());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(uniform_tex, 0);


	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, &verts);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableVertexAttribArray(0);

	gl_print_error();

	return;

	// position:
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray(0);

	// texcoord:
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray(1);

	glUseProgram(shader->program);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	gl_print_error();

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

}

class example_shell : public global_t {
	display_server_t display;
public:
	example_shell(display_server_t disp)
		: global_t(disp, shell_interface, 1, this, NULL),
		display(disp) {
	}

	virtual void bind(resource_t res, void *data) {
		std::cout << "client bind example_shell" << std::endl;

		auto r = new shell_resource_t(res);

		r->on_get_shell_surface() = [&] (shell_surface_resource_t shell_surf, surface_resource_t surf) {
			auto new_shell_surf = new example_shell_surface(shell_surf);
			//example_shell_surface new_shell_surf;
			new_shell_surf->bind_surface(surf);
		};
	}
};

class example_seat : public global_t {
	display_server_t display;
	std::list<resource_t> res_list;
	signal_t selection_signal;
	signal_t destroy_signal;
	signal_t upd_caps_signal;

	seat_capability caps;

public:
	example_seat(display_server_t disp)
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
			auto p = new pointer_resource_t(res);
		};

		r.on_get_keyboard() = [&](keyboard_resource_t res) {
			auto p = new keyboard_resource_t(res);
		};

		r.on_get_touch() = [&](touch_resource_t res) {
			auto p = new touch_resource_t(res);
		};

		r.send_capabilities(caps);
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

	//display_server_t display("waylandpp-example");
	display_server_t display;

	example_compositor compositor(display);
	//compositor.set_weston(weston.ec);

	example_shell shell(display);
	example_seat seat(display);
	shm_t shm(display);
	//display.init_shm();

	//gl_renderer glr(EGL_PLATFORM_WAYLAND_KHR, display.c_ptr(),
	//		NULL, //gl_renderer->alpha_attribs, NULL, 0);

	compositor.run();

	return 0;
}


