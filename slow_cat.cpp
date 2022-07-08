#define INCLUDE_ARGUMENT
#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <vector>

std::vector<const char *> filenames;
int filename_count = 0;
int delay_time = 1000;

void delay_us(int us) {
	std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void process_argument(int argc, char **argv) {
	ArgumentProcessor ap;
	Argument help, file, set_delay_time;

	help.add_name("-h").add_name("--help");
	help.set_argc(0);
	help.set_description("Display this help and exit");
	help.set_act_func([&ap](char **argv) {
		ap.output_help({argv[0], ": Display a file slowly.\nUsage: ", argv[0], " [FILENAME] [ARGUMENT]...\nWith no file, read standard input."});
		exit(0);
	});
	ap.register_argument(help);

	file.add_name("-f").add_name("--file");
	file.set_argc(1);
	file.set_description("Specify file to display");
	file.set_act_func([](char **argv) {
		filenames.push_back(argv[0]);
	});
	ap.register_argument(file);

	set_delay_time.add_name("-t").add_name("--delay-time");
	set_delay_time.set_argc(1);
	set_delay_time.set_description("Specify how long to delay between characters(Default 1000us)(us)");
	set_delay_time.set_act_func([](char **argv) {
		bool success;
		delay_time = parse_int(argv[0], &success);
		if(!success) {
			log_error("The time should be an integer, but got %s", argv[0]);
			exit(1);
		}
		if(delay_time < 0) {
			log_error("The time to delay is to small: %d", delay_time);
			exit(1);
		}
	});
	ap.register_argument(set_delay_time);

	ap.set_default_argument(file);
	if(!ap.process(argc, argv)) {
		exit(0);
	}
}

int main(int argc, char **argv) {
	process_argument(argc, argv);
	std::vector<FILE*> files;
	files.reserve(filenames.size());
	for(const char *filename : filenames) {
		files.push_back(fopen(filename, "r"));
		if(!files.back()) {
			log_error("Can't open file: \"%s\"", filename);
			exit(1);
		}
	}
	if(filenames.empty()) {
		files.push_back(stdin);
	}
	for(FILE *f : files) {
		int got;
		while((got = fgetc(f)) != EOF) {
			char c = got;
			putchar(c);
			fflush(stdout);
			delay_us(delay_time);
		}
		fclose(f);
	}
}
