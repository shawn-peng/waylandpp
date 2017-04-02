/*
 * Copyright (c) 2014, Nils Christopher Brause
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

/** \example opengles.cpp
 * This is an example of how to use the Wayland C++ bindings with OpenGL ES.
 */

#include <stdexcept>
#include <iostream>
#include <array>
#include <wayland-client.hpp>
#include <wayland-egl.hpp>
#include <GLES2/gl2.h>
#include <linux/input.h>
#include <wayland-cursor.hpp>

#include <CImg.h>

#include "helper.hpp"

static const char vertex_shader_source[] =
"attribute vec2 position;\n"
"varying vec2 v_texcoord;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(position, 0.0, 1.0);\n"
"	v_texcoord = position * vec2(0.5) + vec2(0.5);\n"
"}\n";


static const char fragment_brace[] =
"}\n";

static const char texture_fragment_shader_rgba[] =
"//precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"   gl_FragColor = texture2D(tex, v_texcoord);\n"
;

using namespace wayland;
using namespace cimg_library;


void gl_print_error() {
	GLenum err(glGetError());
	while (err != GL_NO_ERROR) {
		string error;
		switch (err) {
			case GL_INVALID_OPERATION:
				error = "INVALID_OPERATION";
				break;
			case GL_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case GL_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error = "INVALID_FRAMEBUFFER_OPERATION";
				break;
			default:
				error = "Unknown error";
				break;
		}
		std::cout << "GL_" << error;
		err = glGetError();
	}
}

static int
compile_shader(GLenum type, int count, const char **sources)
{
	GLuint s;
	char msg[512] = {0};
	GLint status;

	s = glCreateShader(type);
	glShaderSource(s, count, sources, NULL);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &status);
	if (!status) {
		glGetShaderInfoLog(s, sizeof msg, NULL, msg);
		throw std::runtime_error(msg);
		return GL_NONE;
	}

	return s;
}

struct gl_shader {
	GLuint program;
	GLuint vertex_shader, fragment_shader;
	GLint proj_uniform;
	GLint tex_uniforms[3];
	GLint alpha_uniform;
	GLint color_uniform;                        
	const char *vertex_source, *fragment_source;

	gl_shader();

	int init();
};

gl_shader::gl_shader() {
}

int gl_shader::init() {
	const GLchar *vssrc = vertex_shader_source;
	const GLchar *fragment_source = texture_fragment_shader_rgba;
	const char *fssrcs[3];
	fssrcs[0] = fragment_source;
	//fssrcs[1] = fragment_debug;
	fssrcs[1] = fragment_brace;
	int count = 2;
	GLint status;

	//glActiveTexture(GL_TEXTURE0);
	vertex_shader = compile_shader(GL_VERTEX_SHADER, 1, &vssrc);
	fragment_shader = compile_shader(GL_FRAGMENT_SHADER, count, fssrcs);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "texcoord");

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		char msg[512];
		glGetProgramInfoLog(program, sizeof msg, NULL, msg);
		throw std::runtime_error(msg);
		return -1;
	}

	proj_uniform = glGetUniformLocation(program, "proj");
	tex_uniforms[0] = glGetUniformLocation(program, "tex");
	tex_uniforms[1] = glGetUniformLocation(program, "tex1");
	tex_uniforms[2] = glGetUniformLocation(program, "tex2");
	alpha_uniform = glGetUniformLocation(program, "alpha");
	color_uniform = glGetUniformLocation(program, "color");

	return 0;
}
//int gl_shader::init() {
//	const GLchar *vssrc = vertex_shader_source;
//	const GLchar *fssrcs = texture_fragment_shader_rgba;
//	GLint status;
//
//	//glActiveTexture(GL_TEXTURE0);
//	vertex_shader = compile_shader(GL_VERTEX_SHADER, 1, &vssrc);
//	fragment_shader = compile_shader(GL_FRAGMENT_SHADER, 1, &fssrcs);
//
//	program = glCreateProgram();
//	glAttachShader(program, vertex_shader);
//	glAttachShader(program, fragment_shader);
//	glBindAttribLocation(program, 0, "position");
//	//glBindAttribLocation(program, 1, "texcoord");
//
//	glLinkProgram(program);
//	gl_print_error();
//	glGetProgramiv(program, GL_LINK_STATUS, &status);
//	if (!status) {
//		char msg[512];
//		glGetProgramInfoLog(program, sizeof msg, NULL, msg);
//		throw std::runtime_error(msg);
//		return -1;
//	}
//
//	//proj_uniform = glGetUniformLocation(program, "proj");
//	//tex_uniforms[0] = glGetUniformLocation(program, "tex");
//	//tex_uniforms[1] = glGetUniformLocation(program, "tex1");
//	//tex_uniforms[2] = glGetUniformLocation(program, "tex2");
//	//alpha_uniform = glGetUniformLocation(program, "alpha");
//	//color_uniform = glGetUniformLocation(program, "color");
//
//	return 0;
//}



