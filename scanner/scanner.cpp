/*
 * (C) Copyright 2014-2016 Nils Brause
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>

#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <cctype>
#include <cmath>
#include <algorithm>

#include "pugixml.hpp"

using namespace std;
using namespace pugi;

std::list<std::string> interface_names;

enum source_t {
	SERVER = 1,
	CLIENT,
};

struct element_t {
	std::string summary;
	std::string description;
};

struct argument_t : public element_t {
	std::string type;
	std::string name;
	std::string interface;
	std::string enum_iface;
	std::string enum_name;
	bool allow_null;

	std::string print_type(source_t st) {
		if (interface != "") {
			if (st == SERVER) {
				return interface + "_resource_t";
			} else if (st == CLIENT) {
				return interface + "_proxy_t";
			}
		} else if (enum_iface != "")
			return enum_iface + "_" + enum_name;
		else if (type == "int")
			return "int32_t";
		else if (type == "uint")
			return "uint32_t";
		else if (type == "fixed")
			return "fixed_t";
		else if (type == "string")
			return "std::string";
		else if (type == "object") {
			if (st == SERVER) {
				return "resource_t";
			} else if (st == CLIENT) {
				return "proxy_t";
			}
		} else if (type == "new_id") {
			if (st == SERVER) {
				return "resource_t";
			} else if (st == CLIENT) {
				return "proxy_t";
			}
		} else if (type == "fd")
			return "int";
		else if (type == "array")
			return "array_t";
		else
			return type;
	}

	std::string print_short() {
		if (type == "int")
			return "i";
		else if (type == "uint")
			return "u";
		else if (type == "fixed")
			return "f";
		else if (type == "string")
			return "s";
		else if (type == "object")
			return "o";
		else if (type == "new_id")
			return "n";
		else if (type == "array")
			return "a";
		else if (type == "fd")
			return "h";
		else
			return "x";
	}

	std::string print_argument(source_t st) {
		return print_type(st) + " " + name;
	}
};

struct event_t : public element_t {
	int opcode;
	argument_t ret;
	std::string name;
	std::list<argument_t> args;
	int since;

	std::string print_functional() {
		std::stringstream ss;
		ss << "        std::function<void(";
		for (auto &arg : args)
			ss << arg.print_type(CLIENT) << ", ";
		if (args.size())
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		ss.seekp(0, std::ios_base::end);
		ss << ")> " << name << ";";
		return ss.str();
	}

	std::string print_dispatcher(int opcode) {
		std::stringstream ss;
		ss << "    case " << opcode << ":" << std::endl
		   << "        if(events->" << name << ") events->" << name << "(";

		int c = 0;
		for (auto &arg : args)
			if (arg.enum_name != "") {
				ss << arg.print_type(CLIENT) << "(args[" << c++ << "].get<uint32_t>()), ";
			} else if (arg.interface != "") {
				ss << arg.print_type(CLIENT) << "(args[" << c++ << "].get<proxy_t>()), ";
			} else {
				ss << "args[" << c++ << "].get<" << arg.print_type(CLIENT) << ">(), ";
			}
		if (args.size()) {
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		}
		ss.seekp(0, std::ios_base::end);
		ss << ");" << std::endl
		   << "        break;";
		return ss.str();
	}

	std::string print_header() {
		std::stringstream ss;
		if (description != "") {
			ss << "    /** \\brief " << summary << std::endl;
			for (auto &arg : args) {
				if (arg.type == "new_id") {
					if (arg.interface == "")
						ss << "        \\param interface Interface to bind" << std::endl
						   << "        \\param version Interface version" << std::endl;
				} else {
					ss << "        \\param " << arg.name << " ";
					if (arg.summary != "")
						ss << "    " <<  arg.summary;
					ss << std::endl;
				}
			}
			ss << description << std::endl
			   << "     */" << std::endl;
		}

		ss << "    void send_" << name << "(";

		for (auto &arg : args) {
			if (arg.type == "new_id") {
				if (arg.interface == "") {
				// 	ss << "proxy_t &interface, uint32_t version, ";
				// 	new_id_arg = true;
					assert(0);
				}
			} else {
			}
			ss << arg.print_argument(SERVER) << ", ";
		}

		if (ss.str().substr(ss.str().size() - 2, 2) == ", ") {
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		}
		ss.seekp(0, std::ios_base::end);
		ss << ");" << std::endl;
		return ss.str();
	}

	std::string print_body(std::string interface_name) {
		std::stringstream ss;
		ss <<  "void ";
		ss << interface_name << "_resource_t::send_" << name << "(";

		// bool new_id_arg = false;
		for (auto &arg : args) {
			if (arg.type == "new_id") {
				if (arg.interface == "") {
				// 	ss << "proxy_t &interface, uint32_t version, ";
				// 	new_id_arg = true;
					assert(0);
				}
			} else {
			}
			ss << arg.print_argument(SERVER) << ", ";
		}

		if (ss.str().substr(ss.str().size() - 2, 2) == ", ") {
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		}

		ss.seekp(0, std::ios_base::end);
		ss << ") {" << std::endl;

		ss << "    post_event(" << opcode << ", ";

		for (auto &arg : args) {
			if (arg.type == "new_id") {
				if (arg.interface == "") {
					// ss << "std::string(interface.get_iface_ptr()->name), version, ";
					assert(0);
				}
				// ss << "NULL, ";
				ss << "&" << arg.name + ", ";
			} else if (arg.type == "object") {
				ss << "&" << arg.name + ", ";
			} else if (arg.enum_name != "") {
				ss << "static_cast<uint32_t>(" << arg.name + "), ";
			} else {
				ss << arg.name + ", ";
			}
		}

		ss.str(ss.str().substr(0, ss.str().size() - 2));
		ss.seekp(0, std::ios_base::end);
		ss << ");" << std::endl;

		if (ret.name != "") {
			assert(0);
			// if (new_id_arg) {
			// 	ss << "    interface = interface.copy_constructor(p);" << std::endl
			// 	   << "    return interface;" << std::endl;
			// } else
			// 	ss << "    return " << ret.print_type() << "(p);" << std::endl;
		}
		ss << "}" << endl;
		return ss.str();
	}

	std::string print_signal_header() {
		std::stringstream ss;
		if (description != "") {
			ss << "    /** \\brief " << summary << std::endl;
			for (auto &arg : args) {
				ss << "        \\param " << arg.name << " ";
				if (arg.summary != "")
					ss << arg.summary;
				ss << std::endl;
			}
			ss << description << std::endl
			   << "     */" << std::endl;
		}
		ss << "    std::function<void(";
		for (auto &arg : args)
			ss << arg.print_type(CLIENT) + ", ";
		if (args.size())
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		ss.seekp(0, std::ios_base::end);
		ss << ")> &on_" <<  name << "();" << std::endl;

		return ss.str();
	}

	std::string print_signal_body(std::string interface_name) {
		std::stringstream ss;
		ss << "std::function<void(";
		for (auto &arg : args) {
			ss << arg.print_type(CLIENT) << ", ";
		}
		if (args.size()) {
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		}
		ss.seekp(0, std::ios_base::end);
		ss << ")> &" << endl;
	   	ss <<  interface_name + "_proxy_t::on_" + name + "() {" << std::endl
		   << "    return std::static_pointer_cast<events_t>(get_events())->" + name + ";" << std::endl
		   << "}" << std::endl;
		return ss.str();
	}
};

