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

#include "config.h"

#include <signal.h>

#include <iostream>
#include <queue>
#include <vector>
#include <list>

#include <wayland-util.h>

#include <wayland-server.hpp>

//#include <weston-log.h>
//#include <egl-renderer.hpp>
//#include <libweston-2/compositor.h>

//#include "libinput-seat.hpp"
//#include <libunwind.h>
#include <libinput.h>
//#include <dlfcn.h>
//#include <sys/wait.h>
#include <pixman-1/pixman.h>

#include <EGL/eglext.h>

#include "basics.h"
//#include "shared/helpers.h"
//#include "shared/config-parser.h"

//#include "backend.h"

#include "weston.h"

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

	weston_compositor *weston;

	signal_t destroy_signal;

	//struct wl_display *wl_display;
	//struct weston_desktop_xwayland *xwayland;
	//const struct weston_desktop_xwayland_interface *xwayland_interface;

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

	void set_weston(weston_compositor *w) {
		weston = w;
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


/*
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
*/


// load weston backend version

/*
// weston depending functions
static struct wl_list child_process_list;
static struct weston_compositor *segv_compositor;

typedef void (*weston_process_cleanup_func_t)(struct weston_process *process,
					    int status);

struct weston_process {
	pid_t pid;
	weston_process_cleanup_func_t cleanup;
	struct wl_list link;
};

static int
sigchld_handler(int signal_number, void *data)
{
	struct weston_process *p;
	int status;
	pid_t pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		wl_list_for_each(p, &child_process_list, link) {
			if (p->pid == pid)
				break;
		}

		if (&p->link == &child_process_list) {
			weston_log("unknown child process exited\n");
			continue;
		}

		wl_list_remove(&p->link);
		p->cleanup(p, status);
	}

	if (pid < 0 && errno != ECHILD)
		weston_log("waitpid error %m\n");

	return 1;
}

static void
print_backtrace(void)
{
	unw_cursor_t cursor;
	unw_context_t context;
	unw_word_t off;
	unw_proc_info_t pip;
	int ret, i = 0;
	char procname[256];
	const char *filename;
	Dl_info dlinfo;

	pip.unwind_info = NULL;
	ret = unw_getcontext(&context);
	if (ret) {
		weston_log("unw_getcontext: %d\n", ret);
		return;
	}

	ret = unw_init_local(&cursor, &context);
	if (ret) {
		weston_log("unw_init_local: %d\n", ret);
		return;
	}

	ret = unw_step(&cursor);
	while (ret > 0) {
		ret = unw_get_proc_info(&cursor, &pip);
		if (ret) {
			weston_log("unw_get_proc_info: %d\n", ret);
			break;
		}

		ret = unw_get_proc_name(&cursor, procname, 256, &off);
		if (ret && ret != -UNW_ENOMEM) {
			if (ret != -UNW_EUNSPEC)
				weston_log("unw_get_proc_name: %d\n", ret);
			procname[0] = '?';
			procname[1] = 0;
		}

		if (dladdr((void *)(pip.start_ip + off), &dlinfo) && dlinfo.dli_fname &&
		    *dlinfo.dli_fname)
			filename = dlinfo.dli_fname;
		else
			filename = "?";

		weston_log("%u: %s (%s%s+0x%x) [%p]\n", i++, filename, procname,
			   ret == -UNW_ENOMEM ? "..." : "", (int)off, (void *)(pip.start_ip + off));

		ret = unw_step(&cursor);
		if (ret < 0)
			weston_log("unw_step: %d\n", ret);
	}
}

static int on_term_signal(int signal_number, void *data)
{
	struct wl_display *display = (decltype(display))data;

	weston_log("caught signal %d\n", signal_number);
	wl_display_terminate(display);

	return 1;
}

static void
on_caught_signal(int s, siginfo_t *siginfo, void *context)
{
	// This signal handler will do a best-effort backtrace, and
	// then call the backend restore function, which will switch
	// back to the vt we launched from or ungrab X etc and then
	// raise SIGTRAP.  If we run weston under gdb from X or a
	// different vt, and tell gdb "handle *s* nostop", this
	// will allow weston to switch back to gdb on crash and then
	// gdb will catch the crash with SIGTRAP.

	weston_log("caught signal: %d\n", s);

	print_backtrace();

	segv_compositor->backend->restore(segv_compositor);

	raise(SIGTRAP);
}

static void
catch_signals(void)
{
	struct sigaction action;

	action.sa_flags = SA_SIGINFO | SA_RESETHAND;
	action.sa_sigaction = on_caught_signal;
	sigemptyset(&action.sa_mask);
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGABRT, &action, NULL);
}
static int
load_configuration(struct weston_config **config, int32_t noconfig,
		   const char *config_file)
{
	const char *file = "weston.ini";
	const char *full_path;

	*config = NULL;

	if (config_file)
		file = config_file;

	if (noconfig == 0)
		*config = weston_config_parse(file);

	if (*config) {
		full_path = weston_config_get_full_path(*config);

		weston_log("Using config file '%s'\n", full_path);
		setenv(WESTON_CONFIG_FILE_ENV_VAR, full_path, 1);

		return 0;
	}

	if (config_file && noconfig == 0) {
		weston_log("fatal: error opening or reading config file"
			   " '%s'.\n", config_file);

		return -1;
	}

	weston_log("Starting with no config file.\n");
	setenv(WESTON_CONFIG_FILE_ENV_VAR, "", 1);

	return 0;
}

static int
weston_compositor_init_config(struct weston_compositor *ec,
			      struct weston_config *config)
{
	struct xkb_rule_names xkb_names;
	struct weston_config_section *s;
	int repaint_msec;
	int vt_switching;

	s = weston_config_get_section(config, "keyboard", NULL, NULL);
	weston_config_section_get_string(s, "keymap_rules",
					 (char **) &xkb_names.rules, NULL);
	weston_config_section_get_string(s, "keymap_model",
					 (char **) &xkb_names.model, NULL);
	weston_config_section_get_string(s, "keymap_layout",
					 (char **) &xkb_names.layout, NULL);
	weston_config_section_get_string(s, "keymap_variant",
					 (char **) &xkb_names.variant, NULL);
	weston_config_section_get_string(s, "keymap_options",
					 (char **) &xkb_names.options, NULL);

	if (weston_compositor_set_xkb_rule_names(ec, &xkb_names) < 0)
		return -1;

	weston_config_section_get_int(s, "repeat-rate",
				      &ec->kb_repeat_rate, 40);
	weston_config_section_get_int(s, "repeat-delay",
				      &ec->kb_repeat_delay, 400);

	weston_config_section_get_bool(s, "vt-switching",
				       &vt_switching, true);
	ec->vt_switching = vt_switching;

	s = weston_config_get_section(config, "core", NULL, NULL);
	weston_config_section_get_int(s, "repaint-window", &repaint_msec,
				      ec->repaint_msec);
	if (repaint_msec < -10 || repaint_msec > 1000) {
		weston_log("Invalid repaint_window value in config: %d\n",
			   repaint_msec);
	} else {
		ec->repaint_msec = repaint_msec;
	}
	weston_log("Output repaint window is %d ms maximum.\n",
		   ec->repaint_msec);

	return 0;
}

static char *
copy_command_line(int argc, char * const argv[])
{
	FILE *fp;
	char *str = NULL;
	size_t size = 0;
	int i;

	fp = open_memstream(&str, &size);
	if (!fp)
		return NULL;

	fprintf(fp, "%s", argv[0]);
	for (i = 1; i < argc; i++)
		fprintf(fp, " %s", argv[i]);
	fclose(fp);

	return str;
}

static int
usage(int error_code)
{
	fprintf(stderr,
		"Usage: weston [OPTIONS]\n\n"
		"This is weston version " VERSION ", the Wayland reference compositor.\n"
		"Weston supports multiple backends, and depending on which backend is in use\n"
		"different options will be accepted.\n\n"


		"Core options:\n\n"
		"  --version\t\tPrint weston version\n"
		"  -B, --backend=MODULE\tBackend module, one of\n"
#if defined(BUILD_DRM_COMPOSITOR)
			"\t\t\t\tdrm-backend.so\n"
#endif
#if defined(BUILD_FBDEV_COMPOSITOR)
			"\t\t\t\tfbdev-backend.so\n"
#endif
#if defined(BUILD_HEADLESS_COMPOSITOR)
			"\t\t\t\theadless-backend.so\n"
#endif
#if defined(BUILD_RDP_COMPOSITOR)
			"\t\t\t\trdp-backend.so\n"
#endif
#if defined(BUILD_WAYLAND_COMPOSITOR)
			"\t\t\t\twayland-backend.so\n"
#endif
#if defined(BUILD_X11_COMPOSITOR)
			"\t\t\t\tx11-backend.so\n"
#endif
		"  --shell=MODULE\tShell module, defaults to desktop-shell.so\n"
		"  -S, --socket=NAME\tName of socket to listen on\n"
		"  -i, --idle-time=SECS\tIdle time in seconds\n"
		"  --modules\t\tLoad the comma-separated list of modules\n"
		"  --log=FILE\t\tLog to the given file\n"
		"  -c, --config=FILE\tConfig file to load, defaults to weston.ini\n"
		"  --no-config\t\tDo not read weston.ini\n"
		"  -h, --help\t\tThis help message\n\n");

#if defined(BUILD_DRM_COMPOSITOR)
	fprintf(stderr,
		"Options for drm-backend.so:\n\n"
		"  --connector=ID\tBring up only this connector\n"
		"  --seat=SEAT\t\tThe seat that weston should run on\n"
		"  --tty=TTY\t\tThe tty to use\n"
		"  --use-pixman\t\tUse the pixman (CPU) renderer\n"
		"  --current-mode\tPrefer current KMS mode over EDID preferred mode\n\n");
#endif

#if defined(BUILD_FBDEV_COMPOSITOR)
	fprintf(stderr,
		"Options for fbdev-backend.so:\n\n"
		"  --tty=TTY\t\tThe tty to use\n"
		"  --device=DEVICE\tThe framebuffer device to use\n"
		"\n");
#endif

#if defined(BUILD_HEADLESS_COMPOSITOR)
	fprintf(stderr,
		"Options for headless-backend.so:\n\n"
		"  --width=WIDTH\t\tWidth of memory surface\n"
		"  --height=HEIGHT\tHeight of memory surface\n"
		"  --transform=TR\tThe output transformation, TR is one of:\n"
		"\tnormal 90 180 270 flipped flipped-90 flipped-180 flipped-270\n"
		"  --use-pixman\t\tUse the pixman (CPU) renderer (default: no rendering)\n"
		"  --no-outputs\t\tDo not create any virtual outputs\n"
		"\n");
#endif

#if defined(BUILD_RDP_COMPOSITOR)
	fprintf(stderr,
		"Options for rdp-backend.so:\n\n"
		"  --width=WIDTH\t\tWidth of desktop\n"
		"  --height=HEIGHT\tHeight of desktop\n"
		"  --env-socket\t\tUse socket defined in RDP_FD env variable as peer connection\n"
		"  --address=ADDR\tThe address to bind\n"
		"  --port=PORT\t\tThe port to listen on\n"
		"  --no-clients-resize\tThe RDP peers will be forced to the size of the desktop\n"
		"  --rdp4-key=FILE\tThe file containing the key for RDP4 encryption\n"
		"  --rdp-tls-cert=FILE\tThe file containing the certificate for TLS encryption\n"
		"  --rdp-tls-key=FILE\tThe file containing the private key for TLS encryption\n"
		"\n");
#endif

#if defined(BUILD_WAYLAND_COMPOSITOR)
	fprintf(stderr,
		"Options for wayland-backend.so:\n\n"
		"  --width=WIDTH\t\tWidth of Wayland surface\n"
		"  --height=HEIGHT\tHeight of Wayland surface\n"
		"  --scale=SCALE\t\tScale factor of output\n"
		"  --fullscreen\t\tRun in fullscreen mode\n"
		"  --use-pixman\t\tUse the pixman (CPU) renderer\n"
		"  --output-count=COUNT\tCreate multiple outputs\n"
		"  --sprawl\t\tCreate one fullscreen output for every parent output\n"
		"  --display=DISPLAY\tWayland display to connect to\n\n");
#endif

#if defined(BUILD_X11_COMPOSITOR)
	fprintf(stderr,
		"Options for x11-backend.so:\n\n"
		"  --width=WIDTH\t\tWidth of X window\n"
		"  --height=HEIGHT\tHeight of X window\n"
		"  --scale=SCALE\t\tScale factor of output\n"
		"  --fullscreen\t\tRun in fullscreen mode\n"
		"  --use-pixman\t\tUse the pixman (CPU) renderer\n"
		"  --output-count=COUNT\tCreate multiple outputs\n"
		"  --no-input\t\tDont create input devices\n\n");
#endif

	exit(error_code);
}
*/

// load weston backend without config (default config)
int main(int argc, char *argv[]) {
	weston_log_set_handler(vlog, vlog_continue);
	//wl_log_set_handler_server(custom_handler);

	//struct wl_event_source *signals[4];
	//struct wl_event_loop *loop;
	//int i, fd;
	//char *cmdline;
	//char *backend = NULL;
	//char *shell_conf = NULL;
	//int32_t xwayland = 0;
	////char *modules = NULL;
	//char *option_modules = NULL;
	//char *log = NULL;
	//char *server_socket = NULL;
	//int32_t idle_time = -1;
	//int32_t help = 0;
	//char *socket_name = NULL;
	//int32_t version = 0;
	//int32_t noconfig = 0;
	//int32_t numlock_on;
	//char *config_file = NULL;
	//struct weston_config *config = NULL;
	//struct weston_config_section *section;
	//struct wl_client *primary_client;
	//struct wl_listener primary_client_destroyed;
	////struct weston_seat *seat;
	//struct wet_compositor user_data;
	//int require_input;

	display_t display("waylandpp-example");

	const struct weston_option core_options[] = {
		{ WESTON_OPTION_STRING, "backend", 'B', &backend },
		{ WESTON_OPTION_STRING, "shell", 0, &shell_conf },
		{ WESTON_OPTION_STRING, "socket", 'S', &socket_name },
		{ WESTON_OPTION_INTEGER, "idle-time", 'i', &idle_time },
		{ WESTON_OPTION_BOOLEAN, "xwayland", 0, &xwayland },
		{ WESTON_OPTION_STRING, "modules", 0, &option_modules },
		{ WESTON_OPTION_STRING, "log", 0, &log },
		{ WESTON_OPTION_BOOLEAN, "help", 'h', &help },
		{ WESTON_OPTION_BOOLEAN, "version", 0, &version },
		{ WESTON_OPTION_BOOLEAN, "no-config", 0, &noconfig },
		{ WESTON_OPTION_STRING, "config", 'c', &config_file },
	};

	cmdline = copy_command_line(argc, argv);
	parse_options(core_options, ARRAY_LENGTH(core_options), &argc, argv);

	//auto weston = new weston_compositor();
	//weston->wl_display = display.c_ptr();

	auto weston = weston_compositor_create(display.c_ptr(), &user_data);
	
	//if (weston_compositor_init_config(weston, config) < 0)
	//	exit(1);

	//weston_config_section_get_bool(section, "require-input",
	//			       &require_input, true);
	//weston->require_input = require_input;

	loop = wl_display_get_event_loop(display.c_ptr());
	//signals[0] = wl_event_loop_add_signal(loop, SIGTERM, on_term_signal,
	//				      display.c_ptr());
	//signals[1] = wl_event_loop_add_signal(loop, SIGINT, on_term_signal,
	//				      display.c_ptr());
	//signals[2] = wl_event_loop_add_signal(loop, SIGQUIT, on_term_signal,
	//				      display.c_ptr());

	wl_list_init(&child_process_list);
	//signals[3] = wl_event_loop_add_signal(loop, SIGCHLD, sigchld_handler,
	//				      NULL);

	//if (!signals[0] || !signals[1] || !signals[2] || !signals[3])
	//	exit(1);


	
	user_data.config = config;
	user_data.parsed_options = NULL;
	weston->user_data = &user_data;


	section = weston_config_get_section(config, "core", NULL, NULL);

	//load_wayland_backend(weston, &argc, argv, config);

	backend_t backend = load_backend("wayland-backend.so", NULL);





	example_compositor compositor(display);
	compositor.set_weston(weston);

	example_shell shell(display);
	example_seat seat(display);
	//example_shm shm(display);
	display.init_shm();

	//gl_renderer glr(EGL_PLATFORM_WAYLAND_KHR, display.c_ptr(),
	//		NULL, //gl_renderer->alpha_attribs, NULL, 0);

	display.run();
	return 0;
}

/*
// load weston backend with config
int main(int argc, char *argv[]) {
	weston_log_set_handler(vlog, vlog_continue);
	//wl_log_set_handler_server(custom_handler);

	struct wl_event_source *signals[4];
	struct wl_event_loop *loop;
	int i, fd;
	char *cmdline;
	char *backend = NULL;
	char *shell_conf = NULL;
	int32_t xwayland = 0;
	//char *modules = NULL;
	char *option_modules = NULL;
	char *log = NULL;
	char *server_socket = NULL;
	int32_t idle_time = -1;
	int32_t help = 0;
	char *socket_name = NULL;
	int32_t version = 0;
	int32_t noconfig = 0;
	int32_t numlock_on;
	char *config_file = NULL;
	struct weston_config *config = NULL;
	struct weston_config_section *section;
	struct wl_client *primary_client;
	struct wl_listener primary_client_destroyed;
	//struct weston_seat *seat;
	struct wet_compositor user_data;
	int require_input;

	display_t display("waylandpp-example");

	const struct weston_option core_options[] = {
		{ WESTON_OPTION_STRING, "backend", 'B', &backend },
		{ WESTON_OPTION_STRING, "shell", 0, &shell_conf },
		{ WESTON_OPTION_STRING, "socket", 'S', &socket_name },
		{ WESTON_OPTION_INTEGER, "idle-time", 'i', &idle_time },
		{ WESTON_OPTION_BOOLEAN, "xwayland", 0, &xwayland },
		{ WESTON_OPTION_STRING, "modules", 0, &option_modules },
		{ WESTON_OPTION_STRING, "log", 0, &log },
		{ WESTON_OPTION_BOOLEAN, "help", 'h', &help },
		{ WESTON_OPTION_BOOLEAN, "version", 0, &version },
		{ WESTON_OPTION_BOOLEAN, "no-config", 0, &noconfig },
		{ WESTON_OPTION_STRING, "config", 'c', &config_file },
	};

	cmdline = copy_command_line(argc, argv);
	parse_options(core_options, ARRAY_LENGTH(core_options), &argc, argv);

	//auto weston = new weston_compositor();
	//weston->wl_display = display.c_ptr();

	auto weston = weston_compositor_create(display.c_ptr(), &user_data);
	
	if (weston_compositor_init_config(weston, config) < 0)
		exit(1);

	weston_config_section_get_bool(section, "require-input",
				       &require_input, true);
	weston->require_input = require_input;

	loop = wl_display_get_event_loop(display.c_ptr());
	signals[0] = wl_event_loop_add_signal(loop, SIGTERM, on_term_signal,
					      display.c_ptr());
	signals[1] = wl_event_loop_add_signal(loop, SIGINT, on_term_signal,
					      display.c_ptr());
	signals[2] = wl_event_loop_add_signal(loop, SIGQUIT, on_term_signal,
					      display.c_ptr());

	wl_list_init(&child_process_list);
	signals[3] = wl_event_loop_add_signal(loop, SIGCHLD, sigchld_handler,
					      NULL);

	if (!signals[0] || !signals[1] || !signals[2] || !signals[3])
		exit(1);


	
	user_data.config = config;
	user_data.parsed_options = NULL;
	weston->user_data = &user_data;

	load_configuration(&config, noconfig, config_file);

	section = weston_config_get_section(config, "core", NULL, NULL);

	load_wayland_backend(weston, &argc, argv, config);

	//backend_t backend = load_backend("wayland-backend.so", NULL);

	example_compositor compositor(display);
	compositor.set_weston(weston);

	example_shell shell(display);
	example_seat seat(display);
	//example_shm shm(display);
	display.init_shm();

	//gl_renderer glr(EGL_PLATFORM_WAYLAND_KHR, display.c_ptr(),
	//		NULL, //gl_renderer->alpha_attribs, NULL, 0);

	display.run();
	return 0;
}
*/

/*
// weston fully loaded version
int main(int argc, char *argv[]) {
	// ==================================================================
	// Weston init
	int ret = EXIT_FAILURE;
	char *cmdline;
	struct wl_display *display;
	struct weston_compositor *ec;
	struct wl_event_source *signals[4];
	struct wl_event_loop *loop;
	int i, fd;
	char *backend = NULL;
	char *shell = NULL;
	int32_t xwayland = 0;
	char *modules = NULL;
	char *option_modules = NULL;
	char *log = NULL;
	char *server_socket = NULL;
	int32_t idle_time = -1;
	int32_t help = 0;
	char *socket_name = NULL;
	int32_t version = 0;
	int32_t noconfig = 0;
	int32_t numlock_on;
	char *config_file = NULL;
	struct weston_config *config = NULL;
	struct weston_config_section *section;
	struct wl_client *primary_client;
	struct wl_listener primary_client_destroyed;
	struct weston_seat *seat;
	struct wet_compositor user_data;
	int require_input;

	const struct weston_option core_options[] = {
		{ WESTON_OPTION_STRING, "backend", 'B', &backend },
		{ WESTON_OPTION_STRING, "shell", 0, &shell },
		{ WESTON_OPTION_STRING, "socket", 'S', &socket_name },
		{ WESTON_OPTION_INTEGER, "idle-time", 'i', &idle_time },
		{ WESTON_OPTION_BOOLEAN, "xwayland", 0, &xwayland },
		{ WESTON_OPTION_STRING, "modules", 0, &option_modules },
		{ WESTON_OPTION_STRING, "log", 0, &log },
		{ WESTON_OPTION_BOOLEAN, "help", 'h', &help },
		{ WESTON_OPTION_BOOLEAN, "version", 0, &version },
		{ WESTON_OPTION_BOOLEAN, "no-config", 0, &noconfig },
		{ WESTON_OPTION_STRING, "config", 'c', &config_file },
	};

	cmdline = copy_command_line(argc, argv);
	parse_options(core_options, ARRAY_LENGTH(core_options), &argc, argv);

	if (help) {
		free(cmdline);
		usage(EXIT_SUCCESS);
	}

	if (version) {
		printf(PACKAGE_STRING "\n");
		free(cmdline);

		return EXIT_SUCCESS;
	}

	weston_log_set_handler(vlog, vlog_continue);
	weston_log_file_open(log);

	weston_log("%s\n"
		   STAMP_SPACE "%s\n"
		   STAMP_SPACE "Bug reports to: %s\n"
		   STAMP_SPACE "Build: %s\n",
		   PACKAGE_STRING, PACKAGE_URL, PACKAGE_BUGREPORT,
		   BUILD_ID);
	weston_log("Command line: %s\n", cmdline);
	free(cmdline);
	log_uname();

	verify_xdg_runtime_dir();

	display = wl_display_create();

	loop = wl_display_get_event_loop(display);
	signals[0] = wl_event_loop_add_signal(loop, SIGTERM, on_term_signal,
					      display);
	signals[1] = wl_event_loop_add_signal(loop, SIGINT, on_term_signal,
					      display);
	signals[2] = wl_event_loop_add_signal(loop, SIGQUIT, on_term_signal,
					      display);

	wl_list_init(&child_process_list);
	signals[3] = wl_event_loop_add_signal(loop, SIGCHLD, sigchld_handler,
					      NULL);

	if (!signals[0] || !signals[1] || !signals[2] || !signals[3])
		goto out_signals;

	if (load_configuration(&config, noconfig, config_file) < 0)
		goto out_signals;
	user_data.config = config;
	user_data.parsed_options = NULL;

	section = weston_config_get_section(config, "core", NULL, NULL);

	if (!backend) {
		weston_config_section_get_string(section, "backend", &backend,
						 NULL);
		if (!backend)
			backend = weston_choose_default_backend();
	}

	ec = weston_compositor_create(display, &user_data);
	if (ec == NULL) {
		weston_log("fatal: failed to create compositor\n");
		goto out;
	}

	if (weston_compositor_init_config(ec, config) < 0)
		goto out;

	weston_config_section_get_bool(section, "require-input",
				       &require_input, true);
	ec->require_input = require_input;

	if (load_backend(ec, backend, &argc, argv, config) < 0) {
		weston_log("fatal: failed to create compositor backend\n");
		goto out;
	}

	weston_pending_output_coldplug(ec);

	catch_signals();
	segv_compositor = ec;

	if (idle_time < 0)
		weston_config_section_get_int(section, "idle-time", &idle_time, -1);
	if (idle_time < 0)
		idle_time = 300;

	ec->idle_time = idle_time;
	ec->default_pointer_grab = NULL;
	ec->exit = handle_exit;

	weston_compositor_log_capabilities(ec);

	server_socket = getenv("WAYLAND_SERVER_SOCKET");
	if (server_socket) {
		weston_log("Running with single client\n");
		if (!safe_strtoint(server_socket, &fd))
			fd = -1;
	} else {
		fd = -1;
	}

	if (fd != -1) {
		primary_client = wl_client_create(display, fd);
		if (!primary_client) {
			weston_log("fatal: failed to add client: %m\n");
			goto out;
		}
		primary_client_destroyed.notify =
			handle_primary_client_destroyed;
		wl_client_add_destroy_listener(primary_client,
					       &primary_client_destroyed);
	} else if (weston_create_listening_socket(display, socket_name)) {
		goto out;
	}

	if (!shell)
		weston_config_section_get_string(section, "shell", &shell,
						 "desktop-shell.so");

	if (wet_load_shell(ec, shell, &argc, argv) < 0)
		goto out;

	weston_config_section_get_string(section, "modules", &modules, "");
	if (load_modules(ec, modules, &argc, argv, &xwayland) < 0)
		goto out;

	if (load_modules(ec, option_modules, &argc, argv, &xwayland) < 0)
		goto out;

	if (!xwayland)
		weston_config_section_get_bool(section, "xwayland", &xwayland,
					       false);
	if (xwayland) {
		if (wet_load_xwayland(ec) < 0)
			goto out;
	}

	section = weston_config_get_section(config, "keyboard", NULL, NULL);
	weston_config_section_get_bool(section, "numlock-on", &numlock_on, 0);
	if (numlock_on) {
		wl_list_for_each(seat, &ec->seat_list, link) {
			struct weston_keyboard *keyboard =
				weston_seat_get_keyboard(seat);

			if (keyboard)
				weston_keyboard_set_locks(keyboard,
							  WESTON_NUM_LOCK,
							  WESTON_NUM_LOCK);
		}
	}

	for (i = 1; i < argc; i++)
		weston_log("fatal: unhandled option: %s\n", argv[i]);
	if (argc > 1)
		goto out;

	weston_compositor_wake(ec);
	// Weston init end
	// ==================================================================


	// ==================================================================
	// MY COMPOSITOR 

	example_compositor compositor(display);
	compositor.set_weston(weston);

	example_shell shell(display);
	example_seat seat(display);
	//example_shm shm(display);
	display.init_shm();

	//gl_renderer glr(EGL_PLATFORM_WAYLAND_KHR, display.c_ptr(),
	//		NULL, //gl_renderer->alpha_attribs, NULL, 0);

	display.run();


	// Allow for setting return exit code after
	// wl_display_run returns normally. This is
	// useful for devs/testers and automated tests
	// that want to indicate failure status to
	// testing infrastructure above
	
	ret = ec->exit_code;

out:
	// free(NULL) is valid, and it won't be NULL if it's used
	free(user_data.parsed_options);

	weston_compositor_destroy(ec);

out_signals:
	for (i = ARRAY_LENGTH(signals) - 1; i >= 0; i--)
		if (signals[i])
			wl_event_source_remove(signals[i]);

	wl_display_destroy(display);

	weston_log_file_close();

	if (config)
		weston_config_destroy(config);
	free(config_file);
	free(backend);
	free(shell);
	free(socket_name);
	free(option_modules);
	free(log);
	free(modules);

	return ret;
}*/