// example Wayland client
class example {
  private:
	// global objects
	display_client_t display;
	registry_proxy_t registry;
	compositor_proxy_t compositor;
	shell_proxy_t shell;
	seat_proxy_t seat;
	shm_proxy_t shm;

	// local objects
	surface_proxy_t surface;
	shell_surface_proxy_t shell_surface;
	pointer_proxy_t pointer;
	keyboard_proxy_t keyboard;
	callback_proxy_t frame_cb;
	cursor_theme_t cursor_theme;
	cursor_image_t cursor_image;
	buffer_proxy_t cursor_buffer;
	surface_proxy_t cursor_surface;

	// EGL
	egl_window_t egl_window;
	EGLDisplay egldisplay;
	EGLSurface eglsurface;
	EGLContext eglcontext;
	gl_shader shader;

	bool running;
	bool has_pointer;
	bool has_keyboard;

	void *image_data;
	int width, height;

	void init_egl() {
		egldisplay = eglGetDisplay(display);
		if(egldisplay == EGL_NO_DISPLAY)
			throw std::runtime_error("eglGetDisplay");

		EGLint major, minor;
		if(eglInitialize(egldisplay, &major, &minor) == EGL_FALSE)
			throw std::runtime_error("eglInitialize");
		if(!((major == 1 && minor >= 4) || major >= 2))
			throw std::runtime_error("EGL version too old");

		if(eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
			throw std::runtime_error("eglBindAPI");

		std::array<EGLint, 13> config_attribs = {{
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_RED_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_BLUE_SIZE, 8,
				EGL_ALPHA_SIZE, 8,
				EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
				EGL_NONE
			}
		};

		EGLConfig config;
		EGLint num;
		if(eglChooseConfig(egldisplay, config_attribs.data(), &config, 1, &num) == EGL_FALSE || num == 0)
			throw std::runtime_error("eglChooseConfig");

		std::array<EGLint, 3> context_attribs = {{
				EGL_CONTEXT_CLIENT_VERSION, 2,
				EGL_NONE
			}
		};

		eglcontext = eglCreateContext(egldisplay, config, EGL_NO_CONTEXT, context_attribs.data());
		if(eglcontext == EGL_NO_CONTEXT)
			throw std::runtime_error("eglCreateContext");

		eglsurface = eglCreateWindowSurface(egldisplay, config, egl_window, NULL);
		if(eglsurface == EGL_NO_SURFACE)
			throw std::runtime_error("eglCreateWindowSurface");

		if(eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext) == EGL_FALSE)
			throw std::runtime_error("eglMakeCurrent");
	}