struct request_t : public event_t {

	std::string print_functional() {
		std::stringstream ss;
		ss << "        std::function<void(";
		for (auto &arg : args) {
			ss << arg.print_type(SERVER) << ", ";
		}
		if (args.size())
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		ss.seekp(0, std::ios_base::end);
		ss << ")> " << name << ";";
		return ss.str();
	}

	std::string print_dispatcher(int opcode) {
		std::stringstream ss;
		ss << "    case " << opcode << ":" << std::endl
		   << "        if(requests->" << name << ") requests->" << name << "(";

		int c = 0;
		for (auto &arg : args)
			if (arg.enum_name != "") {
				ss << arg.print_type(SERVER) << "(args[" << c++ << "].get<uint32_t>()), ";
			} else if (arg.interface != "") {
				ss << arg.print_type(SERVER) << "(args[" << c++ << "].get<resource_t>()), ";
			} else {
				ss << "args[" << c++ << "].get<" << arg.print_type(SERVER) << ">(), ";
			}
		if (args.size()) {
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		}
		ss.seekp(0, std::ios_base::end);
		ss << ");" << std::endl
		   << "        break;";
		return ss.str();
	}

	std::string print_header() {
		std::stringstream ss;
		if (description != "") {
			ss << "    /** \\brief " << summary << std::endl;
			if (ret.summary != "") {
				ss << "      \\return " << ret.summary << std::endl;
			}
			for (auto &arg : args) {
				if (arg.type == "new_id") {
					if (arg.interface == "")
						ss << "        \\param interface Interface to bind" << std::endl
						   << "        \\param version Interface version" << std::endl;
				} else {
					ss << "        \\param " << arg.name << " ";
					if (arg.summary != "")
						ss << "    " <<  arg.summary;
					ss << std::endl;
				}
			}
			ss << description << std::endl
			   << "     */" << std::endl;
		}

		// =================================================================
		// with definitaion
		// if (ret.name == "")
		// 	ss << "    void ";
		// else
		// 	ss << "    " << ret.print_type() << " ";
		// ss << name << "(";

		// bool new_id_arg = false;
		// for (auto &arg : args) {
		// 	if (arg.type == "new_id") {
		// 		if (arg.interface == "") {
		// 			ss << "proxy_t &interface, uint32_t version, ";
		// 			new_id_arg = true;
		// 		}
		// 	} else {
		// 		ss << arg.print_argument() << ", ";
		// 	}
		// }

		// if (ss.str().substr(ss.str().size() - 2, 2) == ", ")
		// 	ss.str(ss.str().substr(0, ss.str().size() - 2));
		// ss.seekp(0, std::ios_base::end);
		// ss << ") {" << std::endl;

		// if (ret.name == "") {
		// 	ss << "        marshal(" << opcode << ", ";
		// } else {
		// 	ss << "        proxy_t p = marshal_constructor(" << opcode << ", ";
		// 	if (ret.interface == "")
		// 		ss << "interface.interface";
		// 	else
		// 		ss << "&" << ret.interface << "_interface";
		// 	ss << ", ";
		// }

		// for (auto &arg : args) {
		// 	if (arg.type == "new_id") {
		// 		if (arg.interface == "")
		// 			ss << "std::string(interface.interface->name), version, ";
		// 		ss << "NULL, ";
		// 	} else if (arg.enum_name != "")
		// 		ss << "static_cast<uint32_t>(" << arg.name + "), ";
		// 	else
		// 		ss << arg.name + ", ";
		// }

		// ss.str(ss.str().substr(0, ss.str().size() - 2));
		// ss.seekp(0, std::ios_base::end);
		// ss << ");" << std::endl;

		// if (ret.name != "") {
		// 	if (new_id_arg) {
		// 		ss << "        interface = interface.copy_constructor(p);" << std::endl
		// 		   << "        return interface;" << std::endl;
		// 	} else
		// 		ss << "        return " << ret.print_type() << "(p);" << std::endl;
		// }
		// ss << "    }" << std::endl;
		//return ss.str();

		// =================================================================
		// only declaration
		if (ret.name == "")
			ss << "    void ";
		else
			ss << "    " << ret.print_type(CLIENT) << " ";
		ss << name << "(";

		for (auto &arg : args)
			if (arg.type == "new_id") {
				if (arg.interface == "")
					ss << "proxy_t &interface, uint32_t version, ";
			} else
				ss << arg.print_argument(CLIENT) << ", ";

		if (ss.str().substr(ss.str().size() - 2, 2) == ", ")
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		ss.seekp(0, std::ios_base::end);
		ss << ");" << std::endl;
		return ss.str();
	}

