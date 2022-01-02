#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <string>

#define INCLUDE_ARGUMENT
#include <utils.h>

#include <fmt/printf.h>
namespace std {using namespace fmt;}

std::string output_filename;
uint64_t size_bytes = 0;

bool process_argument(int argc, char **argv) {
	ArgumentProcessor ap;

	Argument help, output, size;

	help.add_name("-h").add_name("--help");
	help.set_argc(0);
	help.set_description("Display this help and exit.");
	help.set_act_func([&ap](char **argv) {
		ap.output_help({argv[0], ": Create a sized empty file(filled with 0).\n"});
		exit(0);
	});

	output.add_name("-f").add_name("--file");
	output.set_argc(1);
	output.set_description("Specify path string to output file");
	output.set_called_limit(1);
	output.set_act_func([](char **argv) {
		output_filename = argv[0];
	});

	size.add_name("-s").add_name("--size");
	size.set_argc(1);
	size.set_description("Specify the size of the file. ([NUM][(K|KiB)|(M|MiB)|(G|GiB) or not explicit specified(byte)])");
	size.set_called_limit(1);
	size.set_act_func([](char **argv) {
		const char *size_str = argv[0];
		std::string_view num_part;
		std::string unit_part;
		
		{//test the string and seperate the string
			bool failed = false;
			enum {num, unit} status = num;
			if(isdigit(size_str[0])) for(const char *p = size_str; *p; ++p) {
				char c = *p;
				if(status == num) {
					if(!isdigit(c)) {
						if(isalpha(c)) {
							status = unit;
							num_part = std::string_view(size_str, p - size_str);
							unit_part = p;
						} else {
							failed = true;
							break;
						}
					}
				} else if(status == unit) {
					if(!isalpha(c)) {
						failed = true;
						break;
					}
				}
			} else failed = true;
			if(failed) {
				log_error("Not a legal size string: \"%s\".", argv[0]);
				exit(1);
			}
		}
		if(num_part.size() == 0 && unit_part.size() == 0) {
			num_part = std::string_view(size_str);
		}

		size_t size = std::strtoul(num_part.data(), nullptr, 10);
		std::transform(unit_part.begin(), unit_part.end(), unit_part.begin(), [](char c) -> char {return tolower(c);});
		if(unit_part.size() == 0) {
			size_bytes = size;
		} else if(unit_part == "k" || unit_part == "kib") {
			size_bytes = size * 1024;
		} else if(unit_part == "m" || unit_part == "mib") {
			size_bytes = size * 1024 * 1024;
		} else if(unit_part == "g" || unit_part == "gib") {
			size_bytes = size * 1024 * 1024 * 1024;
		} else {
			log_error("Unknown unit: \"%s\".", unit_part.data());
			exit(1);
		}
	});

	ap.register_argument(help);
	ap.register_argument(output);
	ap.register_argument(size);

	ap.set_default_argument(output);

	return ap.process(argc, argv);
}

int main(int argc, char **argv) {
	if(!process_argument(argc, argv)) return 1;

	if(output_filename.empty()) {
		log_error("Missing output filename. Type \"%s --help\" for usage.", argv[0]);
		return 1;
	}

	if(size_bytes == 0) {
		log_error("Please specify the size of file. Type \"%s --help\" for usage.", argv[0]);
		return 1;
	}

	int fd = open(output_filename.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if(fd < 0) {
		perror(argv[0]);
		return errno;
	}
	char c = 0;
	lseek(fd, size_bytes - 1, SEEK_SET);
	if(write(fd, &c, 1) != 1) {
		perror(argv[0]);
		return errno;
	}
	close(fd);
	
	return 0;
}
