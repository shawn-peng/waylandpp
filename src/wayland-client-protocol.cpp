#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <wayland-client-protocol.hpp>


namespace wayland {

using namespace detail;

display_proxy_t::display_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &display_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return display_proxy_t(p); };
}

display_proxy_t::display_proxy_t() {
  interface = &display_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return display_proxy_t(p); };
}

int display_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->error) events->error(args[0].get<proxy_t>(), args[1].get<uint32_t>(), args[2].get<std::string>());
        break;
    case 1:
        if(events->delete_id) events->delete_id(args[0].get<uint32_t>());
        break;
    }
  return 0;
}
callback_proxy_t display_proxy_t::sync() {
    proxy_t p = marshal_constructor(0, &callback_interface, NULL);
    return callback_proxy_t(p);
}

registry_proxy_t display_proxy_t::get_registry() {
    proxy_t p = marshal_constructor(1, &registry_interface, NULL);
    return registry_proxy_t(p);
}

std::function<void(proxy_t, uint32_t, std::string)> &
display_proxy_t::on_error() {
    return std::static_pointer_cast<events_t>(get_events())->error;
}

std::function<void(uint32_t)> &
display_proxy_t::on_delete_id() {
    return std::static_pointer_cast<events_t>(get_events())->delete_id;
}



registry_proxy_t::registry_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &registry_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return registry_proxy_t(p); };
}

registry_proxy_t::registry_proxy_t() {
  interface = &registry_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return registry_proxy_t(p); };
}

int registry_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->global) events->global(args[0].get<uint32_t>(), args[1].get<std::string>(), args[2].get<uint32_t>());
        break;
    case 1:
        if(events->global_remove) events->global_remove(args[0].get<uint32_t>());
        break;
    }
  return 0;
}
proxy_t registry_proxy_t::bind(uint32_t name, proxy_t &interface, uint32_t version) {
    proxy_t p = marshal_constructor(0, interface.get_iface_ptr(), name, std::string(interface.get_iface_ptr()->name), version, NULL);
    interface = interface.copy_constructor(p);
    return interface;
}

std::function<void(uint32_t, std::string, uint32_t)> &
registry_proxy_t::on_global() {
    return std::static_pointer_cast<events_t>(get_events())->global;
}

std::function<void(uint32_t)> &
registry_proxy_t::on_global_remove() {
    return std::static_pointer_cast<events_t>(get_events())->global_remove;
}



callback_proxy_t::callback_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &callback_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return callback_proxy_t(p); };
}

callback_proxy_t::callback_proxy_t() {
  interface = &callback_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return callback_proxy_t(p); };
}

int callback_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->done) events->done(args[0].get<uint32_t>());
        break;
    }
  return 0;
}
std::function<void(uint32_t)> &
callback_proxy_t::on_done() {
    return std::static_pointer_cast<events_t>(get_events())->done;
}



compositor_proxy_t::compositor_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &compositor_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return compositor_proxy_t(p); };
}

compositor_proxy_t::compositor_proxy_t() {
  interface = &compositor_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return compositor_proxy_t(p); };
}

int compositor_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

  return 0;
}
surface_proxy_t compositor_proxy_t::create_surface() {
    proxy_t p = marshal_constructor(0, &surface_interface, NULL);
    return surface_proxy_t(p);
}

region_proxy_t compositor_proxy_t::create_region() {
    proxy_t p = marshal_constructor(1, &region_interface, NULL);
    return region_proxy_t(p);
}



shm_pool_proxy_t::shm_pool_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(1);
    interface = &shm_pool_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return shm_pool_proxy_t(p); };
}

shm_pool_proxy_t::shm_pool_proxy_t() {
  interface = &shm_pool_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return shm_pool_proxy_t(p); };
}

int shm_pool_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

  return 0;
}
buffer_proxy_t shm_pool_proxy_t::create_buffer(int32_t offset, int32_t width, int32_t height, int32_t stride, shm_format format) {
    proxy_t p = marshal_constructor(0, &buffer_interface, NULL, offset, width, height, stride, static_cast<uint32_t>(format));
    return buffer_proxy_t(p);
}

void shm_pool_proxy_t::resize(int32_t size) {
    marshal(2, size);
}



shm_proxy_t::shm_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &shm_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return shm_proxy_t(p); };
}

shm_proxy_t::shm_proxy_t() {
  interface = &shm_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return shm_proxy_t(p); };
}

