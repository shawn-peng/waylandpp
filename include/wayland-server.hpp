/*
 * Copyright (c) 2016, Nils Christopher Brause
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

#ifndef WAYLAND_SERVER_HPP
#define WAYLAND_SERVER_HPP

/** \file */

#include <list>
#include <memory>
#include <string>
#include <cassert>
#include <wayland-server-core.h>
#include <wayland-util.hpp>

namespace wayland
{
  class display_t;
  class resource_t;

  class client_t
  {
  private:
    wl_client *client;

    client_t(wl_client *c);
    wl_client *c_ptr();

    friend class display_t;

  public:
    void flush();
    display_t get_display();
    void get_credentials(pid_t &pid, uid_t &uid, gid_t &gid);
    resource_t get_object(uint32_t id);
  };
  
  class display_t
  {
  private:
    struct display_data_t
    {
      unsigned int counter;
    };

    wl_display *display;
    display_data_t *data;

    display_t(wl_display* d);
    wl_display *c_ptr();

    friend class client_t;

  public:
    display_t();
    display_t(display_t &&d);
    display_t(const display_t &d);
    ~display_t();
    display_t &operator=(display_t &&d);
    display_t &operator=(const display_t &d);
    client_t create(int fd);
    uint32_t get_serial();
    uint32_t next_serial();
    void add_socket(std::string name);
  };

  class event_loop_t
  {
  private:
    struct event_loop_data_t
    {
      unsigned int counter;
    };

    wl_event_loop *event_loop;
    event_loop_data_t *data;

    event_loop_t(wl_event_loop* e);
    wl_event_loop *c_ptr();

  public:
    event_loop_t();
    event_loop_t(event_loop_t &&e);
    event_loop_t(const event_loop_t &e);
    ~event_loop_t();
    event_loop_t &operator=(event_loop_t &&e);
    event_loop_t &operator=(const event_loop_t &e);
  };
  
  class resource_t
  {
  private:
    wl_resource *resource;

    resource_t(wl_resource *r);
    wl_resource *c_ptr();

    friend class client_t;

  public:
  };
}

#endif