	std::string print_handle_header() {
		std::stringstream ss;
		if (description != "") {
			ss << "    /** \\brief " << summary << std::endl;
			for (auto &arg : args) {
				ss << "        \\param " << arg.name << " ";
				if (arg.summary != "")
					ss << arg.summary;
				ss << std::endl;
			}
			ss << description << std::endl
			   << "     */" << std::endl;
		}
		ss << "    std::function<void(";
		for (auto &arg : args) {
			ss << arg.print_type(SERVER) + ", ";
		}
		if (args.size())
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		ss.seekp(0, std::ios_base::end);
		ss << ")> &on_" <<  name << "();" << std::endl;

		return ss.str();
	}

	std::string print_handle_body(std::string interface_name) {
		std::stringstream ss;
		// if (description != "") {
		// 	ss << "/** \\brief " << summary << std::endl;
		// 	for (auto &arg : args) {
		// 		ss << "    \\param " << arg.name << " ";
		// 		if (arg.summary != "")
		// 			ss << arg.summary;
		// 		ss << std::endl;
		// 	}
		// 	ss << description << std::endl
		// 	   << " */" << std::endl;
		// }
		ss << "std::function<void(";
		for (auto &arg : args)
			ss << arg.print_type(SERVER) + ", ";
		if (args.size())
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		ss.seekp(0, std::ios_base::end);
		ss << ")> &" << endl
		   << interface_name << "_resource_t::on_" <<  name << "() {" << std::endl
		   << "    return std::static_pointer_cast<requests_t>(get_requests())->" + name + ";" << std::endl
		   << "}" << std::endl;
		return ss.str();
	}

	std::string print_body(std::string interface_name) {
		std::stringstream ss;
		if (ret.name == "")
			ss <<  "void ";
		else
			ss << ret.print_type(CLIENT) << " ";
		ss << interface_name << "_proxy_t::" << name << "(";

		bool new_id_arg = false;
		for (auto &arg : args) {
			if (arg.type == "new_id") {
				if (arg.interface == "") {
					ss << "proxy_t &interface, uint32_t version, ";
					new_id_arg = true;
				}
			} else {
				ss << arg.print_argument(CLIENT) << ", ";
			}
		}

		if (ss.str().substr(ss.str().size() - 2, 2) == ", ") {
			ss.str(ss.str().substr(0, ss.str().size() - 2));
		}

		ss.seekp(0, std::ios_base::end);
		ss << ") {" << std::endl;

		if (ret.name == "") {
			ss << "    marshal(" << opcode << ", ";
		} else {
			ss << "    proxy_t p = marshal_constructor(" << opcode << ", ";
			if (ret.interface == "") {
				//ss << "    proxy_t p = marshal_constructor_dynamic(" << opcode << ", interface";
				ss << "interface.get_iface_ptr()";
			} else {
				//ss << "    proxy_t p = marshal_constructor(" << opcode << ", ";
				ss << "&" << ret.interface << "_interface";
			}
			ss << ", ";
		}

		for (auto &arg : args) {
			if (arg.type == "new_id") {
				if (arg.interface == "") {
					ss << "std::string(interface.get_iface_ptr()->name), version, ";
				}
				ss << "NULL, ";
			} else if (arg.type == "object") {
				ss << "&" << arg.name + ", ";
			} else if (arg.enum_name != "") {
				ss << "static_cast<uint32_t>(" << arg.name + "), ";
			} else {
				ss << arg.name + ", ";
			}
		}

		ss.str(ss.str().substr(0, ss.str().size() - 2));
		ss.seekp(0, std::ios_base::end);
		ss << ");" << std::endl;

		if (ret.name != "") {
			if (new_id_arg) {
				ss << "    interface = interface.copy_constructor(p);" << std::endl
				   << "    return interface;" << std::endl;
			} else
				ss << "    return " << ret.print_type(CLIENT) << "(p);" << std::endl;
		}
		ss << "}" << endl;
		return ss.str();
	}
};

struct enum_entry_t : public element_t {
	std::string name;
	std::string value;
};

struct enumeration_t : public element_t {
	std::string name;
	std::list<enum_entry_t> entries;
	bool bitfield;
	int id;
	uint32_t width;

	std::string print_forward(std::string iface_name) {
		std::stringstream ss;
		if (!bitfield)
			ss << "enum class " << iface_name << "_" << name << " : uint32_t;" << std::endl;
		else
			ss << "struct " << iface_name << "_" << name << ";" << std::endl;
		return ss.str();
	}