int shm_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->format) events->format(shm_format(args[0].get<uint32_t>()));
        break;
    }
  return 0;
}
shm_pool_proxy_t shm_proxy_t::create_pool(int fd, int32_t size) {
    proxy_t p = marshal_constructor(0, &shm_pool_interface, NULL, fd, size);
    return shm_pool_proxy_t(p);
}

std::function<void(shm_format)> &
shm_proxy_t::on_format() {
    return std::static_pointer_cast<events_t>(get_events())->format;
}



buffer_proxy_t::buffer_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(0);
    interface = &buffer_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return buffer_proxy_t(p); };
}

buffer_proxy_t::buffer_proxy_t() {
  interface = &buffer_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return buffer_proxy_t(p); };
}

int buffer_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->release) events->release();
        break;
    }
  return 0;
}
std::function<void()> &
buffer_proxy_t::on_release() {
    return std::static_pointer_cast<events_t>(get_events())->release;
}



data_offer_proxy_t::data_offer_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(2);
    interface = &data_offer_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return data_offer_proxy_t(p); };
}

data_offer_proxy_t::data_offer_proxy_t() {
  interface = &data_offer_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return data_offer_proxy_t(p); };
}

int data_offer_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->offer) events->offer(args[0].get<std::string>());
        break;
    case 1:
        if(events->source_actions) events->source_actions(data_device_manager_dnd_action(args[0].get<uint32_t>()));
        break;
    case 2:
        if(events->action) events->action(data_device_manager_dnd_action(args[0].get<uint32_t>()));
        break;
    }
  return 0;
}
void data_offer_proxy_t::accept(uint32_t serial, std::string mime_type) {
    marshal(0, serial, mime_type);
}

void data_offer_proxy_t::receive(std::string mime_type, int fd) {
    marshal(1, mime_type, fd);
}

void data_offer_proxy_t::finish() {
    marshal(3);
}

void data_offer_proxy_t::set_actions(data_device_manager_dnd_action dnd_actions, data_device_manager_dnd_action preferred_action) {
    marshal(4, static_cast<uint32_t>(dnd_actions), static_cast<uint32_t>(preferred_action));
}

std::function<void(std::string)> &
data_offer_proxy_t::on_offer() {
    return std::static_pointer_cast<events_t>(get_events())->offer;
}

std::function<void(data_device_manager_dnd_action)> &
data_offer_proxy_t::on_source_actions() {
    return std::static_pointer_cast<events_t>(get_events())->source_actions;
}

std::function<void(data_device_manager_dnd_action)> &
data_offer_proxy_t::on_action() {
    return std::static_pointer_cast<events_t>(get_events())->action;
}



data_source_proxy_t::data_source_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(1);
    interface = &data_source_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return data_source_proxy_t(p); };
}

data_source_proxy_t::data_source_proxy_t() {
  interface = &data_source_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return data_source_proxy_t(p); };
}

int data_source_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->target) events->target(args[0].get<std::string>());
        break;
    case 1:
        if(events->send) events->send(args[0].get<std::string>(), args[1].get<int>());
        break;
    case 2:
        if(events->cancelled) events->cancelled();
        break;
    case 3:
        if(events->dnd_drop_performed) events->dnd_drop_performed();
        break;
    case 4:
        if(events->dnd_finished) events->dnd_finished();
        break;
    case 5:
        if(events->action) events->action(data_device_manager_dnd_action(args[0].get<uint32_t>()));
        break;
    }
  return 0;
}
void data_source_proxy_t::offer(std::string mime_type) {
    marshal(0, mime_type);
}

void data_source_proxy_t::set_actions(data_device_manager_dnd_action dnd_actions) {
    marshal(2, static_cast<uint32_t>(dnd_actions));
}

std::function<void(std::string)> &
data_source_proxy_t::on_target() {
    return std::static_pointer_cast<events_t>(get_events())->target;
}

std::function<void(std::string, int)> &
data_source_proxy_t::on_send() {
    return std::static_pointer_cast<events_t>(get_events())->send;
}

std::function<void()> &
data_source_proxy_t::on_cancelled() {
    return std::static_pointer_cast<events_t>(get_events())->cancelled;
}

std::function<void()> &
data_source_proxy_t::on_dnd_drop_performed() {
    return std::static_pointer_cast<events_t>(get_events())->dnd_drop_performed;
}

