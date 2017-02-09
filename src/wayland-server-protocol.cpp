#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <wayland-server-protocol.hpp>


namespace wayland {

using namespace detail;

    /** \brief asynchronous roundtrip
        \param callback 

	The sync request asks the server to emit the 'done' event
	on the returned wl_callback object.  Since requests are
	handled in-order and events are delivered in-order, this can
	be used as a barrier to ensure all previous requests and the
	resulting events have been handled.

	The object returned by this request will be destroyed by the
	compositor after the callback is fired and as such the client must not
	attempt to use it after that point.

	The callback_data passed in the callback is the event serial.
      
     */
    std::function<void(callback_proxy_t)> &display_resource_t::on_sync() {
        return std::static_pointer_cast<requests_t>(get_requests())->sync;
    }

    /** \brief get global registry object
        \param registry 

	This request creates a registry object that allows the client
	to list and bind the global objects available from the
	compositor.
      
     */
    std::function<void(registry_proxy_t)> &display_resource_t::on_get_registry() {
        return std::static_pointer_cast<requests_t>(get_requests())->get_registry;
    }



    /** \brief bind an object to the display
        \param name 
        \param id 

	Binds a new, client-created object to the server using the
        specified name as the identifier.
      
     */
    std::function<void(uint32_t, proxy_t)> &registry_resource_t::on_bind() {
        return std::static_pointer_cast<requests_t>(get_requests())->bind;
    }





    /** \brief create new surface
        \param id 

	Ask the compositor to create a new surface.
      
     */
    std::function<void(surface_proxy_t)> &compositor_resource_t::on_create_surface() {
        return std::static_pointer_cast<requests_t>(get_requests())->create_surface;
    }

    /** \brief create new region
        \param id 

	Ask the compositor to create a new region.
      
     */
    std::function<void(region_proxy_t)> &compositor_resource_t::on_create_region() {
        return std::static_pointer_cast<requests_t>(get_requests())->create_region;
    }



    /** \brief create a buffer from the pool
        \param id 
        \param offset 
        \param width 
        \param height 
        \param stride 
        \param format 

	Create a wl_buffer object from the pool.

	The buffer is created offset bytes into the pool and has
	width and height as specified.  The stride argument specifies
	the number of bytes from the beginning of one row to the beginning
	of the next.  The format is the pixel format of the buffer and
	must be one of those advertised through the wl_shm.format event.

	A buffer will keep a reference to the pool it was created from
	so it is valid to destroy the pool immediately after creating
	a buffer from it.
      
     */
    std::function<void(buffer_proxy_t, int32_t, int32_t, int32_t, int32_t, shm_format)> &shm_pool_resource_t::on_create_buffer() {
        return std::static_pointer_cast<requests_t>(get_requests())->create_buffer;
    }

    /** \brief destroy the pool

	Destroy the shared memory pool.

	The mmapped memory will be released when all
	buffers that have been created from this pool
	are gone.
      
     */
    std::function<void()> &shm_pool_resource_t::on_destroy() {
        return std::static_pointer_cast<requests_t>(get_requests())->destroy;
    }

    /** \brief change the size of the pool mapping
        \param size 

	This request will cause the server to remap the backing memory
	for the pool from the file descriptor passed when the pool was
	created, but using the new size.  This request can only be
	used to make the pool bigger.
      
     */
    std::function<void(int32_t)> &shm_pool_resource_t::on_resize() {
        return std::static_pointer_cast<requests_t>(get_requests())->resize;
    }



    /** \brief create a shm pool
        \param id 
        \param fd 
        \param size 

	Create a new wl_shm_pool object.

	The pool can be used to create shared memory based buffer
	objects.  The server will mmap size bytes of the passed file
        descriptor, to use as backing memory for the pool.
      
     */
    std::function<void(shm_pool_proxy_t, int, int32_t)> &shm_resource_t::on_create_pool() {
        return std::static_pointer_cast<requests_t>(get_requests())->create_pool;
    }



    /** \brief destroy a buffer

	Destroy a buffer. If and how you need to release the backing
	storage is defined by the buffer factory interface.

	For possible side-effects to a surface, see wl_surface.attach.
      
     */
    std::function<void()> &buffer_resource_t::on_destroy() {
        return std::static_pointer_cast<requests_t>(get_requests())->destroy;
    }



    /** \brief accept one of the offered mime types
        \param serial 
        \param mime_type 

	Indicate that the client can accept the given mime type, or
	NULL for not accepted.

	For objects of version 2 or older, this request is used by the
	client to give feedback whether the client can receive the given
	mime type, or NULL if none is accepted; the feedback does not
	determine whether the drag-and-drop operation succeeds or not.

	For objects of version 3 or newer, this request determines the
	final result of the drag-and-drop operation. If the end result
	is that no mime types were accepted, the drag-and-drop operation
	will be cancelled and the corresponding drag source will receive
	wl_data_source.cancelled. Clients may still use this event in
	conjunction with wl_data_source.action for feedback.
      
     */
    std::function<void(uint32_t, std::string)> &data_offer_resource_t::on_accept() {
        return std::static_pointer_cast<requests_t>(get_requests())->accept;
    }