	std::string print_header(std::string iface_name) {
		std::stringstream ss;
		if (description != "") {
			ss << "/** \\brief " << summary << std::endl
			   << description << std::endl
			   << "  */" << std::endl;
		}

		if (!bitfield) {
			ss << "enum class " << iface_name << "_" << name << " : uint32_t {" << std::endl;
		} else {
			ss << "struct " << iface_name << "_" << name << " : public detail::bitfield<" << width << ", " << id << "> {" << std::endl
			   << "    " << iface_name << "_" << name << "(const detail::bitfield<" << width << ", " << id << "> &b)" << std::endl
			   << "        : detail::bitfield<" << width << ", " << id << ">(b) {}" << std::endl
			   << "    " << iface_name << "_" << name << "(const uint32_t value)" << std::endl
			   << "        : detail::bitfield<" << width << ", " << id << ">(value) {}" << std::endl;
		}

		for (auto &entry : entries) {
			if (entry.description != "") {
				ss << "    /** \\brief " << entry.summary << std::endl
				   << entry.description << std::endl
				   << "     */" << std::endl;
			}

			if (!bitfield) {
				ss << "    " << entry.name << " = " << entry.value << "," << std::endl;
			} else {
				ss << "    static const detail::bitfield<" << width << ", " << id << "> " << entry.name << ";" << std::endl;
			}
		}

		if (!bitfield) {
			ss.str(ss.str().substr(0, ss.str().size() - 2));
			ss.seekp(0, std::ios_base::end);
			ss << std::endl;
		}

		ss << "};" << std::endl;
		return ss.str();
	}

	std::string print_body(std::string iface_name) {
		std::stringstream ss;
		if (bitfield)
			for (auto &entry : entries) {
				ss << "const bitfield<" << width << ", " << id << "> " << iface_name << "_" << name
				   << "::" << entry.name << "{" << entry.value << "};" << std::endl;
			}
		return ss.str();
	}
};

struct interface_t : public element_t {
	int version;
	std::string name;
	std::string client_class;
	std::string server_class;
	std::string orig_name;
	int destroy_opcode;
	std::list<request_t> requests;
	std::list<event_t> events;
	std::list<enumeration_t> enums;

	//std::string proxyclass_name;

	std::string print_forward(source_t stype) {
		std::stringstream ss;
		if (stype == SERVER) {
			ss << "class " << server_class << ";" << std::endl;
			for (auto &e : enums) {
				ss << e.print_forward(name);
			}
		} else if (stype == CLIENT) {
			ss << "class " << client_class << ";" << std::endl;
			for (auto &e : enums) {
				ss << e.print_forward(name);
			}
		}
		ss << std::endl;
		return ss.str();
	}

	std::string print_header(source_t stype) {
		std::stringstream ss;
		if (description != "") {
			ss << "/** \\brief " << summary << std::endl
			   << description << std::endl
			   << " */" << std::endl;
		}

		if (stype == SERVER) {
			ss << "class " << server_class << " : public resource_t {" << std::endl
				<< "private:" << std::endl
				<< "    struct requests_t : public resource_t::requests_base_t {" << std::endl;

			//for (auto &event : events) {
			for (auto &request : requests) {
				ss << request.print_functional() << std::endl;
			}

			ss << "    };" << std::endl
				<< std::endl
				<< "    static int dispatcher(int opcode, std::vector<detail::any> args, std::shared_ptr<resource_t::requests_base_t> e);" << std::endl
				<< std::endl;

			ss << "public:" << std::endl
				<< "    " << server_class << "();" << std::endl
				<< "    explicit " << server_class << "(const resource_t &resource);" << std::endl
				<< std::endl
				<< "    void bind();" << std::endl
				<< std::endl;

			for (auto &request : requests) {
				ss << request.print_handle_header() << std::endl;
			}

			for (auto &event : events) {
				if (event.name != "destroy") {
					ss << event.print_header() << std::endl;
				}
			}
		} else if (stype == CLIENT) {
			ss << "class " << client_class << " : public proxy_t {" << std::endl
				<< "private:" << std::endl
				<< "    struct events_t : public proxy_t::events_base_t {" << std::endl;

			for (auto &event : events) {
				ss << event.print_functional() << std::endl;
			}

			ss << "    };" << std::endl
				<< std::endl
				<< "    static int dispatcher(int opcode, std::vector<detail::any> args, std::shared_ptr<proxy_t::events_base_t> e);" << std::endl
				<< std::endl;

			ss << "public:" << std::endl
				<< "    " << client_class << "();" << std::endl
				<< "    explicit " << client_class << "(const proxy_t &proxy);" << std::endl
				<< std::endl;

			for (auto &event : events) {
				ss << event.print_signal_header() << std::endl;
			}

			for (auto &request : requests) {
				if (request.name != "destroy") {
					ss << request.print_header() << std::endl;
				}
			}
		}

		ss << "};" << std::endl
		   << std::endl;

		for (auto &enumeration : enums) {
			ss << enumeration.print_header(name) << std::endl;
		}

		return ss.str();
	}

	std::string print_interface_header() {
		std::stringstream ss;
		ss << "    extern const wl_interface " << name << "_interface;" << std::endl;
		return ss.str();
	}