std::function<void()> &
data_source_proxy_t::on_dnd_finished() {
    return std::static_pointer_cast<events_t>(get_events())->dnd_finished;
}

std::function<void(data_device_manager_dnd_action)> &
data_source_proxy_t::on_action() {
    return std::static_pointer_cast<events_t>(get_events())->action;
}



data_device_proxy_t::data_device_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &data_device_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return data_device_proxy_t(p); };
}

data_device_proxy_t::data_device_proxy_t() {
  interface = &data_device_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return data_device_proxy_t(p); };
}

int data_device_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->data_offer) events->data_offer(data_offer_proxy_t(args[0].get<proxy_t>()));
        break;
    case 1:
        if(events->enter) events->enter(args[0].get<uint32_t>(), surface_proxy_t(args[1].get<proxy_t>()), args[2].get<int32_t>(), args[3].get<int32_t>(), data_offer_proxy_t(args[4].get<proxy_t>()));
        break;
    case 2:
        if(events->leave) events->leave();
        break;
    case 3:
        if(events->motion) events->motion(args[0].get<uint32_t>(), args[1].get<int32_t>(), args[2].get<int32_t>());
        break;
    case 4:
        if(events->drop) events->drop();
        break;
    case 5:
        if(events->selection) events->selection(data_offer_proxy_t(args[0].get<proxy_t>()));
        break;
    }
  return 0;
}
void data_device_proxy_t::start_drag(data_source_proxy_t source, surface_proxy_t origin, surface_proxy_t icon, uint32_t serial) {
    marshal(0, &source, &origin, &icon, serial);
}

void data_device_proxy_t::set_selection(data_source_proxy_t source, uint32_t serial) {
    marshal(1, &source, serial);
}

void data_device_proxy_t::release() {
    marshal(2);
}

std::function<void(data_offer_proxy_t)> &
data_device_proxy_t::on_data_offer() {
    return std::static_pointer_cast<events_t>(get_events())->data_offer;
}

std::function<void(uint32_t, surface_proxy_t, int32_t, int32_t, data_offer_proxy_t)> &
data_device_proxy_t::on_enter() {
    return std::static_pointer_cast<events_t>(get_events())->enter;
}

std::function<void()> &
data_device_proxy_t::on_leave() {
    return std::static_pointer_cast<events_t>(get_events())->leave;
}

std::function<void(uint32_t, int32_t, int32_t)> &
data_device_proxy_t::on_motion() {
    return std::static_pointer_cast<events_t>(get_events())->motion;
}

std::function<void()> &
data_device_proxy_t::on_drop() {
    return std::static_pointer_cast<events_t>(get_events())->drop;
}

std::function<void(data_offer_proxy_t)> &
data_device_proxy_t::on_selection() {
    return std::static_pointer_cast<events_t>(get_events())->selection;
}



data_device_manager_proxy_t::data_device_manager_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &data_device_manager_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return data_device_manager_proxy_t(p); };
}

data_device_manager_proxy_t::data_device_manager_proxy_t() {
  interface = &data_device_manager_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return data_device_manager_proxy_t(p); };
}

int data_device_manager_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

  return 0;
}
data_source_proxy_t data_device_manager_proxy_t::create_data_source() {
    proxy_t p = marshal_constructor(0, &data_source_interface, NULL);
    return data_source_proxy_t(p);
}

data_device_proxy_t data_device_manager_proxy_t::get_data_device(seat_proxy_t seat) {
    proxy_t p = marshal_constructor(1, &data_device_interface, NULL, &seat);
    return data_device_proxy_t(p);
}



shell_proxy_t::shell_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &shell_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return shell_proxy_t(p); };
}

shell_proxy_t::shell_proxy_t() {
  interface = &shell_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return shell_proxy_t(p); };
}

int shell_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

  return 0;
}
shell_surface_proxy_t shell_proxy_t::get_shell_surface(surface_proxy_t surface) {
    proxy_t p = marshal_constructor(0, &shell_surface_interface, NULL, &surface);
    return shell_surface_proxy_t(p);
}



shell_surface_proxy_t::shell_surface_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &shell_surface_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return shell_surface_proxy_t(p); };
}

shell_surface_proxy_t::shell_surface_proxy_t() {
  interface = &shell_surface_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return shell_surface_proxy_t(p); };
}