    /** \brief request that the data is transferred
        \param mime_type 
        \param fd 

	To transfer the offered data, the client issues this request
	and indicates the mime type it wants to receive.  The transfer
	happens through the passed file descriptor (typically created
	with the pipe system call).  The source client writes the data
	in the mime type representation requested and then closes the
	file descriptor.

	The receiving client reads from the read end of the pipe until
	EOF and then closes its end, at which point the transfer is
	complete.

	This request may happen multiple times for different mime types,
	both before and after wl_data_device.drop. Drag-and-drop destination
	clients may preemptively fetch data or examine it more closely to
	determine acceptance.
      
     */
    std::function<void(std::string, int)> &data_offer_resource_t::on_receive() {
        return std::static_pointer_cast<requests_t>(get_requests())->receive;
    }

    /** \brief destroy data offer

	Destroy the data offer.
      
     */
    std::function<void()> &data_offer_resource_t::on_destroy() {
        return std::static_pointer_cast<requests_t>(get_requests())->destroy;
    }

    /** \brief the offer will no longer be used

	Notifies the compositor that the drag destination successfully
	finished the drag-and-drop operation.

	Upon receiving this request, the compositor will emit
	wl_data_source.dnd_finished on the drag source client.

	It is a client error to perform other requests than
	wl_data_offer.destroy after this one. It is also an error to perform
	this request after a NULL mime type has been set in
	wl_data_offer.accept or no action was received through
	wl_data_offer.action.
      
     */
    std::function<void()> &data_offer_resource_t::on_finish() {
        return std::static_pointer_cast<requests_t>(get_requests())->finish;
    }

