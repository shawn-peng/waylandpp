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

#include "compositor.hpp"


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


int example_surface::bind(surface_resource_t surf) {
	//surface_resource_t(surf) {
	resource = surf;

	// lambda functions with members captured
	surf.on_destroy() = [&]() {
	};

	surf.on_attach() = [&](wayland::buffer_resource_t buf_res, int x, int y) {
		cout << "attach buffer(" << buf_res.get_id() << ") to: x(" << x << "), y(" << y << ")" << endl;
		if (pending.buffer) {
			pending.buffer->release();
		}
		shm_buffer_t *buffer = shm_buffer_t::from_resource(buf_res);
		//pending.buffer.reset(buffer);
		pending.newly_attached = true;
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

surface_resource_t &example_surface::get_resource() {
	return resource;
}

shm_buffer_t *example_surface::get_buffer() {
	//return current.buffer.get();
	return pending.buffer;
}

std::vector<int> example_surface::to_window_space(std::vector<float> v)
{
}

std::vector<float> example_surface::to_screen_space(std::vector<int> v)
{
}

void example_surface::frame_done() {
	if (frame_queue.empty()) {
		return;
	}
	timeval tv;
	gettimeofday(&tv, 0);
	uint32_t x = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	frame_queue.front().send_done(x);
	frame_queue.pop();
}



example_surface::example_surface(example_compositor *c)
	: compositor(c), view(NULL)
{
	shader = c->get_shader();
}
void example_surface::draw() {

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

	int new_x = view->get_left() + pending.sx;
	int new_y = view->get_top() + pending.sy;

	shm_buffer_t &buf = *pending.buffer;

	int new_width = buf.get_width();
	int new_height = buf.get_height();

	view->set_geometry(new_x, new_y, new_width, new_height);

	if (pending.newly_attached) {
		if (buf.get_format() == shm_format::argb8888) {
			buf.swap_BR_channels();
		}
		pending.newly_attached = false;
	}

	//cout << "drawing surface(" << resource.get_id() << ")"
	//	<< "with attached buffer(" << buf.get_resource().get_id() << ")"
	//	<< endl;

	int port_x = new_x;
	int port_y = (compositor->get_height()-height-new_y);
	glViewport(port_x, port_y, new_width, new_height);
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

	//// position:
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
	//glEnableVertexAttribArray(0);

	//// texcoord:
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, verts);
	//glEnableVertexAttribArray(1);

	//glUseProgram(shader->program);
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//gl_print_error();

	//glDisableVertexAttribArray(1);
	//glDisableVertexAttribArray(0);

}

void example_surface::commit_state() {
	//compositor->attach(pending.buffer);
	//pending.buffer = NULL;

}

void example_shell_surface::bind(shell_surface_resource_t surf) {
	res = surf;
	surf.on_pong() = [&](uint32_t serial) {
		printf("get pong (%d).\n", serial);
	};

	surf.on_set_title() = [&](std::string title) {
		cout << "set title: " << title << endl;
	};

	surf.on_set_toplevel() = [&](void) {
	};

	surf.on_move() = [&](seat_resource_t seat, uint32_t serial) {
		surface_grabbing = true;
		compositor->start_grabbing_surface();
	};
}

void example_seat::bind(resource_t res, void *data) {
	std::cout << "client bind example_seat" << std::endl;
	res_list.push_back(res);

	seat_resource_t r(res);

	r.on_get_pointer() = [&](pointer_resource_t res) {
		//auto p = new pointer_resource_t(res);
		auto p = new example_pointer(this);
		p->bind(res);
		client_t c = res.get_client();
		example_view *v = compositor->find_view(c);
		v->bind_pointer(p);
	};

	r.on_get_keyboard() = [&](keyboard_resource_t res) {
		auto p = new keyboard_resource_t(res);
	};

	r.on_get_touch() = [&](touch_resource_t res) {
		auto p = new touch_resource_t(res);
	};

	r.send_capabilities(caps);
}

bool example_surface::bind_view(example_view *v) {
	if (view) {
		cerr << "the surface has already got a view." << endl;
		return false;
	}
	view = v;
	return true;
}

example_compositor::example_compositor(display_server_t disp)
	: global_t(disp, compositor_interface, 4, this, NULL),
	display(disp),
	shell(disp, this), seat(disp, this), shm(disp),
	session_active(true),
	focus(NULL), surface_grabbing(false),
	prev_pnt_x(0), prev_pnt_y(0)
{
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
	wrapper.on_pointer_button() =
		bind_mem_fn(&example_compositor::pointer_button, this);
}

void example_compositor::bind(resource_t res, void *data) {
	std::cout << "client bind example_compositor" << std::endl;

	auto compositor = new compositor_resource_t(res);
	compositor->on_create_surface() = [&](surface_resource_t surf_res) {
		//surface_resource_t surf_res(*resource_t::create(res.get_client(), surface_interface, res.get_version(), id));
		//new example_surface(surf_res);
		auto s = new example_surface(this);
		auto v = new example_view(s);

		s->bind(surf_res);
		s->bind_view(v);
		surface_list.push_back(s);

		view_list.push_back(v);
		view_client_dict[surf_res.get_client()] = v;
	};
}

void example_compositor::pointer_motion(uint32_t time, int32_t x, int32_t y) {
	//cout << "pointer motion (" << x << ", " << y << ")@"
	//	<< time << endl;
	int dx = x - prev_pnt_x;
	int dy = y - prev_pnt_y;
	prev_pnt_x = x;
	prev_pnt_y = y;
	if (surface_grabbing) {
		assert(focus);
		focus->get_view()->move(dx, dy);
		display.wake_epoll();
		return;
	}
	example_view *focus_v = NULL;
	for (auto &&v : view_list) {
		if (v->contain_point(x, y)) {
			focus_v = v;
			focus = v->get_surface();
			break;
			//s->get_resource()->
		}
	}
	if (focus_v) {
		focus_v->notify_motion(time, x, y);
	}
}

void example_compositor::pointer_button(uint32_t serial, uint32_t time,
		uint32_t button, pointer_button_state state,
		function<void()> parent_handler) {
	if (!focus) {
		parent_handler();
		return;
	}
	if (surface_grabbing) {
		if (state == pointer_button_state::released) {
			surface_grabbing = false;
		}
	}
	auto v = focus->get_view();
	v->notify_button(serial, time, button, state);
	display.wake_epoll();
}

// No weston version
int main(int argc, char *argv[]) {
	//weston_t weston;
	//weston.init(argc, argv);

	//display_server_t display("waylandpp-example");
	display_server_t display;

	example_compositor compositor(display);
	//compositor.set_weston(weston.ec);

	//display.init_shm();

	//gl_renderer glr(EGL_PLATFORM_WAYLAND_KHR, display.c_ptr(),
	//		NULL, //gl_renderer->alpha_attribs, NULL, 0);

	compositor.run();

	return 0;
}