	// std::string print_body() {
	// 	std::stringstream ss;
	// 	ss << name << "_t::" << name << "_t(const proxy_t &p)" << std::endl
	// 	   << "  : proxy_t(p) {" << std::endl
	// 	   << "    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);" << std::endl
	// 	   << "    set_destroy_opcode(" << destroy_opcode << ");" << std::endl
	// 	   << "    interface = &" << name << "_interface;" << std::endl
	// 	   << "    copy_constructor = [] (const proxy_t &p) -> proxy_t" << std::endl
	// 	   << "    { return " << name << "_t(p); };" << std::endl
	// 	   << "}" << std::endl
	// 	   << std::endl
	// 	   << name << "_t::" << name << "_t() {" << std::endl
	// 	   << "  interface = &" << name << "_interface;" << std::endl
	// 	   << "  copy_constructor = [] (const proxy_t &p) -> proxy_t" << std::endl
	// 	   << "    { return " << name << "_t(p); };" << std::endl
	// 	   << "}" << std::endl
	// 	   << std::endl;

	// 	for (auto &request : requests)
	// 		if (request.name != "destroy")
	// 			ss << request.print_body(name) << std::endl
	// 			   << std::endl;

	// 	ss << "int " << name << "_t::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e)" << std::endl
	// 	   << "{" << std::endl;

	// 	if (events.size()) {
	// 		ss << "  std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);" << std::endl
	// 		   << "  switch(opcode)" << std::endl
	// 		   << "    {" << std::endl;

	// 		int opcode = 0;
	// 		for (auto &event : events)
	// 			ss << event.print_dispatcher(opcode++) << std::endl;

	// 		ss << "    }" << std::endl;
	// 	}

	// 	ss << "  return 0;" << std::endl
	// 	   << "}" << std::endl;

	// 	for (auto &enumeration : enums)
	// 		ss << enumeration.print_body(name) << std::endl;

	// 	return ss.str();
	// }

	std::string print_common_defs() {
		std::stringstream ss;
		ss << "const wl_interface wayland::detail::" << name << "_interface = {" << std::endl
		   << "    \"" << orig_name << "\"," << std::endl
		   << "    " << version << "," << std::endl
		   << "    " << requests.size() << "," << std::endl
		   << "    (const wl_message[]) {" << std::endl;
		for (auto &request : requests) {
			ss << "        {" << std::endl
			   << "            \"" << request.name << "\"," << std::endl
			   << "            \"";
			if (request.since > 1) {
				ss << request.since;
			}
			for (auto &arg : request.args) {
				if (arg.allow_null) {
					ss << "?";
				}
				if (arg.type == "new_id" && arg.interface == "") {
					ss << "su";
				}
				ss << arg.print_short();
			}
			ss << "\"," << std::endl
			   << "            (const wl_interface*[]) {" << std::endl;
			for (auto &arg : request.args) {
				if (arg.interface != "") {
					ss  << "                &" << arg.interface << "_interface," << std::endl;
				} else {
					ss  << "                NULL," << std::endl;
				}
			}
			ss << "            }," << std::endl
			   << "        }," << std::endl;
		}
		ss << "    }," << std::endl
		   << "    " << events.size() << "," << std::endl
		   << "    (const wl_message[]) {" << std::endl;
		for (auto &event : events) {
			ss << "        {" << std::endl
			   << "            \"" << event.name << "\"," << std::endl
			   << "            \"";
			if (event.since > 1) {
				ss << event.since;
			}
			for (auto &arg : event.args) {
				if (arg.allow_null) {
					ss << "?";
				}
				if (arg.type == "new_id" && arg.interface == "") {
					ss << "su";
				}
				ss << arg.print_short();
			}
			ss << "\"," << std::endl
			   << "            (const wl_interface*[]) {" << std::endl;
			for (auto &arg : event.args) {
				if (arg.interface != "") {
					ss  << "                &" << arg.interface << "_interface," << std::endl;
				} else {
					ss  << "                NULL," << std::endl;
				}
			}
			ss << "            }," << std::endl
			   << "        }," << std::endl;
		}
		ss << "    }" << std::endl
		   << "};" << std::endl
		   << std::endl;

		for (auto &enumeration : enums)
			ss << enumeration.print_body(name) << std::endl;

		ss << std::endl;

		return ss.str();
	}