int shell_surface_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->ping) events->ping(args[0].get<uint32_t>());
        break;
    case 1:
        if(events->configure) events->configure(shell_surface_resize(args[0].get<uint32_t>()), args[1].get<int32_t>(), args[2].get<int32_t>());
        break;
    case 2:
        if(events->popup_done) events->popup_done();
        break;
    }
  return 0;
}
void shell_surface_proxy_t::pong(uint32_t serial) {
    marshal(0, serial);
}

void shell_surface_proxy_t::move(seat_proxy_t seat, uint32_t serial) {
    marshal(1, &seat, serial);
}

void shell_surface_proxy_t::resize(seat_proxy_t seat, uint32_t serial, shell_surface_resize edges) {
    marshal(2, &seat, serial, static_cast<uint32_t>(edges));
}

void shell_surface_proxy_t::set_toplevel() {
    marshal(3);
}

void shell_surface_proxy_t::set_transient(surface_proxy_t parent, int32_t x, int32_t y, shell_surface_transient flags) {
    marshal(4, &parent, x, y, static_cast<uint32_t>(flags));
}

void shell_surface_proxy_t::set_fullscreen(shell_surface_fullscreen_method method, uint32_t framerate, output_proxy_t output) {
    marshal(5, static_cast<uint32_t>(method), framerate, &output);
}

void shell_surface_proxy_t::set_popup(seat_proxy_t seat, uint32_t serial, surface_proxy_t parent, int32_t x, int32_t y, shell_surface_transient flags) {
    marshal(6, &seat, serial, &parent, x, y, static_cast<uint32_t>(flags));
}

void shell_surface_proxy_t::set_maximized(output_proxy_t output) {
    marshal(7, &output);
}

void shell_surface_proxy_t::set_title(std::string title) {
    marshal(8, title);
}

void shell_surface_proxy_t::set_class(std::string class_) {
    marshal(9, class_);
}

std::function<void(uint32_t)> &
shell_surface_proxy_t::on_ping() {
    return std::static_pointer_cast<events_t>(get_events())->ping;
}

std::function<void(shell_surface_resize, int32_t, int32_t)> &
shell_surface_proxy_t::on_configure() {
    return std::static_pointer_cast<events_t>(get_events())->configure;
}

std::function<void()> &
shell_surface_proxy_t::on_popup_done() {
    return std::static_pointer_cast<events_t>(get_events())->popup_done;
}



surface_proxy_t::surface_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(0);
    interface = &surface_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return surface_proxy_t(p); };
}

surface_proxy_t::surface_proxy_t() {
  interface = &surface_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return surface_proxy_t(p); };
}

int surface_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->enter) events->enter(output_proxy_t(args[0].get<proxy_t>()));
        break;
    case 1:
        if(events->leave) events->leave(output_proxy_t(args[0].get<proxy_t>()));
        break;
    }
  return 0;
}
void surface_proxy_t::attach(buffer_proxy_t buffer, int32_t x, int32_t y) {
    marshal(1, &buffer, x, y);
}

void surface_proxy_t::damage(int32_t x, int32_t y, int32_t width, int32_t height) {
    marshal(2, x, y, width, height);
}

callback_proxy_t surface_proxy_t::frame() {
    proxy_t p = marshal_constructor(3, &callback_interface, NULL);
    return callback_proxy_t(p);
}

void surface_proxy_t::set_opaque_region(region_proxy_t region) {
    marshal(4, &region);
}

void surface_proxy_t::set_input_region(region_proxy_t region) {
    marshal(5, &region);
}

void surface_proxy_t::commit() {
    marshal(6);
}

void surface_proxy_t::set_buffer_transform(output_transform transform) {
    marshal(7, static_cast<uint32_t>(transform));
}

void surface_proxy_t::set_buffer_scale(int32_t scale) {
    marshal(8, scale);
}

void surface_proxy_t::damage_buffer(int32_t x, int32_t y, int32_t width, int32_t height) {
    marshal(9, x, y, width, height);
}

std::function<void(output_proxy_t)> &
surface_proxy_t::on_enter() {
    return std::static_pointer_cast<events_t>(get_events())->enter;
}

std::function<void(output_proxy_t)> &
surface_proxy_t::on_leave() {
    return std::static_pointer_cast<events_t>(get_events())->leave;
}



seat_proxy_t::seat_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &seat_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return seat_proxy_t(p); };
}

