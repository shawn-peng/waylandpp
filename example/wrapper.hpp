
#ifndef __WRAPPER_HPP_
#define __WRAPPER_HPP_

#include <thread>
#include <future>
#include <unordered_map>

#include <GLES2/gl2.h>

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

class display_wrapper_t {
public:
	typedef void (*callback_t)(void *owner, void *data);
	struct notify_t {
		callback_t *func;
		void *data;
	};

private:
	bool running;
	bool has_pointer;
	bool has_keyboard;

	void *owner;
	void *userdata;

	gl_shader shader;
	promise<gl_shader *> initialized_shader;

	std::thread *td;

	void init_egl();

	//callback_t frame_callback;
	//callback_t quit_callback;
	std::unordered_map<std::string, callback_t> callback_dict;

	int width;
	int height;

public:
	display_wrapper_t();
	~display_wrapper_t();

	void attach(void *buffer);
	void draw(uint32_t serial = 0);
	void commit();
	void run();
	void start();
	void stop();
	void join();
	void dispatch();

	void set_owner(void *owner);
	void set_user_data(void *data);

	void *get_frame_buffer();

	int register_callback(std::string event, callback_t f);
	// events: frame, quit

	int get_width();
	int get_height();

	callback_t &on_frame();

	gl_shader *get_shader();
};

void gl_print_error();

#endif