	std::string print_memdef(source_t stype) {
		std::stringstream ss;

		if (stype == SERVER) {
			// constructor
			ss << server_class << "::" << server_class << "(const resource_t &p)" << std::endl
			   << "  : resource_t(p) {" << std::endl
			   //<< "    set_requests(std::shared_ptr<resource_t::requests_base_t>(new requests_t), dispatcher);" << std::endl
			   << "    bind();" << std::endl
			   << "    interface = &" << name << "_interface;" << std::endl
			   << "    copy_constructor = [] (const resource_t &p) -> resource_t" << std::endl
			   << "    { return " << server_class << "(p); };" << std::endl
			   << "}" << std::endl
			   << std::endl
			   << server_class << "::" << server_class << "() {" << std::endl
			   << "  interface = &" << name << "_interface;" << std::endl
			   << "  copy_constructor = [] (const resource_t &p) -> resource_t" << std::endl
			   << "    { return " << server_class << "(p); };" << std::endl
			   << "}" << std::endl
			   << std::endl;

			// bind
			ss << "void " << server_class << "::bind() {" << std::endl
			   << "    set_requests(std::shared_ptr<resource_t::requests_base_t>(new requests_t), dispatcher);" << std::endl
			   << "}" << std::endl
			   << std::endl;

			// dispatcher
			ss << "int " << server_class << "::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<resource_t::requests_base_t> e) {" << std::endl
			   << std::endl;
	
			if (requests.size()) {
				ss << "    std::shared_ptr<requests_t> requests = std::static_pointer_cast<requests_t>(e);" << std::endl
				   << "    switch(opcode) {" << std::endl;
	
				int opcode = 0;
				for (auto &request : requests) {
					ss << request.print_dispatcher(opcode++) << std::endl;
				}
	
				ss << "    }" << std::endl;
			}
	
			ss << "    return 0;" << std::endl
			   << "}" << std::endl;

			for (auto &event : events) {
				ss << event.print_body(name) << std::endl;
			}
			for (auto &request : requests) {
				ss << request.print_handle_body(name) << std::endl;
			}
		} else if (stype == CLIENT) {
			ss << client_class << "::" << client_class << "(const proxy_t &p)" << std::endl
			   << "  : proxy_t(p) {" << std::endl
			   << "    set_events(std::shared_ptr<proxy_t::events_base_t>(new events_t), dispatcher);" << std::endl
			   << "    set_destroy_opcode(" << destroy_opcode << ");" << std::endl
			   << "    interface = &" << name << "_interface;" << std::endl
			   << "    copy_constructor = [] (const proxy_t &p) -> proxy_t" << std::endl
			   << "    { return " << client_class << "(p); };" << std::endl
			   << "}" << std::endl
			   << std::endl
			   << client_class << "::" << client_class << "() {" << std::endl
			   << "  interface = &" << name << "_interface;" << std::endl
			   << "  copy_constructor = [] (const proxy_t &p) -> proxy_t" << std::endl
			   << "    { return " << client_class << "(p); };" << std::endl
			   << "}" << std::endl
			   << std::endl;

			// dispatcher
			ss << "int " << client_class << "::dispatcher(int opcode, std::vector<any> args, std::shared_ptr<proxy_t::events_base_t> e) {" << std::endl
			   << std::endl;
	
			if (events.size()) {
				ss << "    std::shared_ptr<events_t> events = std::static_pointer_cast<events_t>(e);" << std::endl
				   << "    switch(opcode) {" << std::endl;
	
				int opcode = 0;
				for (auto &event : events) {
					ss << event.print_dispatcher(opcode++) << std::endl;
				}
	
				ss << "    }" << std::endl;
			}
	
			ss << "  return 0;" << std::endl
			   << "}" << std::endl;

			for (auto &request : requests) {
				if (request.name != "destroy") {
					ss << request.print_body(name) << std::endl;
				}
			}
			for (auto &event : events) {
				ss << event.print_signal_body(name) << std::endl;
			}
		}
		ss << std::endl;

		return ss.str();
	}
};

struct protocol_t : public element_t {
	std::string name;
	std::list<interface_t> interfaces;