seat_proxy_t::seat_proxy_t() {
  interface = &seat_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return seat_proxy_t(p); };
}

int seat_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->capabilities) events->capabilities(seat_capability(args[0].get<uint32_t>()));
        break;
    case 1:
        if(events->name) events->name(args[0].get<std::string>());
        break;
    }
  return 0;
}
pointer_proxy_t seat_proxy_t::get_pointer() {
    proxy_t p = marshal_constructor(0, &pointer_interface, NULL);
    return pointer_proxy_t(p);
}

keyboard_proxy_t seat_proxy_t::get_keyboard() {
    proxy_t p = marshal_constructor(1, &keyboard_interface, NULL);
    return keyboard_proxy_t(p);
}

touch_proxy_t seat_proxy_t::get_touch() {
    proxy_t p = marshal_constructor(2, &touch_interface, NULL);
    return touch_proxy_t(p);
}

void seat_proxy_t::release() {
    marshal(3);
}

std::function<void(seat_capability)> &
seat_proxy_t::on_capabilities() {
    return std::static_pointer_cast<events_t>(get_events())->capabilities;
}

std::function<void(std::string)> &
seat_proxy_t::on_name() {
    return std::static_pointer_cast<events_t>(get_events())->name;
}



pointer_proxy_t::pointer_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &pointer_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return pointer_proxy_t(p); };
}

pointer_proxy_t::pointer_proxy_t() {
  interface = &pointer_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return pointer_proxy_t(p); };
}

int pointer_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->enter) events->enter(args[0].get<uint32_t>(), surface_proxy_t(args[1].get<proxy_t>()), args[2].get<int32_t>(), args[3].get<int32_t>());
        break;
    case 1:
        if(events->leave) events->leave(args[0].get<uint32_t>(), surface_proxy_t(args[1].get<proxy_t>()));
        break;
    case 2:
        if(events->motion) events->motion(args[0].get<uint32_t>(), args[1].get<int32_t>(), args[2].get<int32_t>());
        break;
    case 3:
        if(events->button) events->button(args[0].get<uint32_t>(), args[1].get<uint32_t>(), args[2].get<uint32_t>(), pointer_button_state(args[3].get<uint32_t>()));
        break;
    case 4:
        if(events->axis) events->axis(args[0].get<uint32_t>(), pointer_axis(args[1].get<uint32_t>()), args[2].get<int32_t>());
        break;
    case 5:
        if(events->frame) events->frame();
        break;
    case 6:
        if(events->axis_source) events->axis_source(pointer_axis_source(args[0].get<uint32_t>()));
        break;
    case 7:
        if(events->axis_stop) events->axis_stop(args[0].get<uint32_t>(), pointer_axis(args[1].get<uint32_t>()));
        break;
    case 8:
        if(events->axis_discrete) events->axis_discrete(pointer_axis(args[0].get<uint32_t>()), args[1].get<int32_t>());
        break;
    }
  return 0;
}
void pointer_proxy_t::set_cursor(uint32_t serial, surface_proxy_t surface, int32_t hotspot_x, int32_t hotspot_y) {
    marshal(0, serial, &surface, hotspot_x, hotspot_y);
}

void pointer_proxy_t::release() {
    marshal(1);
}

std::function<void(uint32_t, surface_proxy_t, int32_t, int32_t)> &
pointer_proxy_t::on_enter() {
    return std::static_pointer_cast<events_t>(get_events())->enter;
}

std::function<void(uint32_t, surface_proxy_t)> &
pointer_proxy_t::on_leave() {
    return std::static_pointer_cast<events_t>(get_events())->leave;
}

std::function<void(uint32_t, int32_t, int32_t)> &
pointer_proxy_t::on_motion() {
    return std::static_pointer_cast<events_t>(get_events())->motion;
}

std::function<void(uint32_t, uint32_t, uint32_t, pointer_button_state)> &
pointer_proxy_t::on_button() {
    return std::static_pointer_cast<events_t>(get_events())->button;
}

std::function<void(uint32_t, pointer_axis, int32_t)> &
pointer_proxy_t::on_axis() {
    return std::static_pointer_cast<events_t>(get_events())->axis;
}

std::function<void()> &
pointer_proxy_t::on_frame() {
    return std::static_pointer_cast<events_t>(get_events())->frame;
}

