#include "argument_utils.h"
#include "utils.h"

#include <cstdio>
#include <cstring>
#include <exception>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

using std::map;
using std::string;
using std::string_view;
using std::function;
using std::exception;
using std::initializer_list;

bool Argument::is_name(const char *name_) const {
	for(const string &name : names) {
		if(name == name_) {
			return true;
		}
	}
	return false;
}

bool Argument::is_name(const string &name_) const {
	return is_name(name_.c_str());
}

Argument& Argument::add_name(const string &name_) {
	names.emplace(name_);
	return *this;
}
Argument& Argument::set_argc(size_t argc_) {
	argc = argc_;
	return *this;
}
Argument& Argument::set_act_func(function<void(char**)> act_func_) {
	act_func = std::move(act_func_);
	return *this;
}
Argument& Argument::set_description(const string &description_) {
	description = description_;
	return *this;
}

Argument& Argument::set_called_limit(size_t given_max_called_time) {
	max_called_time = given_max_called_time;
	return *this;
}
void Argument::act(char **argv) const {
	try {
		act_func(argv);
	} catch(const std::bad_function_call &) {}
}

bool operator==(const Argument &lfs, const Argument &rfs) {
	if(lfs.argc != rfs.argc ||
		lfs.max_called_time != rfs.max_called_time ||
		lfs.description != rfs.description ||
		lfs.names != rfs.names) return false;

	const auto &p1 = lfs.act_func.target<Argument::act_func_t>();
	const auto &p2 = rfs.act_func.target<Argument::act_func_t>();
	if(p1 == p2) return true;
	if(p1 && p2) {
		if(*p1 == *p2) return true;
		else return false;
	} else return false;
}

bool ArgumentProcessor::register_argument(const Argument &given_arg) {
	arguments.push_back(given_arg);
	return true;
}
void ArgumentProcessor::set_default_argument(const Argument &specified_arg) {
	int index = 0;
	for(const Argument &arg : arguments) {
		if(specified_arg == arg) {
			default_argument_pointer = &arg;
			return;
		}
		++index;
	}
	struct ArgumentNotFoundException : exception {
		const char *what() const noexcept override {
			return "ArgumentNotFoundException";
		}
	};
	throw ArgumentNotFoundException();
}

bool ArgumentProcessor::process(size_t argc, char **argv) const {
	if(arguments.empty()) {
		return false;
	}
	struct Temp {
		size_t called_time = 0;
		std::vector<size_t> arg_pos;
	};
	std::map<const Argument *, Temp> result;

	auto longest_name  = [](const Argument &arg) -> const char * {
		const string *result_p = nullptr;
		size_t max = 0;
		for(const string &str : arg.names) {
			size_t len = str.size();
			if(len >= max) {
				result_p = &str;
				max = len;
			}
		}
		if(result_p) {
			return result_p -> c_str();
		} else {
			return nullptr;
		}
	};
	auto find_argument = [this](const char *given_name) -> const Argument * {
		for(const Argument &arg : arguments) {
			if(arg.is_name(given_name)) {
				return &arg;
			}
		}
		return nullptr;
	};

	auto process_argument = [argc, argv, &find_argument, &result](const Argument &called_arg,
								   const char *called_arg_name,
								   size_t start_index) -> bool {
		size_t given_argc = 0;
		for(size_t k = start_index; k < argc; k++) {
			if(given_argc == called_arg.argc) {
				break;
			}
			if(strlen(argv[k]) == 0) {
				++given_argc;
				continue;
			}
			if(argv[k][0] == '-') {
				if(!find_argument(argv[k])) {
					given_argc ++;
					continue;
				} else {
					break;
				}
			}
			given_argc ++;
		}
		if(given_argc != called_arg.argc) {
			log_error("Argument \"%s\" requires %d sub parameter%s, but got %d.", called_arg_name, called_arg.argc, called_arg.argc < 2 ? "" : "s", given_argc);
			return false;
		}
		if(result[&called_arg].called_time == called_arg.max_called_time) {
			log_error("The time of argument \"%s\" to be called reaches its limit %d.", called_arg_name, called_arg.max_called_time);
			return false;
		}
		auto &temp = result[&called_arg];
		temp.arg_pos.push_back(start_index);
		++temp.called_time;
		return true;
	};

	for(size_t i = 1; i < argc; ++i) {
		if(strlen(argv[i]) == 0) {
			continue;
		}
		const Argument *find_result = find_argument(argv[i]);
		if(find_result) {
			const Argument &called_arg = *find_result;
			const char *called_arg_name = argv[i];

			if(!process_argument(called_arg, called_arg_name, i + 1)) return false;

			i += called_arg.argc;
		} else {
			if(!default_argument_pointer) {
				if(argv[i][0] == '-') {
					log_error("Unknown argument \"%s\". Type \"%s --help\" for usage.", argv[i], argv[0]);
				} else {
					log_error("Unexcepted \"%s\". Type \"%s --help\" for usage.", argv[i], argv[0]);
				}
				return false;
			}
			const Argument &called_arg = *default_argument_pointer;
			const char *called_arg_name = longest_name(called_arg);
			if(!called_arg_name) {
				called_arg_name = "";
			}

			if(!process_argument(called_arg, called_arg_name, i)) return false;

			i += called_arg.argc - 1;
		}
	}
	for(const auto &arg : arguments) {
		if(result.find(&arg) != result.end() && result[&arg].called_time != 0) {
			const auto &temp = result[&arg];
			for(size_t i = 0; i < temp.called_time; ++i) {
				if(arg.argc == 0) {
					arg.act(argv);
				} else {
					arg.act(argv + temp.arg_pos[i]);
				}
			}
		}
	}
	return true;
}

void ArgumentProcessor::output_help(initializer_list<string_view> head, initializer_list<string_view> tail) const {
	auto output_string_view = [](string_view str) {
		for(char c : str) {
			fputc(c, stderr);
		}
	};
	for(const auto &str : head) {
		output_string_view(str);
	}

	fprintf(stderr, "\nArguments:\n");
	for(const Argument &arg : arguments) {
		fprintf(stderr, "\t");
		bool first = true;
		for(const string &name : arg.names) {
			if(first) {
				first = false;
			} else {
				fprintf(stderr, ", ");
			}
			fprintf(stderr, "%s", name.c_str());
		}
		if(arg.description.empty()) {
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, ":\n\t\t%s\n", arg.description.c_str());
		}
	}

	for(const auto &str : tail) {
		output_string_view(str);
	}
}