	protocol_t(const xml_node &protocol_node) {
		name = protocol_node.attribute("name").value();

		int enum_id = 0;

		for (xml_node &interface : protocol_node.children("interface")) {
			interface_t iface;
			iface.destroy_opcode = -1;
			iface.orig_name = interface.attribute("name").value();
			if (iface.orig_name.substr(0, 3) == "wl_") {
				iface.name = iface.orig_name.substr(3, iface.orig_name.size());
			} else {
				iface.name = iface.orig_name;
			}
			iface.client_class = iface.name + "_proxy_t";
			iface.server_class = iface.name + "_resource_t";

			if (interface.attribute("version")) {
				std::string ver_str(interface.attribute("version").value());
				iface.version = std::stoi(ver_str);
			} else {
				iface.version = 1;
			}

			if (interface.child("description")) {
				xml_node description = interface.child("description");
				iface.summary = description.attribute("summary").value();
				iface.description = description.text().get();
			}

			interface_names.push_back(iface.name);

			int opcode = 0; // Opcodes are in order of the XML. (Sadly undocumented)
			for (xml_node &request : interface.children("request")) {
				request_t req;
				req.opcode = opcode++;
				req.name = request.attribute("name").value();

				if (request.attribute("since"))
					req.since = std::stoi(std::string(request.attribute("since").value()));
				else
					req.since = 1;

				if (request.child("description")) {
					xml_node description = request.child("description");
					req.summary = description.attribute("summary").value();
					req.description = description.text().get();
				}

				// destruction takes place through the class destuctor
				if (req.name == "destroy") {
					iface.destroy_opcode = req.opcode;
				}
				for (xml_node &argument : request.children("arg")) {
					argument_t arg;
					arg.type = argument.attribute("type").value();
					arg.name = argument.attribute("name").value();
					if (argument.child("description")) {
						xml_node description = argument.child("description");
						arg.summary = description.attribute("summary").value();
						arg.description = description.text().get();
					}

					if (argument.attribute("interface")) {
						arg.interface = argument.attribute("interface").value();
						if (arg.interface.substr(0, 3) == "wl_")
							arg.interface = arg.interface.substr(3, arg.interface.size());
					}

					if (argument.attribute("enum")) {
						std::string tmp = argument.attribute("enum").value();
						if (tmp.find('.') == std::string::npos) {
							arg.enum_iface = iface.name;
							arg.enum_name = tmp;
						} else {
							arg.enum_iface = tmp.substr(0, tmp.find('.'));
							if (arg.enum_iface.substr(0, 3) == "wl_")
								arg.enum_iface = arg.enum_iface.substr(3, arg.enum_iface.size());
							arg.enum_name = tmp.substr(tmp.find('.') + 1);
						}
					}

					if (argument.attribute("allow-null") && std::string(argument.attribute("allow-null").value()) == "true")
						arg.allow_null = true;
					else
						arg.allow_null = false;

					if (arg.type == "new_id")
						req.ret = arg;
					req.args.push_back(arg);
				}
				iface.requests.push_back(req);
			}

			opcode = 0;

			for (xml_node &event : interface.children("event")) {
				event_t ev;
				ev.opcode = opcode++;
				ev.name = event.attribute("name").value();

				if (event.attribute("since"))
					ev.since = std::stoi(std::string(event.attribute("since").value()));
				else
					ev.since = 1;

				if (event.child("description")) {
					xml_node description = event.child("description");
					ev.summary = description.attribute("summary").value();
					ev.description = description.text().get();
				}

				for (xml_node &argument : event.children("arg")) {
					argument_t arg;
					arg.type = argument.attribute("type").value();
					arg.name = argument.attribute("name").value();
					if (argument.child("description")) {
						xml_node description = argument.child("description");
						arg.summary = description.attribute("summary").value();
						arg.description = description.text().get();
					}

					if (argument.attribute("interface")) {
						arg.interface = argument.attribute("interface").value();
						if (arg.interface.substr(0, 3) == "wl_")
							arg.interface = arg.interface.substr(3, arg.interface.size());
					}

					if (argument.attribute("enum")) {
						std::string tmp = argument.attribute("enum").value();
						if (tmp.find('.') == std::string::npos) {
							arg.enum_iface = iface.name;
							arg.enum_name = tmp;
						} else {
							arg.enum_iface = tmp.substr(0, tmp.find('.'));
							if (arg.enum_iface.substr(0, 3) == "wl_")
								arg.enum_iface = arg.enum_iface.substr(3, arg.enum_iface.size());
							arg.enum_name = tmp.substr(tmp.find('.') + 1);
						}
					}

					if (argument.attribute("allow-null") &&
							std::string(argument.attribute("allow-null").value())
							== "true")
						arg.allow_null = true;
					else
						arg.allow_null = false;

					ev.args.push_back(arg);
				}
				iface.events.push_back(ev);
			}

			for (xml_node &enumeration : interface.children("enum")) {
				enumeration_t enu;
				enu.name = enumeration.attribute("name").value();
				if (enumeration.child("description")) {
					xml_node description = enumeration.child("description");
					enu.summary = description.attribute("summary").value();
					enu.description = description.text().get();
				}

				if (enumeration.attribute("bitfield")) {
					std::string tmp = enumeration.attribute("bitfield").value();
					enu.bitfield = (tmp == "true");
				} else
					enu.bitfield = false;
				enu.id = enum_id++;
				enu.width = 0;

				for (xml_node entry = enumeration.child("entry"); entry;
						entry = entry.next_sibling("entry")) {
					enum_entry_t enum_entry;
					enum_entry.name = entry.attribute("name").value();
					if (enum_entry.name == "default"
							|| isdigit(enum_entry.name.at(0)))
						enum_entry.name.insert(0, 1, '_');
					enum_entry.value = entry.attribute("value").value();
					if (entry.child("description")) {
						xml_node description = entry.child("description");
						enum_entry.summary = description.attribute("summary").value();
						enum_entry.description = description.text().get();
					}

					uint32_t tmp = std::floor(std::log2(stol(enum_entry.value))) + 1;
					if (tmp > enu.width) {
						enu.width = tmp;
					}

					enu.entries.push_back(enum_entry);
				}
				iface.enums.push_back(enu);
			}

			interfaces.push_back(iface);
		}
	}
};