std::function<void(pointer_axis_source)> &
pointer_proxy_t::on_axis_source() {
    return std::static_pointer_cast<events_t>(get_events())->axis_source;
}

std::function<void(uint32_t, pointer_axis)> &
pointer_proxy_t::on_axis_stop() {
    return std::static_pointer_cast<events_t>(get_events())->axis_stop;
}

std::function<void(pointer_axis, int32_t)> &
pointer_proxy_t::on_axis_discrete() {
    return std::static_pointer_cast<events_t>(get_events())->axis_discrete;
}



keyboard_proxy_t::keyboard_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &keyboard_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return keyboard_proxy_t(p); };
}

keyboard_proxy_t::keyboard_proxy_t() {
  interface = &keyboard_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return keyboard_proxy_t(p); };
}

int keyboard_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->keymap) events->keymap(keyboard_keymap_format(args[0].get<uint32_t>()), args[1].get<int>(), args[2].get<uint32_t>());
        break;
    case 1:
        if(events->enter) events->enter(args[0].get<uint32_t>(), surface_proxy_t(args[1].get<proxy_t>()), args[2].get<array_t>());
        break;
    case 2:
        if(events->leave) events->leave(args[0].get<uint32_t>(), surface_proxy_t(args[1].get<proxy_t>()));
        break;
    case 3:
        if(events->key) events->key(args[0].get<uint32_t>(), args[1].get<uint32_t>(), args[2].get<uint32_t>(), keyboard_key_state(args[3].get<uint32_t>()));
        break;
    case 4:
        if(events->modifiers) events->modifiers(args[0].get<uint32_t>(), args[1].get<uint32_t>(), args[2].get<uint32_t>(), args[3].get<uint32_t>(), args[4].get<uint32_t>());
        break;
    case 5:
        if(events->repeat_info) events->repeat_info(args[0].get<int32_t>(), args[1].get<int32_t>());
        break;
    }
  return 0;
}
void keyboard_proxy_t::release() {
    marshal(0);
}

std::function<void(keyboard_keymap_format, int, uint32_t)> &
keyboard_proxy_t::on_keymap() {
    return std::static_pointer_cast<events_t>(get_events())->keymap;
}

std::function<void(uint32_t, surface_proxy_t, array_t)> &
keyboard_proxy_t::on_enter() {
    return std::static_pointer_cast<events_t>(get_events())->enter;
}

std::function<void(uint32_t, surface_proxy_t)> &
keyboard_proxy_t::on_leave() {
    return std::static_pointer_cast<events_t>(get_events())->leave;
}

std::function<void(uint32_t, uint32_t, uint32_t, keyboard_key_state)> &
keyboard_proxy_t::on_key() {
    return std::static_pointer_cast<events_t>(get_events())->key;
}

std::function<void(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)> &
keyboard_proxy_t::on_modifiers() {
    return std::static_pointer_cast<events_t>(get_events())->modifiers;
}

std::function<void(int32_t, int32_t)> &
keyboard_proxy_t::on_repeat_info() {
    return std::static_pointer_cast<events_t>(get_events())->repeat_info;
}



touch_proxy_t::touch_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &touch_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return touch_proxy_t(p); };
}

touch_proxy_t::touch_proxy_t() {
  interface = &touch_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return touch_proxy_t(p); };
}

int touch_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->down) events->down(args[0].get<uint32_t>(), args[1].get<uint32_t>(), surface_proxy_t(args[2].get<proxy_t>()), args[3].get<int32_t>(), args[4].get<int32_t>(), args[5].get<int32_t>());
        break;
    case 1:
        if(events->up) events->up(args[0].get<uint32_t>(), args[1].get<uint32_t>(), args[2].get<int32_t>());
        break;
    case 2:
        if(events->motion) events->motion(args[0].get<uint32_t>(), args[1].get<int32_t>(), args[2].get<int32_t>(), args[3].get<int32_t>());
        break;
    case 3:
        if(events->frame) events->frame();
        break;
    case 4:
        if(events->cancel) events->cancel();
        break;
    }
  return 0;
}
void touch_proxy_t::release() {
    marshal(0);
}

std::function<void(uint32_t, uint32_t, surface_proxy_t, int32_t, int32_t, int32_t)> &
touch_proxy_t::on_down() {
    return std::static_pointer_cast<events_t>(get_events())->down;
}