	void draw(uint32_t serial = 0) {
		float h = ((serial >> 4) & 0xFF) / 255.0;
		float s = 1, v = 1;

		int hi = h * 6;
		float f = h * 6 - hi;
		float p = v * (1 - s);
		float q = v * (1 - s * f);
		float t = v * (1 - s * (1 - f));
		float r, g, b;

		switch(hi) {
			case 1:
				r = q;
				g = v;
				b = p;
				break;
			case 2:
				r = p;
				g = v;
				b = t;
				break;
			case 3:
				r = p;
				g = q;
				b = v;
				break;
			case 4:
				r = t;
				g = p;
				b = v;
				break;
			case 5:
				r = v;
				g = p;
				b = q;
				break;
			default: // 0,6
				r = v;
				g = t;
				b = p;
				break;
		}

		glUseProgram(shader.program);

		// draw stuff
		//glClearColor(r, g, b, 0.5f);
		//glClear(GL_COLOR_BUFFER_BIT);

		static const GLfloat verts[] = { 
			-1.0f,  1.0f,
			-1.0f, -1.0f,
			1.0f, -1.0f,
			1.0f,  1.0f,
		};

		GLint uniform_tex
			= glGetUniformLocation(shader.program, "tex");

		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0,
					GL_RGB, 
					width,
					height,
					0,
					GL_RGB, GL_UNSIGNED_BYTE,
					image_data);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(uniform_tex, 0);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, &verts);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glDisableVertexAttribArray(0);
		

		// schedule next draw
		frame_cb = surface.frame();
		frame_cb.on_done() = bind_mem_fn(&example::draw, this);

		// swap buffers
		if(eglSwapBuffers(egldisplay, eglsurface) == EGL_FALSE)
			throw std::runtime_error("eglSwapBuffers");
	}

  public:
	example() {
		CImg<unsigned char> image("example/ocean.jpg");
		width = image.width();
		height = image.height();
		image_data = new unsigned char[image.size()];
		uint8_t (*p)[width][3] = (decltype(p))image_data;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				p[i][j][2] = image(j, i, 0, 0);
				p[i][j][1] = image(j, i, 0, 1);
				p[i][j][0] = image(j, i, 0, 2);
			}
		}
		

		// retrieve global objects
		registry = display.get_registry();
		registry.on_global() = [&](uint32_t name, std::string interface, uint32_t version) {
			if(interface == "wl_compositor")
				registry.bind(name, compositor, version);
			else if(interface == "wl_shell")
				registry.bind(name, shell, version);
			else if(interface == "wl_seat")
				registry.bind(name, seat, version);
			else if(interface == "wl_shm")
				registry.bind(name, shm, version);
		};
		display.dispatch();

		seat.on_capabilities() = [&](seat_capability capability) {
			has_keyboard = capability & seat_capability::keyboard;
			has_pointer = capability & seat_capability::pointer;
		};
		display.dispatch();

		if(!has_keyboard)
			throw std::runtime_error("No keyboard found.");
		if(!has_pointer)
			throw std::runtime_error("No pointer found.");

		// create a surface
		surface = compositor.create_surface();
		shell_surface = shell.get_shell_surface(surface);

		shell_surface.on_ping() = [&](uint32_t serial) {
			shell_surface.pong(serial);
		};
		shell_surface.set_title("Window");
		shell_surface.set_toplevel();

		// Get input devices
		pointer = seat.get_pointer();
		keyboard = seat.get_keyboard();

		// load cursor theme
		cursor_theme = cursor_theme_t("default", 16, shm);
		cursor_t cursor = cursor_theme.get_cursor("cross");
		cursor_image = cursor.image(0);
		cursor_buffer = cursor_image.get_buffer();

		// create cursor surface
		cursor_surface = compositor.create_surface();

		// draw cursor
		pointer.on_enter() = [&](uint32_t serial, surface_proxy_t, int32_t, int32_t) {
			cursor_surface.attach(cursor_buffer, 0, 0);
			cursor_surface.damage(0, 0, cursor_image.width(), cursor_image.height());
			cursor_surface.commit();
			pointer.set_cursor(serial, cursor_surface, 0, 0);
		};

		// window movement
		pointer.on_button() = [&](uint32_t serial, uint32_t time, uint32_t button, pointer_button_state state) {
			if(button == BTN_LEFT && state == pointer_button_state::pressed)
				shell_surface.move(seat, serial);
		};

		// press 'q' to exit
		keyboard.on_key() = [&](uint32_t, uint32_t, uint32_t key, keyboard_key_state state) {
			if(key == KEY_Q && state == keyboard_key_state::pressed)
				running = false;
		};

		// intitialize egl
		//egl_window = egl_window_t(surface, 320, 240);
		egl_window = egl_window_t(surface, width, height);
		init_egl();

		shader.init();

		// draw stuff
		draw();
	}

	~example() {
		// finialize EGL
		//if(eglDestroyContext(egldisplay, eglcontext) == EGL_FALSE)
		//	throw std::runtime_error("eglDestroyContext");
		//if(eglTerminate(egldisplay) == EGL_FALSE)
		//	throw std::runtime_error("eglTerminate");
		eglDestroyContext(egldisplay, eglcontext);
		eglTerminate(egldisplay);
	}

	void run() {
		// event loop
		running = true;
		while(running)
			display.dispatch();
	}
};

int main() {
	example e;
	e.run();
	return 0;
}