void gen_client_header(std::string filepath,
		const protocol_t &protocol) {
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		std::cerr << "Usage:" << std::endl
		          << "  " << argv[0] << " protocol.xml inc_dir src_dir"
				  //" server_header.hpp client_header.hpp source.cpp"
		          << std::endl;
		return 1;
	}

	xml_document doc;
	doc.load_file(argv[1]);
	xml_node protocol_node = doc.child("protocol");
	protocol_t protocol(protocol_node);

	std::list<interface_t> &interfaces = protocol.interfaces;

	// filenames
	//std::string ext_name(argv[2]);
	std::string inc_dir(argv[2]);
	std::string src_dir(argv[3]);
	std::string server_header_filename = inc_dir + "/" + protocol.name + "-server-protocol.hpp";
	std::string client_header_filename = inc_dir + "/" + protocol.name + "-client-protocol.hpp";
	std::string server_source_filename = src_dir + "/" + protocol.name + "-server-protocol.cpp";
	std::string client_source_filename = src_dir + "/" + protocol.name + "-client-protocol.cpp";
	std::string common_source_filename = src_dir + "/" + protocol.name + "-protocol.cpp";
	//std::string server_header_filename(argv[2]);
	//std::string client_header_filename(argv[3]);
	//std::string source_filename(argv[4]);

	std::fstream wayland_server_hpp(server_header_filename,
	                                std::ios_base::out | std::ios_base::trunc);
	std::fstream wayland_client_hpp(client_header_filename,
	                                std::ios_base::out | std::ios_base::trunc);
	std::fstream wayland_server_cpp(server_source_filename,
	                         std::ios_base::out | std::ios_base::trunc);
	std::fstream wayland_client_cpp(client_source_filename,
	                         std::ios_base::out | std::ios_base::trunc);
	std::fstream wayland_cpp(common_source_filename,
	                         std::ios_base::out | std::ios_base::trunc);

	// header vars
	std::string server_header_guard = protocol.name + "_SERVER_PROTOCOL_HPP";
	std::string client_header_guard = protocol.name + "_CLIENT_PROTOCOL_HPP";
	for (auto & c : server_header_guard) c = toupper(c);
	for (auto & c : client_header_guard) c = toupper(c);

	// client header

	// header intro
	wayland_client_hpp << "#ifndef " << client_header_guard << std::endl
	                   << "#define " << client_header_guard << std::endl
	                   << std::endl
	                   << "#include <array>" << std::endl
	                   << "#include <functional>" << std::endl
	                   << "#include <memory>" << std::endl
	                   << "#include <string>" << std::endl
	                   << "#include <vector>" << std::endl
	                   << "#include <wayland-util.hpp>" << std::endl
	                   << "#include <wayland-client-core.hpp>" << std::endl
	                   << std::endl
	                   //<< "#include <wayland-client.hpp>" << std::endl
	                   << std::endl
	                   << "namespace wayland {" << std::endl
	                   << std::endl;

	// forward declarations
	for (auto &iface : interfaces) {
		wayland_client_hpp << iface.print_forward(CLIENT);
	}
	wayland_client_hpp << std::endl;

	// interface headers
	wayland_client_hpp << "namespace detail {" << std::endl;
	for (auto &iface : interfaces) {
		wayland_client_hpp << iface.print_interface_header();
	}
	wayland_client_hpp  << "}" << std::endl
	                    << std::endl;

	// class declarations
	for (auto &iface : interfaces) {
		wayland_client_hpp << iface.print_header(CLIENT) << std::endl;
	}

	wayland_client_hpp << std::endl
	                   << "}" << std::endl
	                   << std::endl
	                   << "#endif" << std::endl;

	// server header

	// header intro
	wayland_server_hpp << "#ifndef " << server_header_guard << std::endl
	                   << "#define " << server_header_guard << std::endl
	                   << std::endl
	                   << "#include <array>" << std::endl
	                   << "#include <functional>" << std::endl
	                   << "#include <memory>" << std::endl
	                   << "#include <string>" << std::endl
	                   << "#include <vector>" << std::endl
	                   << "#include <wayland-util.hpp>" << std::endl
	                   << "#include <wayland-server-core.hpp>" << std::endl
	                   << std::endl
	                   << std::endl
	                   << "namespace wayland {" << std::endl
	                   << std::endl;

	// forward declarations
	for (auto &iface : interfaces) {
		wayland_server_hpp << iface.print_forward(SERVER);
	}
	wayland_server_hpp << std::endl;

	// interface headers
	wayland_server_hpp << "namespace detail {" << std::endl;
	for (auto &iface : interfaces) {
		wayland_server_hpp << iface.print_interface_header();
	}
	wayland_server_hpp  << "}" << std::endl
	                    << std::endl;

	// class declarations
	for (auto &iface : interfaces) {
		wayland_server_hpp << iface.print_header(SERVER) << std::endl;
	}

	wayland_server_hpp << std::endl
	                   << "}" << std::endl
	                   << std::endl
	                   << "#endif" << std::endl;

	// source file
	
	// server source
	wayland_server_cpp << "#include <array>" << std::endl
	                   << "#include <functional>" << std::endl
	                   << "#include <memory>" << std::endl
	                   << "#include <string>" << std::endl
	                   << "#include <vector>" << std::endl
	                   << "#include <wayland-server-core.hpp>" << std::endl
	                   << "#include <wayland-server-protocol.hpp>" << std::endl
	                   << std::endl
	                   << std::endl
	                   << "namespace wayland {" << std::endl
	                   << std::endl
					   << "using namespace detail;" << std::endl
	                   << std::endl;

	// class member function definitions
	for (auto &iface : interfaces) {
		wayland_server_cpp << iface.print_memdef(SERVER) << std::endl;
	}

	wayland_server_cpp << std::endl
	                   << "}" << std::endl
	                   << std::endl;

	// client source
	
	wayland_client_cpp << "#include <array>" << std::endl
	                   << "#include <functional>" << std::endl
	                   << "#include <memory>" << std::endl
	                   << "#include <string>" << std::endl
	                   << "#include <vector>" << std::endl
	                   << "#include <wayland-client-core.hpp>" << std::endl
	                   << "#include <wayland-client-protocol.hpp>" << std::endl
	                   << std::endl
	                   << std::endl
	                   << "namespace wayland {" << std::endl
	                   << std::endl
					   << "using namespace detail;" << std::endl
	                   << std::endl;

	// class member function definitions
	for (auto &iface : interfaces) {
		wayland_client_cpp << iface.print_memdef(CLIENT) << std::endl;
	}

	wayland_client_cpp << std::endl
	                   << "}" << std::endl
	                   << std::endl;

	// source intro
	wayland_cpp << "#include <wayland-client-protocol.hpp>" << std::endl
	            << std::endl
	            << "using namespace wayland;" << std::endl
	            << "using namespace detail;" << std::endl
	            << std::endl;

	// interface bodys
	for (auto &iface : interfaces)
		wayland_cpp << iface.print_common_defs();

	// class member definitions
	//for (auto &iface : interfaces)
	//	if (iface.name != "display")
	//		wayland_cpp << iface.print_body() << std::endl;
	//wayland_cpp << std::endl;

	// clean up
	wayland_client_hpp.close();
	wayland_server_hpp.close();
	wayland_client_cpp.close();
	wayland_server_cpp.close();
	wayland_cpp.close();

	return 0;
}
