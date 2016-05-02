/*
 * Copyright (c) 2014-2016, Nils Christopher Brause
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

#include <iostream>
#include <wayland-server.hpp>
//#include <wayland-client-protocol.hpp>
#include <unordered_map>

using namespace wayland;
using namespace wayland::detail;

// user data interface

std::unordered_map<void*, void*> user_data;

void set_user_data(void* object, void *data)
{
  user_data[object] = data;
}

void *get_user_data(void *object)
{
  return user_data[object];
}

// client interface 

client_t::client_t(wl_client *c)
  : client(c)
{
  c_ptr();
}

wl_client *client_t::c_ptr()
{
  if(!client)
    throw std::runtime_error("client is NULL.");
  return client;
}

void client_t::flush()
{
  wl_client_flush(c_ptr());
}

display_t client_t::get_display()
{
  return display_t(wl_client_get_display(client));
}

void client_t::get_credentials(pid_t &pid, uid_t &uid, gid_t &gid)
{
  wl_client_get_credentials(c_ptr(), &pid, &uid, &gid);
}

resource_t client_t::get_object(uint32_t id)
{
  return resource_t(wl_client_get_object(client, id));
}

// display interface

display_t::display_t(wl_display *d)
  : display(d)
{
  c_ptr();
  display_data_t *data = reinterpret_cast<display_data_t*>(get_user_data(c_ptr()));
  if(!data)
    {
      data = new display_data_t{0,};
      set_user_data(display, data);
    }
  data->counter++;
}

display_t::display_t()
{
  display_t(wl_display_create());
}

display_t::~display_t()
{
  if(display)
    {
      display_data_t *data = reinterpret_cast<display_data_t*>(get_user_data(c_ptr()));
      data->counter--;
      if(data->counter == 0)
        {
          wl_display_destroy(display);
          delete data;
        }
    }
}

display_t::display_t(display_t &&d)
{
  operator=(std::move(d));
}

display_t::display_t(const display_t &d)
{
  operator=(d);
}

display_t &display_t::operator=(display_t &&d)
{
  std::swap(display, d.display);
  return *this;
}

display_t &display_t::operator=(const display_t &d)
{
  display = d.display;
  if(!display)
    {
      display_data_t *data = reinterpret_cast<display_data_t*>(get_user_data(c_ptr()));
      if(!data)
        {
          std::cerr << "Found display_t without meta data." << std::endl;
          data = new display_data_t{0,};
          set_user_data(display, data);
        }
      data->counter++;
    }
  return *this;
}

wl_display *display_t::c_ptr()
{
  if(!display)
    throw std::runtime_error("display is NULL.");
  return display;
}

client_t display_t::create(int fd)
{
  return client_t(wl_client_create(c_ptr(), fd));
}

uint32_t display_t::get_serial()
{
  return wl_display_get_serial(c_ptr());
}

uint32_t display_t::next_serial()
{
  return wl_display_next_serial(c_ptr());
}

void display_t::add_socket(std::string name)
{
  if(wl_display_add_socket(c_ptr(), name == "" ? NULL : name.c_str()) < 0)
    throw std::runtime_error("wl_display_add_socket failed.");
}

// event loop

event_loop_t::event_loop_t(wl_event_loop *d)
  : event_loop(d)
{
  if(!event_loop)
    throw std::runtime_error("event_loop is NULL.");
  data = reinterpret_cast<event_loop_data_t*>(get_user_data(c_ptr()));
  if(!data)
    {
      data = new event_loop_data_t{0,};
      set_user_data(event_loop, data);
    }
  data->counter++;
}

wl_event_loop *event_loop_t::c_ptr()
{
  return event_loop;
}

event_loop_t::event_loop_t()
{
  event_loop_t(wl_event_loop_create());
}

event_loop_t::~event_loop_t()
{
  if(event_loop)
    {
      data->counter--;
      if(data->counter == 0)
        {
          wl_event_loop_destroy(event_loop);
          delete data;
        }
    }
}

event_loop_t::event_loop_t(event_loop_t &&d)
{
  operator=(std::move(d));
}

event_loop_t::event_loop_t(const event_loop_t &d)
{
  operator=(d);
}

event_loop_t &event_loop_t::operator=(event_loop_t &&d)
{
  std::swap(event_loop, d.event_loop);
  std::swap(data, d.data);
  return *this;
}

event_loop_t &event_loop_t::operator=(const event_loop_t &d)
{
  event_loop = d.event_loop;
  data = d.data;
  data->counter++;
  return *this;
}

// ressource

resource_t::resource_t(wl_resource *r)
  : resource(r)
{
  c_ptr();
}

wl_resource *resource_t::c_ptr()
{
  if(!resource)
    throw std::runtime_error("resource is NULL.");
  return resource;
}
