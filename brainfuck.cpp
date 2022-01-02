#define INCLUDE_ARGUMENT
#include "utils.h"
#include <vector>

class Program {
private:
	std::vector<unsigned char> memory;

	size_t pc = 0, program_index_of_loop_end;
	void point_initialize();
	bool process(const char *source_code, size_t program_index);
public:
	void brainfuck(const char *source_code);
};

void Program::point_initialize() {
	while(memory.size() <= pc) {
		memory.push_back(0);
	}
}

bool Program::process(const char *source_code, size_t program_index) {
	for(; source_code[program_index]; ++program_index) {
		char c = source_code[program_index];
		switch(c) {
			case '+':
				point_initialize();
				++memory[pc];
				break;
			case '-':
				point_initialize();
				--memory[pc];
				break;
			case '>':
				++pc;
				break;
			case '<':
				if(pc == 0) {
					return false;
				}
				--pc;
				break;
			case '.':
				point_initialize();
				putchar(memory[pc]);
				fflush(stdout);
				break;
			case '!':
				point_initialize();
				memory[pc] = getch();
				break;
			case '[':
				point_initialize();
				if(memory[pc] == 0) {
					size_t loop_depth = 1;
					for(size_t i = program_index + 1;; ++i) {
						if(source_code[i] == '[') {
							++loop_depth;
						} else if(source_code[i] == ']') {
							--loop_depth;
							if(loop_depth == 0) {
								program_index = i;
								break;
							}
						}
					}
					continue;
				}
				while(memory[pc]) {
					process(source_code, program_index + 1);
				}
				program_index = program_index_of_loop_end;
				break;
			case ']':
				program_index_of_loop_end = program_index;
				return true;
				break;
		}
	}
	return true;
}
void Program::brainfuck(const char *source_code) {
	size_t loop_depth = 0;
	for(size_t i = 0; source_code[i]; ++i) {
		if(source_code[i] == '[') {
			++loop_depth;
		} else if(source_code[i] == ']') {
			--loop_depth;
		}
	}
	if(loop_depth != 0) {
		log_error("ERROR");
		exit(-1);
	}
	if(!process(source_code, 0)) {
		log_error("ERROR");
		exit(-2);
	}
}

void end(int code) {
	exit(code);
}
std::string source_code;

void process_argument(int argc, char **argv) {
	ArgumentProcessor ap;

	Argument help, file, text;
	help.add_name("-h").add_name("--help");
	help.set_argc(0);
	help.set_called_limit(1);
	help.set_description("Display this help and exit");
	help.set_act_func([&ap](char **argv) {
		ap.output_help({argv[0], ": Brainfuck"});
		exit(0);
	});

	text.add_name("-t").add_name("--text");
	text.set_argc(1);
	text.set_description("Specify source code");
	text.set_act_func([](char **argv) {
		source_code += argv[0];
	});

	ap.register_argument(help);
	ap.register_argument(file);
	ap.register_argument(text);
	ap.set_default_argument(text);

	if(!ap.process(argc, argv)) {
		end(1);
	}
}

int main(int argc, char **argv) {
	process_argument(argc, argv);
	if(source_code.empty()) {
		source_code = "++++++++++[>++++++++++<-]>>++++[>+++++++++++<-]<++++.---.+++++++..+++.>>.------------.<<++++++++.--------.+++.------.--------.>++++++++++.";
	} else if(source_code == "-") {
		char c;
		source_code.clear();
		while((c = getchar()) != static_cast<char>(EOF)) {
			source_code.push_back(c);
		}
	}
	Program p;
	p.brainfuck(source_code.c_str());
}
