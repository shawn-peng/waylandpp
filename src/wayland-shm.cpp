/*
 * Copyright (c) 2014, Nils Christopher Brause
 * Copyright (c) 2016-2017, Yisu Peng
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

#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>
#include <wayland-shm.hpp>

#include <wayland-server-protocol.hpp>

using namespace wayland;
using namespace wayland::detail;

shm_buffer_t::shm_buffer_t(shm_pool_t &pl,
			int off, int w, int h, int s, shm_format fmt)
	: pool(&pl)
{
	//if (offset < 0 || width <= 0 || height <= 0 || stride < width || 
	//		INT32_MAX / stride <= height ||
	//		offset > size - stride * height) {
	//	throw std::runtime_error("invalid buffer parameters!");
	//}

	width = w;
	height = h;
	format = fmt;
	stride = s;
	offset = off;
}

shm_buffer_t *shm_buffer_t::from_resource(buffer_resource_t res) {
	void *p = res.get_user_data();
	return reinterpret_cast<shm_buffer_t *>(p);
}

buffer_resource_t shm_buffer_t::get_resource() {
	return resource;
}

void shm_buffer_t::bind(buffer_resource_t res) {
	//res_list.push_back(res);
	resource = res;
	res.set_user_data(this);
}

void *shm_buffer_t::get_data() {
	assert(pool);
	if (!pool) {
		return NULL;
	}
	return (char *)pool->get_data() + offset;
	//return wl_shm_buffer_get_data(buffer);
}

int shm_buffer_t::get_stride() {
	return stride;
	//return wl_shm_buffer_get_stride(buffer);
}

shm_format shm_buffer_t::get_format() {
	return format;
	//return wl_shm_buffer_get_format(buffer);
}

int shm_buffer_t::get_width() {
	return width;
	//return wl_shm_buffer_get_width(buffer);
}

int shm_buffer_t::get_height() {
	return height;
	//return wl_shm_buffer_get_height(buffer);
}

void shm_buffer_t::swap_BR_channels() {
	uint8_t *sp = (uint8_t *)get_data();
	unsigned char (*p)[4];
	for (int i = 0; i < height; i++) {
		p = (decltype (p))&sp[i * stride];
		for (int j = 0; j < width; j++) {
			swap(p[j][0], p[j][2]);
		}
	}
}

void shm_buffer_t::release() {
	resource.send_release();
}

shm_pool_t::shm_pool_t(int32_t size, int fd) {
	internal_refcount = 1;
	external_refcount = 0;
	this->size = size;
	new_size = size;
	data = (char *)mmap(NULL, size,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
}

void shm_pool_t::bind(shm_pool_resource_t res) {
	resource = res;
	res.on_create_buffer() = [&] (buffer_resource_t buf_res, int32_t offset,
			int32_t width, int32_t height,
			int32_t stride, shm_format format) {

		if (offset < 0 || width <= 0 || height <= 0 || stride < width || 
				INT32_MAX / stride <= height ||
				offset > size - stride * height) {
			throw std::runtime_error("invalid buffer parameters!");
		}
		internal_refcount++;
		auto buf = new shm_buffer_t(*this, offset,
				width, height, stride, format);
		buf->bind(buf_res);
	};
	res.on_resize() = [&] (int32_t sz) {
		if (sz < size) {
			return;
		}
		new_size = sz;

		// If the compositor has taken references on this pool it
		// may be caching pointers into it. In that case we
		// defer the resize (which may move the entire mapping)
		// until the compositor finishes dereferencing the pool.
		if (external_refcount == 0) {
			finish_resize();
		}
	};
}

void shm_pool_t::finish_resize() {
	if (size == new_size) {
		return;
	}
	void *p = mremap(data, size, new_size, MREMAP_MAYMOVE);
	data = (char *)p;
	size = new_size;
}

void *shm_pool_t::get_data() {
	if (external_refcount && (size != new_size)) {
		cerr << "Buffer address requested when its parent pool "
				"has an external reference and a deferred resize "
				"pending.\n";
	}
	return data;
}

shm_t::shm_t(display_server_t disp)
	: display(disp),
	global_t(disp, shm_interface, 1, this, NULL)
{
}

void shm_t::bind(resource_t res, void *data) {
	shm_resource_t r(res);
	res_list.push_back(r);

	r.send_format(shm_format::argb8888);
	r.send_format(shm_format::rgba8888);

	r.on_create_pool() = [&] (shm_pool_resource_t res,
			int fd, int32_t size) {
		if (size <= 0) {
			cerr << "invalid size(" << size << ")" << endl;
		}
		auto pool = new shm_pool_t(size, fd);
		
		pool->bind(res);
	};
}


