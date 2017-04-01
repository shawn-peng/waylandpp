
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

#ifndef CURSOR_HPP
#define CURSOR_HPP

#include <memory>
#include <string>
#include <list>
#include <wayland-server-core.hpp>
#include <wayland-server-protocol.hpp>

namespace wayland {

class shm_pool_t;

class shm_buffer_t {
private:
	//wl_shm_buffer *buffer;
	//shared_ptr<shm_pool_t> pool;
	shm_pool_t *pool;
	//std::list<buffer_resource_t> res_list;
	buffer_resource_t resource;
	int32_t width, height;
	int32_t stride;
	shm_format format;
	int offset;

public:
	shm_buffer_t(shm_pool_t &pl, int off, int w, int h,
			int s, shm_format fmt);
	static shm_buffer_t *from_resource(buffer_resource_t res);
	buffer_resource_t get_resource();
	void bind(buffer_resource_t res);
	void *get_data();
	int get_stride();
	shm_format get_format();
	int get_width();
	int get_height();

	void release();
};

class shm_pool_t {
private:
	//shared_ptr<shm_t> shm;
	//wl_shm_pool *pool;
	shm_pool_resource_t resource;
	int internal_refcount;
	int external_refcount;
	char *data;
	int32_t size;
	int32_t new_size;

	std::list<shm_resource_t> res_list;

public:
	shm_pool_t(int32_t size, int fd);

	void bind(shm_pool_resource_t res);

	void finish_resize();

	void *get_data();
};

class shm_t : public global_t {
private:
	display_server_t display;
	std::list<shm_resource_t> res_list;

public:
	shm_t(display_server_t disp);

	virtual void bind(resource_t res, void *data);
};

}

#endif
