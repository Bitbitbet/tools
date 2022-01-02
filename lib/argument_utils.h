#ifndef __ARGUMENT_UTILS_H__
#define __ARGUMENT_UTILS_H__

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <list>
#include <string>
#include <string_view>
#include <unordered_set>

class Argument {
	friend class ArgumentProcessor;
	friend bool operator==(const Argument &, const Argument &);
private:
	constexpr static size_t DEFAULT_MAXIMUM_CALLED_TIME = 1024;
	using act_func_t = void(char **);

	size_t argc = 0;
	size_t max_called_time = DEFAULT_MAXIMUM_CALLED_TIME;
	std::function<act_func_t> act_func;

	std::unordered_set<std::string> names;
	std::string description;

	void act(char **argv) const;
public:
	Argument() = default;
	Argument(const Argument &arg_) = default;
	Argument(Argument &&arg_) = default;

	bool is_name(const char *) const;
	bool is_name(const std::string &) const;

	Argument& add_name(const std::string &);
	Argument& set_argc(size_t);
	Argument& set_act_func(std::function<void(char**)>);
	Argument& set_description(const std::string &description_);
	Argument& set_called_limit(size_t);
};

class ArgumentProcessor {
	constexpr static size_t npos = -1;
private:
	std::list<Argument> arguments;
	const Argument *default_argument_pointer = nullptr;
public:
	bool register_argument(const Argument &given_arg);
	void set_default_argument(const Argument &specified_arg);

	bool process(size_t argc, char **argv) const;
	void output_help(std::initializer_list<std::string_view> head = {}, std::initializer_list<std::string_view> tail = {}) const;
};

#endif