std::function<void(uint32_t, uint32_t, int32_t)> &
touch_proxy_t::on_up() {
    return std::static_pointer_cast<events_t>(get_events())->up;
}

std::function<void(uint32_t, int32_t, int32_t, int32_t)> &
touch_proxy_t::on_motion() {
    return std::static_pointer_cast<events_t>(get_events())->motion;
}

std::function<void()> &
touch_proxy_t::on_frame() {
    return std::static_pointer_cast<events_t>(get_events())->frame;
}

std::function<void()> &
touch_proxy_t::on_cancel() {
    return std::static_pointer_cast<events_t>(get_events())->cancel;
}



output_proxy_t::output_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(-1);
    interface = &output_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return output_proxy_t(p); };
}

output_proxy_t::output_proxy_t() {
  interface = &output_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return output_proxy_t(p); };
}

int output_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);
    switch(opcode) {
    case 0:
        if(events->geometry) events->geometry(args[0].get<int32_t>(), args[1].get<int32_t>(), args[2].get<int32_t>(), args[3].get<int32_t>(), output_subpixel(args[4].get<uint32_t>()), args[5].get<std::string>(), args[6].get<std::string>(), output_transform(args[7].get<uint32_t>()));
        break;
    case 1:
        if(events->mode) events->mode(output_mode(args[0].get<uint32_t>()), args[1].get<int32_t>(), args[2].get<int32_t>(), args[3].get<int32_t>());
        break;
    case 2:
        if(events->done) events->done();
        break;
    case 3:
        if(events->scale) events->scale(args[0].get<int32_t>());
        break;
    }
  return 0;
}
std::function<void(int32_t, int32_t, int32_t, int32_t, output_subpixel, std::string, std::string, output_transform)> &
output_proxy_t::on_geometry() {
    return std::static_pointer_cast<events_t>(get_events())->geometry;
}

std::function<void(output_mode, int32_t, int32_t, int32_t)> &
output_proxy_t::on_mode() {
    return std::static_pointer_cast<events_t>(get_events())->mode;
}

std::function<void()> &
output_proxy_t::on_done() {
    return std::static_pointer_cast<events_t>(get_events())->done;
}

std::function<void(int32_t)> &
output_proxy_t::on_scale() {
    return std::static_pointer_cast<events_t>(get_events())->scale;
}



region_proxy_t::region_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(0);
    interface = &region_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return region_proxy_t(p); };
}

region_proxy_t::region_proxy_t() {
  interface = &region_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return region_proxy_t(p); };
}

int region_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

  return 0;
}
void region_proxy_t::add(int32_t x, int32_t y, int32_t width, int32_t height) {
    marshal(1, x, y, width, height);
}

void region_proxy_t::subtract(int32_t x, int32_t y, int32_t width, int32_t height) {
    marshal(2, x, y, width, height);
}



subcompositor_proxy_t::subcompositor_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(0);
    interface = &subcompositor_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return subcompositor_proxy_t(p); };
}

subcompositor_proxy_t::subcompositor_proxy_t() {
  interface = &subcompositor_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return subcompositor_proxy_t(p); };
}

int subcompositor_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

  return 0;
}
subsurface_proxy_t subcompositor_proxy_t::get_subsurface(surface_proxy_t surface, surface_proxy_t parent) {
    proxy_t p = marshal_constructor(1, &subsurface_interface, NULL, &surface, &parent);
    return subsurface_proxy_t(p);
}



subsurface_proxy_t::subsurface_proxy_t(const proxy_t &p)
  : proxy_t(p) {
    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);
    set_destroy_opcode(0);
    interface = &subsurface_interface;
    copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return subsurface_proxy_t(p); };
}

subsurface_proxy_t::subsurface_proxy_t() {
  interface = &subsurface_interface;
  copy_constructor = [] (const proxy_t &p) -> proxy_t
    { return subsurface_proxy_t(p); };
}

int subsurface_proxy_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {

  return 0;
}
void subsurface_proxy_t::set_position(int32_t x, int32_t y) {
    marshal(1, x, y);
}

void subsurface_proxy_t::place_above(surface_proxy_t sibling) {
    marshal(2, &sibling);
}

void subsurface_proxy_t::place_below(surface_proxy_t sibling) {
    marshal(3, &sibling);
}

void subsurface_proxy_t::set_sync() {
    marshal(4);
}

void subsurface_proxy_t::set_desync() {
    marshal(5);
}




}

