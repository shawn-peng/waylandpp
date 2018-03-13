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

#ifndef WAYLAND_UTIL_HPP
#define WAYLAND_UTIL_HPP

#include <algorithm>
#include <typeinfo>
#include <utility>
#include <vector>
#include <list>
#include <string>
#include <wayland-util.h>

using namespace std;

#define wl_array_for_each_cpp(pos, array)                                    \
	for (pos = (decltype(pos))(array)->data;                                 \
		(const char *) pos < ((const char *) (array)->data + (array)->size); \
		(pos)++)

namespace wayland {
//class proxy_t;

class array_t;
class fixed_t;

namespace detail {
class any {
  private:
	class base {
	  public:
		virtual ~base() { }
		virtual const std::type_info &type_info() const = 0;
		virtual base *clone() const = 0;
	};

	template <typename T>
	class derived : public base {
	  private:
		T val;
		friend class any;

	  public:
		derived(const T &t)
			: val(t) { }

		virtual const std::type_info &type_info() const override {
			return typeid(T);
		}

		virtual base *clone() const override {
			return new derived<T>(val);
		}
	};

	base *val;

  public:
	any()
		: val(nullptr) { }

	any(const any &a)
		: val(a.val ? a.val->clone() : nullptr) { }

	template <typename T>
	any(const T &t)
		: val(new derived<T>(t)) { }

	~any() {
		delete val;
	}

	any &operator=(const any &a) {
		delete val;
		val = a.val ? a.val->clone() : nullptr;
		return *this;
	}

	template <typename T>
	any &operator=(const T &t) {
		if(val && typeid(T) == val->type_info())
			static_cast<derived<T>*>(val)->val = t;
		else {
			delete val;
			val = new derived<T>(t);
		}
		return *this;
	}

	template <typename T>
	T &get() {
		if(val && typeid(T) == val->type_info())
			return static_cast<derived<T>*>(val)->val;
		else
			throw std::bad_cast();
	}
};

template<unsigned int size, int id = 0>
class bitfield {
	uint32_t v;
	static const uint32_t mask = (1 << size) - 1;

  public:
	explicit bitfield(const uint32_t value = 0)
		: v(value) {
	}

	explicit operator uint32_t() const {
		return v;
	}

	operator bool() const {
		return v;
	}

	bitfield(const bitfield<size, id> &b) {
		operator=(b);
	}

	bool operator==(const bitfield<size, id> &b) {
		return v == b.v;
	}

	bool operator!=(const bitfield<size, id> &b) {
		return !operator==(b);
	}

	bitfield<size, id> &operator=(const bitfield<size, id> &b) {
		v = static_cast<uint32_t>(b);
		return *this;
	}

	bitfield<size, id> operator|(const bitfield<size, id> &b) const {
		return bitfield<size, id>(v | static_cast<uint32_t>(b));
	}

	bitfield<size, id> operator&(const bitfield<size, id> &b) const {
		return bitfield<size, id>(v & static_cast<uint32_t>(b));
	}

	bitfield<size, id> operator^(const bitfield<size, id> &b) const {
		return bitfield<size, id>((v ^ static_cast<uint32_t>(b)) & mask);
	}

	bitfield<size, id> operator~() const {
		return bitfield<size, id>(~v & mask);
	}

	bitfield<size, id> &operator|=(const bitfield<size, id> &b) {
		operator=(*this | b);
		return *this;
	}

	bitfield<size, id> &operator&=(const bitfield<size, id> &b) {
		operator=(*this & b);
		return *this;
	}

	bitfield<size, id> &operator^=(const bitfield<size, id> &b) {
		operator=(*this ^ b);
		return *this;
	}
};

class object_t {
  protected:
	wl_object *object;

  public:
	object_t(wl_object *obj) : object(obj) { }
	object_t &operator =(const object_t &o) { object = o.object; }

	friend class argument_t;
};

class argument_t {
  private:
	bool is_array;

  public:
	wl_argument argument;

	argument_t();
	argument_t(const argument_t &arg);
	argument_t &operator=(const argument_t &arg);
	~argument_t();

	// handles integers, file descriptors and fixed point numbers
	// (this works, because wl_argument is an union)
	argument_t(uint32_t i);

	// handels fixed
	argument_t(fixed_t f);

	// handles strings
	argument_t(std::string s);

	// handles objects
	argument_t(object_t *p);

	// handles arrays
	argument_t(array_t a);
};
}

class fixed_t {
  private:
	  wl_fixed_t fixed;
  public:
	  fixed_t() { }
	  fixed_t(int x) { fixed = wl_fixed_from_int(x); }
	  fixed_t(double x) { fixed = wl_fixed_from_double(x); }
	  //fixed_t(wl_fixed_t x) { fixed = x; }
	  void set_data(wl_fixed_t x) { fixed = x; }
	  wl_fixed_t get_data() { return fixed; }
	  operator int() { return wl_fixed_to_int(fixed); }
	  operator double() { return wl_fixed_to_double(fixed); }
};

class array_t {
  private:
	wl_array a;

	array_t(wl_array *arr);
	void get(wl_array *arr);

	friend class proxy_t;
	friend class resource_t;
	friend class detail::argument_t;

  public:
	array_t();
	array_t(const array_t &arr);
	array_t(array_t &&arr);

	template <typename T> array_t(const std::vector<T> &v) {
		wl_array_init(&a);
		wl_array_add(&a, v.size()*sizeof(T));
		T *p;
		unsigned int c = 0;
		wl_array_for_each_cpp(p, &a) {
			*p = v.at(c++);
		}
	}

	~array_t();
	array_t &operator=(const array_t &arr);
	array_t &operator=(array_t &&arr);

	template <typename T> array_t &operator=(const std::vector<T> &v) {
		wl_array_release(&a);
		wl_array_init(&a);
		wl_array_add(&a, v.size()*sizeof(T));
		T *p;
		unsigned int c = 0;
		wl_array_for_each_cpp(p, &a) {
			*p = v.at(c++);
		}
		return *this;
	}

	template <typename T> operator std::vector<T>() {
		std::vector<T> v;
		T *p;
		wl_array_for_each_cpp(p, &a) {
			v.push_back(*p);
		}
		return v;
	}
};

typedef wl_interface interface_t;

}

#endif