    /** \brief set the available/preferred drag-and-drop actions
        \param dnd_actions 
        \param preferred_action 

	Sets the actions that the destination side client supports for
	this operation. This request may trigger the emission of
	wl_data_source.action and wl_data_offer.action events if the compositor
	needs to change the selected action.

	This request can be called multiple times throughout the
	drag-and-drop operation, typically in response to wl_data_device.enter
	or wl_data_device.motion events.

	This request determines the final result of the drag-and-drop
	operation. If the end result is that no action is accepted,
	the drag source will receive wl_drag_source.cancelled.

	The dnd_actions argument must contain only values expressed in the
	wl_data_device_manager.dnd_actions enum, and the preferred_action
	argument must only contain one of those values set, otherwise it
	will result in a protocol error.

	While managing an "ask" action, the destination drag-and-drop client
	may perform further wl_data_offer.receive requests, and is expected
	to perform one last wl_data_offer.set_actions request with a preferred
	action other than "ask" (and optionally wl_data_offer.accept) before
	requesting wl_data_offer.finish, in order to convey the action selected
	by the user. If the preferred action is not in the
	wl_data_offer.source_actions mask, an error will be raised.

	If the "ask" action is dismissed (e.g. user cancellation), the client
	is expected to perform wl_data_offer.destroy right away.

	This request can only be made on drag-and-drop offers, a protocol error
	will be raised otherwise.
      
     */
    std::function<void(data_device_manager_dnd_action, data_device_manager_dnd_action)> &data_offer_resource_t::on_set_actions() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_actions;
    }



    /** \brief add an offered mime type
        \param mime_type 

	This request adds a mime type to the set of mime types
	advertised to targets.  Can be called several times to offer
	multiple types.
      
     */
    std::function<void(std::string)> &data_source_resource_t::on_offer() {
        return std::static_pointer_cast<requests_t>(get_requests())->offer;
    }

    /** \brief destroy the data source

	Destroy the data source.
      
     */
    std::function<void()> &data_source_resource_t::on_destroy() {
        return std::static_pointer_cast<requests_t>(get_requests())->destroy;
    }

    /** \brief set the available drag-and-drop actions
        \param dnd_actions 

	Sets the actions that the source side client supports for this
	operation. This request may trigger wl_data_source.action and
	wl_data_offer.action events if the compositor needs to change the
	selected action.

	The dnd_actions argument must contain only values expressed in the
	wl_data_device_manager.dnd_actions enum, otherwise it will result
	in a protocol error.

	This request must be made once only, and can only be made on sources
	used in drag-and-drop, so it must be performed before
	wl_data_device.start_drag. Attempting to use the source other than
	for drag-and-drop will raise a protocol error.
      
     */
    std::function<void(data_device_manager_dnd_action)> &data_source_resource_t::on_set_actions() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_actions;
    }



    /** \brief start drag-and-drop operation
        \param source 
        \param origin 
        \param icon 
        \param serial 

	This request asks the compositor to start a drag-and-drop
	operation on behalf of the client.

	The source argument is the data source that provides the data
	for the eventual data transfer. If source is NULL, enter, leave
	and motion events are sent only to the client that initiated the
	drag and the client is expected to handle the data passing
	internally.

	The origin surface is the surface where the drag originates and
	the client must have an active implicit grab that matches the
	serial.

	The icon surface is an optional (can be NULL) surface that
	provides an icon to be moved around with the cursor.  Initially,
	the top-left corner of the icon surface is placed at the cursor
	hotspot, but subsequent wl_surface.attach request can move the
	relative position. Attach requests must be confirmed with
	wl_surface.commit as usual. The icon surface is given the role of
	a drag-and-drop icon. If the icon surface already has another role,
	it raises a protocol error.

	The current and pending input regions of the icon wl_surface are
	cleared, and wl_surface.set_input_region is ignored until the
	wl_surface is no longer used as the icon surface. When the use
	as an icon ends, the current and pending input regions become
	undefined, and the wl_surface is unmapped.
      
     */
    std::function<void(data_source_proxy_t, surface_proxy_t, surface_proxy_t, uint32_t)> &data_device_resource_t::on_start_drag() {
        return std::static_pointer_cast<requests_t>(get_requests())->start_drag;
    }

    /** \brief copy data to the selection
        \param source 
        \param serial 

	This request asks the compositor to set the selection
	to the data from the source on behalf of the client.

	To unset the selection, set the source to NULL.
      
     */
    std::function<void(data_source_proxy_t, uint32_t)> &data_device_resource_t::on_set_selection() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_selection;
    }

    /** \brief destroy data device

	This request destroys the data device.
      
     */
    std::function<void()> &data_device_resource_t::on_release() {
        return std::static_pointer_cast<requests_t>(get_requests())->release;
    }



    /** \brief create a new data source
        \param id 

        Create a new data source.
      
     */
    std::function<void(data_source_proxy_t)> &data_device_manager_resource_t::on_create_data_source() {
        return std::static_pointer_cast<requests_t>(get_requests())->create_data_source;
    }

    /** \brief create a new data device
        \param id 
        \param seat 

        Create a new data device for a given seat.
      
     */
    std::function<void(data_device_proxy_t, seat_proxy_t)> &data_device_manager_resource_t::on_get_data_device() {
        return std::static_pointer_cast<requests_t>(get_requests())->get_data_device;
    }



    /** \brief create a shell surface from a surface
        \param id 
        \param surface 

	Create a shell surface for an existing surface. This gives
	the wl_surface the role of a shell surface. If the wl_surface
	already has another role, it raises a protocol error.

	Only one shell surface can be associated with a given surface.
      
     */
    std::function<void(shell_surface_proxy_t, surface_proxy_t)> &shell_resource_t::on_get_shell_surface() {
        return std::static_pointer_cast<requests_t>(get_requests())->get_shell_surface;
    }



    /** \brief respond to a ping event
        \param serial 

	A client must respond to a ping event with a pong request or
	the client may be deemed unresponsive.
      
     */
    std::function<void(uint32_t)> &shell_surface_resource_t::on_pong() {
        return std::static_pointer_cast<requests_t>(get_requests())->pong;
    }

    /** \brief start an interactive move
        \param seat 
        \param serial 

	Start a pointer-driven move of the surface.

	This request must be used in response to a button press event.
	The server may ignore move requests depending on the state of
	the surface (e.g. fullscreen or maximized).
      
     */
    std::function<void(seat_proxy_t, uint32_t)> &shell_surface_resource_t::on_move() {
        return std::static_pointer_cast<requests_t>(get_requests())->move;
    }

    /** \brief start an interactive resize
        \param seat 
        \param serial 
        \param edges 

	Start a pointer-driven resizing of the surface.

	This request must be used in response to a button press event.
	The server may ignore resize requests depending on the state of
	the surface (e.g. fullscreen or maximized).
      
     */
    std::function<void(seat_proxy_t, uint32_t, shell_surface_resize)> &shell_surface_resource_t::on_resize() {
        return std::static_pointer_cast<requests_t>(get_requests())->resize;
    }

    /** \brief make the surface a toplevel surface

	Map the surface as a toplevel surface.

	A toplevel surface is not fullscreen, maximized or transient.
      
     */
    std::function<void()> &shell_surface_resource_t::on_set_toplevel() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_toplevel;
    }

    /** \brief make the surface a transient surface
        \param parent 
        \param x 
        \param y 
        \param flags 

	Map the surface relative to an existing surface.

	The x and y arguments specify the location of the upper left
	corner of the surface relative to the upper left corner of the
	parent surface, in surface-local coordinates.

	The flags argument controls details of the transient behaviour.
      
     */
    std::function<void(surface_proxy_t, int32_t, int32_t, shell_surface_transient)> &shell_surface_resource_t::on_set_transient() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_transient;
    }

    /** \brief make the surface a fullscreen surface
        \param method 
        \param framerate 
        \param output 

	Map the surface as a fullscreen surface.

	If an output parameter is given then the surface will be made
	fullscreen on that output. If the client does not specify the
	output then the compositor will apply its policy - usually
	choosing the output on which the surface has the biggest surface
	area.

	The client may specify a method to resolve a size conflict
	between the output size and the surface size - this is provided
	through the method parameter.

	The framerate parameter is used only when the method is set
	to "driver", to indicate the preferred framerate. A value of 0
	indicates that the client does not care about framerate.  The
	framerate is specified in mHz, that is framerate of 60000 is 60Hz.

	A method of "scale" or "driver" implies a scaling operation of
	the surface, either via a direct scaling operation or a change of
	the output mode. This will override any kind of output scaling, so
	that mapping a surface with a buffer size equal to the mode can
	fill the screen independent of buffer_scale.

	A method of "fill" means we don't scale up the buffer, however
	any output scale is applied. This means that you may run into
	an edge case where the application maps a buffer with the same
	size of the output mode but buffer_scale 1 (thus making a
	surface larger than the output). In this case it is allowed to
	downscale the results to fit the screen.

	The compositor must reply to this request with a configure event
	with the dimensions for the output on which the surface will
	be made fullscreen.
      
     */
    std::function<void(shell_surface_fullscreen_method, uint32_t, output_proxy_t)> &shell_surface_resource_t::on_set_fullscreen() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_fullscreen;
    }

    /** \brief make the surface a popup surface
        \param seat 
        \param serial 
        \param parent 
        \param x 
        \param y 
        \param flags 

	Map the surface as a popup.

	A popup surface is a transient surface with an added pointer
	grab.

	An existing implicit grab will be changed to owner-events mode,
	and the popup grab will continue after the implicit grab ends
	(i.e. releasing the mouse button does not cause the popup to
	be unmapped).

	The popup grab continues until the window is destroyed or a
	mouse button is pressed in any other client's window. A click
	in any of the client's surfaces is reported as normal, however,
	clicks in other clients' surfaces will be discarded and trigger
	the callback.

	The x and y arguments specify the location of the upper left
	corner of the surface relative to the upper left corner of the
	parent surface, in surface-local coordinates.
      
     */
    std::function<void(seat_proxy_t, uint32_t, surface_proxy_t, int32_t, int32_t, shell_surface_transient)> &shell_surface_resource_t::on_set_popup() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_popup;
    }

    /** \brief make the surface a maximized surface
        \param output 

	Map the surface as a maximized surface.

	If an output parameter is given then the surface will be
	maximized on that output. If the client does not specify the
	output then the compositor will apply its policy - usually
	choosing the output on which the surface has the biggest surface
	area.

	The compositor will reply with a configure event telling
	the expected new surface size. The operation is completed
	on the next buffer attach to this surface.

	A maximized surface typically fills the entire output it is
	bound to, except for desktop elements such as panels. This is
	the main difference between a maximized shell surface and a
	fullscreen shell surface.

	The details depend on the compositor implementation.
      
     */
    std::function<void(output_proxy_t)> &shell_surface_resource_t::on_set_maximized() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_maximized;
    }

    /** \brief set surface title
        \param title 

	Set a short title for the surface.

	This string may be used to identify the surface in a task bar,
	window list, or other user interface elements provided by the
	compositor.

	The string must be encoded in UTF-8.
      
     */
    std::function<void(std::string)> &shell_surface_resource_t::on_set_title() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_title;
    }

    /** \brief set surface class
        \param class_ 

	Set a class for the surface.

	The surface class identifies the general class of applications
	to which the surface belongs. A common convention is to use the
	file name (or the full path if it is a non-standard location) of
	the application's .desktop file as the class.
      
     */
    std::function<void(std::string)> &shell_surface_resource_t::on_set_class() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_class;
    }



    /** \brief delete surface

	Deletes the surface and invalidates its object ID.
      
     */
    std::function<void()> &surface_resource_t::on_destroy() {
        return std::static_pointer_cast<requests_t>(get_requests())->destroy;
    }

    /** \brief set the surface contents
        \param buffer 
        \param x 
        \param y 

	Set a buffer as the content of this surface.

	The new size of the surface is calculated based on the buffer
	size transformed by the inverse buffer_transform and the
	inverse buffer_scale. This means that the supplied buffer
	must be an integer multiple of the buffer_scale.

	The x and y arguments specify the location of the new pending
	buffer's upper left corner, relative to the current buffer's upper
	left corner, in surface-local coordinates. In other words, the
	x and y, combined with the new surface size define in which
	directions the surface's size changes.

	Surface contents are double-buffered state, see wl_surface.commit.

	The initial surface contents are void; there is no content.
	wl_surface.attach assigns the given wl_buffer as the pending
	wl_buffer. wl_surface.commit makes the pending wl_buffer the new
	surface contents, and the size of the surface becomes the size
	calculated from the wl_buffer, as described above. After commit,
	there is no pending buffer until the next attach.

	Committing a pending wl_buffer allows the compositor to read the
	pixels in the wl_buffer. The compositor may access the pixels at
	any time after the wl_surface.commit request. When the compositor
	will not access the pixels anymore, it will send the
	wl_buffer.release event. Only after receiving wl_buffer.release,
	the client may reuse the wl_buffer. A wl_buffer that has been
	attached and then replaced by another attach instead of committed
	will not receive a release event, and is not used by the
	compositor.

	Destroying the wl_buffer after wl_buffer.release does not change
	the surface contents. However, if the client destroys the
	wl_buffer before receiving the wl_buffer.release event, the surface
	contents become undefined immediately.

	If wl_surface.attach is sent with a NULL wl_buffer, the
	following wl_surface.commit will remove the surface content.
      
     */
    std::function<void(buffer_proxy_t, int32_t, int32_t)> &surface_resource_t::on_attach() {
        return std::static_pointer_cast<requests_t>(get_requests())->attach;
    }

    /** \brief mark part of the surface damaged
        \param x 
        \param y 
        \param width 
        \param height 

	This request is used to describe the regions where the pending
	buffer is different from the current surface contents, and where
	the surface therefore needs to be repainted. The compositor
	ignores the parts of the damage that fall outside of the surface.

	Damage is double-buffered state, see wl_surface.commit.

	The damage rectangle is specified in surface-local coordinates.

	The initial value for pending damage is empty: no damage.
	wl_surface.damage adds pending damage: the new pending damage
	is the union of old pending damage and the given rectangle.

	wl_surface.commit assigns pending damage as the current damage,
	and clears pending damage. The server will clear the current
	damage as it repaints the surface.

	Alternatively, damage can be posted with wl_surface.damage_buffer
	which uses buffer coordinates instead of surface coordinates,
	and is probably the preferred and intuitive way of doing this.
      
     */
    std::function<void(int32_t, int32_t, int32_t, int32_t)> &surface_resource_t::on_damage() {
        return std::static_pointer_cast<requests_t>(get_requests())->damage;
    }

    /** \brief request a frame throttling hint
        \param callback 

	Request a notification when it is a good time to start drawing a new
	frame, by creating a frame callback. This is useful for throttling
	redrawing operations, and driving animations.

	When a client is animating on a wl_surface, it can use the 'frame'
	request to get notified when it is a good time to draw and commit the
	next frame of animation. If the client commits an update earlier than
	that, it is likely that some updates will not make it to the display,
	and the client is wasting resources by drawing too often.

	The frame request will take effect on the next wl_surface.commit.
	The notification will only be posted for one frame unless
	requested again. For a wl_surface, the notifications are posted in
	the order the frame requests were committed.

	The server must send the notifications so that a client
	will not send excessive updates, while still allowing
	the highest possible update rate for clients that wait for the reply
	before drawing again. The server should give some time for the client
	to draw and commit after sending the frame callback events to let it
	hit the next output refresh.

	A server should avoid signaling the frame callbacks if the
	surface is not visible in any way, e.g. the surface is off-screen,
	or completely obscured by other opaque surfaces.

	The object returned by this request will be destroyed by the
	compositor after the callback is fired and as such the client must not
	attempt to use it after that point.

	The callback_data passed in the callback is the current time, in
	milliseconds, with an undefined base.
      
     */
    std::function<void(callback_proxy_t)> &surface_resource_t::on_frame() {
        return std::static_pointer_cast<requests_t>(get_requests())->frame;
    }

    /** \brief set opaque region
        \param region 

	This request sets the region of the surface that contains
	opaque content.

	The opaque region is an optimization hint for the compositor
	that lets it optimize the redrawing of content behind opaque
	regions.  Setting an opaque region is not required for correct
	behaviour, but marking transparent content as opaque will result
	in repaint artifacts.

	The opaque region is specified in surface-local coordinates.

	The compositor ignores the parts of the opaque region that fall
	outside of the surface.

	Opaque region is double-buffered state, see wl_surface.commit.

	wl_surface.set_opaque_region changes the pending opaque region.
	wl_surface.commit copies the pending region to the current region.
	Otherwise, the pending and current regions are never changed.

	The initial value for an opaque region is empty. Setting the pending
	opaque region has copy semantics, and the wl_region object can be
	destroyed immediately. A NULL wl_region causes the pending opaque
	region to be set to empty.
      
     */
    std::function<void(region_proxy_t)> &surface_resource_t::on_set_opaque_region() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_opaque_region;
    }

    /** \brief set input region
        \param region 

	This request sets the region of the surface that can receive
	pointer and touch events.

	Input events happening outside of this region will try the next
	surface in the server surface stack. The compositor ignores the
	parts of the input region that fall outside of the surface.

	The input region is specified in surface-local coordinates.

	Input region is double-buffered state, see wl_surface.commit.

	wl_surface.set_input_region changes the pending input region.
	wl_surface.commit copies the pending region to the current region.
	Otherwise the pending and current regions are never changed,
	except cursor and icon surfaces are special cases, see
	wl_pointer.set_cursor and wl_data_device.start_drag.

	The initial value for an input region is infinite. That means the
	whole surface will accept input. Setting the pending input region
	has copy semantics, and the wl_region object can be destroyed
	immediately. A NULL wl_region causes the input region to be set
	to infinite.
      
     */
    std::function<void(region_proxy_t)> &surface_resource_t::on_set_input_region() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_input_region;
    }

    /** \brief commit pending surface state

	Surface state (input, opaque, and damage regions, attached buffers,
	etc.) is double-buffered. Protocol requests modify the pending state,
	as opposed to the current state in use by the compositor. A commit
	request atomically applies all pending state, replacing the current
	state. After commit, the new pending state is as documented for each
	related request.

	On commit, a pending wl_buffer is applied first, and all other state
	second. This means that all coordinates in double-buffered state are
	relative to the new wl_buffer coming into use, except for
	wl_surface.attach itself. If there is no pending wl_buffer, the
	coordinates are relative to the current surface contents.

	All requests that need a commit to become effective are documented
	to affect double-buffered state.

	Other interfaces may add further double-buffered surface state.
      
     */
    std::function<void()> &surface_resource_t::on_commit() {
        return std::static_pointer_cast<requests_t>(get_requests())->commit;
    }

    /** \brief sets the buffer transformation
        \param transform 

	This request sets an optional transformation on how the compositor
	interprets the contents of the buffer attached to the surface. The
	accepted values for the transform parameter are the values for
	wl_output.transform.

	Buffer transform is double-buffered state, see wl_surface.commit.

	A newly created surface has its buffer transformation set to normal.

	wl_surface.set_buffer_transform changes the pending buffer
	transformation. wl_surface.commit copies the pending buffer
	transformation to the current one. Otherwise, the pending and current
	values are never changed.

	The purpose of this request is to allow clients to render content
	according to the output transform, thus permitting the compositor to
	use certain optimizations even if the display is rotated. Using
	hardware overlays and scanning out a client buffer for fullscreen
	surfaces are examples of such optimizations. Those optimizations are
	highly dependent on the compositor implementation, so the use of this
	request should be considered on a case-by-case basis.

	Note that if the transform value includes 90 or 270 degree rotation,
	the width of the buffer will become the surface height and the height
	of the buffer will become the surface width.

	If transform is not one of the values from the
	wl_output.transform enum the invalid_transform protocol error
	is raised.
      
     */
    std::function<void(output_transform)> &surface_resource_t::on_set_buffer_transform() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_buffer_transform;
    }

    /** \brief sets the buffer scaling factor
        \param scale 

	This request sets an optional scaling factor on how the compositor
	interprets the contents of the buffer attached to the window.

	Buffer scale is double-buffered state, see wl_surface.commit.

	A newly created surface has its buffer scale set to 1.

	wl_surface.set_buffer_scale changes the pending buffer scale.
	wl_surface.commit copies the pending buffer scale to the current one.
	Otherwise, the pending and current values are never changed.

	The purpose of this request is to allow clients to supply higher
	resolution buffer data for use on high resolution outputs. It is
	intended that you pick the same buffer scale as the scale of the
	output that the surface is displayed on. This means the compositor
	can avoid scaling when rendering the surface on that output.

	Note that if the scale is larger than 1, then you have to attach
	a buffer that is larger (by a factor of scale in each dimension)
	than the desired surface size.

	If scale is not positive the invalid_scale protocol error is
	raised.
      
     */
    std::function<void(int32_t)> &surface_resource_t::on_set_buffer_scale() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_buffer_scale;
    }

    /** \brief mark part of the surface damaged using buffer coordinates
        \param x 
        \param y 
        \param width 
        \param height 

	This request is used to describe the regions where the pending
	buffer is different from the current surface contents, and where
	the surface therefore needs to be repainted. The compositor
	ignores the parts of the damage that fall outside of the surface.

	Damage is double-buffered state, see wl_surface.commit.

	The damage rectangle is specified in buffer coordinates.

	The initial value for pending damage is empty: no damage.
	wl_surface.damage_buffer adds pending damage: the new pending
	damage is the union of old pending damage and the given rectangle.

	wl_surface.commit assigns pending damage as the current damage,
	and clears pending damage. The server will clear the current
	damage as it repaints the surface.

	This request differs from wl_surface.damage in only one way - it
	takes damage in buffer coordinates instead of surface-local
	coordinates. While this generally is more intuitive than surface
	coordinates, it is especially desirable when using wp_viewport
	or when a drawing library (like EGL) is unaware of buffer scale
	and buffer transform.

	Note: Because buffer transformation changes and damage requests may
	be interleaved in the protocol stream, it is impossible to determine
	the actual mapping between surface and buffer damage until
	wl_surface.commit time. Therefore, compositors wishing to take both
	kinds of damage into account will have to accumulate damage from the
	two requests separately and only transform from one to the other
	after receiving the wl_surface.commit.
      
     */
    std::function<void(int32_t, int32_t, int32_t, int32_t)> &surface_resource_t::on_damage_buffer() {
        return std::static_pointer_cast<requests_t>(get_requests())->damage_buffer;
    }



    /** \brief return pointer object
        \param id 

	The ID provided will be initialized to the wl_pointer interface
	for this seat.

	This request only takes effect if the seat has the pointer
	capability, or has had the pointer capability in the past.
	It is a protocol violation to issue this request on a seat that has
	never had the pointer capability.
      
     */
    std::function<void(pointer_proxy_t)> &seat_resource_t::on_get_pointer() {
        return std::static_pointer_cast<requests_t>(get_requests())->get_pointer;
    }

    /** \brief return keyboard object
        \param id 

	The ID provided will be initialized to the wl_keyboard interface
	for this seat.

	This request only takes effect if the seat has the keyboard
	capability, or has had the keyboard capability in the past.
	It is a protocol violation to issue this request on a seat that has
	never had the keyboard capability.
      
     */
    std::function<void(keyboard_proxy_t)> &seat_resource_t::on_get_keyboard() {
        return std::static_pointer_cast<requests_t>(get_requests())->get_keyboard;
    }

    /** \brief return touch object
        \param id 

	The ID provided will be initialized to the wl_touch interface
	for this seat.

	This request only takes effect if the seat has the touch
	capability, or has had the touch capability in the past.
	It is a protocol violation to issue this request on a seat that has
	never had the touch capability.
      
     */
    std::function<void(touch_proxy_t)> &seat_resource_t::on_get_touch() {
        return std::static_pointer_cast<requests_t>(get_requests())->get_touch;
    }

    /** \brief release the seat object

	Using this request a client can tell the server that it is not going to
	use the seat object anymore.
      
     */
    std::function<void()> &seat_resource_t::on_release() {
        return std::static_pointer_cast<requests_t>(get_requests())->release;
    }



    /** \brief set the pointer surface
        \param serial 
        \param surface 
        \param hotspot_x 
        \param hotspot_y 

	Set the pointer surface, i.e., the surface that contains the
	pointer image (cursor). This request gives the surface the role
	of a cursor. If the surface already has another role, it raises
	a protocol error.

	The cursor actually changes only if the pointer
	focus for this device is one of the requesting client's surfaces
	or the surface parameter is the current pointer surface. If
	there was a previous surface set with this request it is
	replaced. If surface is NULL, the pointer image is hidden.

	The parameters hotspot_x and hotspot_y define the position of
	the pointer surface relative to the pointer location. Its
	top-left corner is always at (x, y) - (hotspot_x, hotspot_y),
	where (x, y) are the coordinates of the pointer location, in
	surface-local coordinates.

	On surface.attach requests to the pointer surface, hotspot_x
	and hotspot_y are decremented by the x and y parameters
	passed to the request. Attach must be confirmed by
	wl_surface.commit as usual.

	The hotspot can also be updated by passing the currently set
	pointer surface to this request with new values for hotspot_x
	and hotspot_y.

	The current and pending input regions of the wl_surface are
	cleared, and wl_surface.set_input_region is ignored until the
	wl_surface is no longer used as the cursor. When the use as a
	cursor ends, the current and pending input regions become
	undefined, and the wl_surface is unmapped.
      
     */
    std::function<void(uint32_t, surface_proxy_t, int32_t, int32_t)> &pointer_resource_t::on_set_cursor() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_cursor;
    }

    /** \brief release the pointer object

	Using this request a client can tell the server that it is not going to
	use the pointer object anymore.

	This request destroys the pointer proxy object, so clients must not call
	wl_pointer_destroy() after using this request.
      
     */
    std::function<void()> &pointer_resource_t::on_release() {
        return std::static_pointer_cast<requests_t>(get_requests())->release;
    }



    std::function<void()> &keyboard_resource_t::on_release() {
        return std::static_pointer_cast<requests_t>(get_requests())->release;
    }



    std::function<void()> &touch_resource_t::on_release() {
        return std::static_pointer_cast<requests_t>(get_requests())->release;
    }





    /** \brief destroy region

	Destroy the region.  This will invalidate the object ID.
      
     */
    std::function<void()> &region_resource_t::on_destroy() {
        return std::static_pointer_cast<requests_t>(get_requests())->destroy;
    }

    /** \brief add rectangle to region
        \param x 
        \param y 
        \param width 
        \param height 

	Add the specified rectangle to the region.
      
     */
    std::function<void(int32_t, int32_t, int32_t, int32_t)> &region_resource_t::on_add() {
        return std::static_pointer_cast<requests_t>(get_requests())->add;
    }

    /** \brief subtract rectangle from region
        \param x 
        \param y 
        \param width 
        \param height 

	Subtract the specified rectangle from the region.
      
     */
    std::function<void(int32_t, int32_t, int32_t, int32_t)> &region_resource_t::on_subtract() {
        return std::static_pointer_cast<requests_t>(get_requests())->subtract;
    }



    /** \brief unbind from the subcompositor interface

	Informs the server that the client will not be using this
	protocol object anymore. This does not affect any other
	objects, wl_subsurface objects included.
      
     */
    std::function<void()> &subcompositor_resource_t::on_destroy() {
        return std::static_pointer_cast<requests_t>(get_requests())->destroy;
    }

    /** \brief give a surface the role sub-surface
        \param id 
        \param surface 
        \param parent 

	Create a sub-surface interface for the given surface, and
	associate it with the given parent surface. This turns a
	plain wl_surface into a sub-surface.

	The to-be sub-surface must not already have another role, and it
	must not have an existing wl_subsurface object. Otherwise a protocol
	error is raised.
      
     */
    std::function<void(subsurface_proxy_t, surface_proxy_t, surface_proxy_t)> &subcompositor_resource_t::on_get_subsurface() {
        return std::static_pointer_cast<requests_t>(get_requests())->get_subsurface;
    }



    /** \brief remove sub-surface interface

	The sub-surface interface is removed from the wl_surface object
	that was turned into a sub-surface with a
	wl_subcompositor.get_subsurface request. The wl_surface's association
	to the parent is deleted, and the wl_surface loses its role as
	a sub-surface. The wl_surface is unmapped.
      
     */
    std::function<void()> &subsurface_resource_t::on_destroy() {
        return std::static_pointer_cast<requests_t>(get_requests())->destroy;
    }

    /** \brief reposition the sub-surface
        \param x 
        \param y 

	This schedules a sub-surface position change.
	The sub-surface will be moved so that its origin (top left
	corner pixel) will be at the location x, y of the parent surface
	coordinate system. The coordinates are not restricted to the parent
	surface area. Negative values are allowed.

	The scheduled coordinates will take effect whenever the state of the
	parent surface is applied. When this happens depends on whether the
	parent surface is in synchronized mode or not. See
	wl_subsurface.set_sync and wl_subsurface.set_desync for details.

	If more than one set_position request is invoked by the client before
	the commit of the parent surface, the position of a new request always
	replaces the scheduled position from any previous request.

	The initial position is 0, 0.
      
     */
    std::function<void(int32_t, int32_t)> &subsurface_resource_t::on_set_position() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_position;
    }

    /** \brief restack the sub-surface
        \param sibling 

	This sub-surface is taken from the stack, and put back just
	above the reference surface, changing the z-order of the sub-surfaces.
	The reference surface must be one of the sibling surfaces, or the
	parent surface. Using any other surface, including this sub-surface,
	will cause a protocol error.

	The z-order is double-buffered. Requests are handled in order and
	applied immediately to a pending state. The final pending state is
	copied to the active state the next time the state of the parent
	surface is applied. When this happens depends on whether the parent
	surface is in synchronized mode or not. See wl_subsurface.set_sync and
	wl_subsurface.set_desync for details.

	A new sub-surface is initially added as the top-most in the stack
	of its siblings and parent.
      
     */
    std::function<void(surface_proxy_t)> &subsurface_resource_t::on_place_above() {
        return std::static_pointer_cast<requests_t>(get_requests())->place_above;
    }

    /** \brief restack the sub-surface
        \param sibling 

	The sub-surface is placed just below the reference surface.
	See wl_subsurface.place_above.
      
     */
    std::function<void(surface_proxy_t)> &subsurface_resource_t::on_place_below() {
        return std::static_pointer_cast<requests_t>(get_requests())->place_below;
    }

    /** \brief set sub-surface to synchronized mode

	Change the commit behaviour of the sub-surface to synchronized
	mode, also described as the parent dependent mode.

	In synchronized mode, wl_surface.commit on a sub-surface will
	accumulate the committed state in a cache, but the state will
	not be applied and hence will not change the compositor output.
	The cached state is applied to the sub-surface immediately after
	the parent surface's state is applied. This ensures atomic
	updates of the parent and all its synchronized sub-surfaces.
	Applying the cached state will invalidate the cache, so further
	parent surface commits do not (re-)apply old state.

	See wl_subsurface for the recursive effect of this mode.
      
     */
    std::function<void()> &subsurface_resource_t::on_set_sync() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_sync;
    }

    /** \brief set sub-surface to desynchronized mode

	Change the commit behaviour of the sub-surface to desynchronized
	mode, also described as independent or freely running mode.

	In desynchronized mode, wl_surface.commit on a sub-surface will
	apply the pending state directly, without caching, as happens
	normally with a wl_surface. Calling wl_surface.commit on the
	parent surface has no effect on the sub-surface's wl_surface
	state. This mode allows a sub-surface to be updated on its own.

	If cached state exists when wl_surface.commit is called in
	desynchronized mode, the pending state is added to the cached
	state, and applied as a whole. This invalidates the cache.

	Note: even if a sub-surface is set to desynchronized, a parent
	sub-surface may override it to behave as synchronized. For details,
	see wl_subsurface.

	If a surface's parent surface behaves as desynchronized, then
	the cached state is applied on set_desync.
      
     */
    std::function<void()> &subsurface_resource_t::on_set_desync() {
        return std::static_pointer_cast<requests_t>(get_requests())->set_desync;
    }




}

