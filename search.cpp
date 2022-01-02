#include <filesystem>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#define INCLUDE_ARGUMENT
#include "utils.h"

using namespace std::filesystem;
using std::string;
using std::vector;
using std::pair;

vector<string> root_pathnames;
string content;

const int RED = 0;
const int NORMAL = 1;

void change_color(int color) {
	switch(color) {
		case RED:
			printf("\033[91m\033[1m");
			break;
		case NORMAL:
			printf("\033[0m");
			break;
	}
}

class SearchResult {
private:
	using size_type = string::size_type;
	vector<pair<size_type, size_type>> positions;
public:
	SearchResult() = default;
	SearchResult(const SearchResult &) = default;
	SearchResult(SearchResult &&) = default;

	string text;
	size_type line;

	void reset() {
		text = "";
		line = 0;
		positions.erase(positions.begin(), positions.end());
	}
	bool search() {
		size_type first, pos = 0;
		while((first = text.find(content, pos)) != string::npos) {
			positions.push_back({first, first + content.size()});
			pos = first + 1;
		}
		if(positions.empty()) {
			return false;
		}
		return true;
	}
	void print() const {
		printf("%ld: ", line);
		size_type index = 0;
		for(char c : text) {
			for(int i = 0; i < positions.size(); ++i) {
				if(index == positions[i].first) {
					change_color(RED);
				} else if(index == positions[i].second) {
					change_color(NORMAL);
				}
			}
			putchar(c);

			++index;
		}
		change_color(NORMAL);
		putchar('\n');
	}
};

void end(int code) {
	exit(code);
}

void process_argument(int argc,char **argv) {
	static ArgumentProcessor *ap;
	ap = new ArgumentProcessor;

	Argument help, path, arg_content;

	help.add_name("-h").add_name("--help");
	help.set_argc(0);
	help.set_called_limit(1);
	help.set_description("Display this help and exit.");
	help.set_act_func([](char **argv) {
		ap->output_help({argv[0], ": Search content in specified context.\n"});
		end(0);
	});

	path.add_name("-p").add_name("-f").add_name("--path").add_name("--file");
	path.set_argc(1);
	path.set_description("[PATH] Specify path(file or directory) as context.");
	path.set_act_func([](char **argv) {
		root_pathnames.push_back(argv[0]);
	});

	arg_content.add_name("--content");
	arg_content.set_argc(1);
	arg_content.set_description("[CONTENT] Set content to search.");
	arg_content.set_act_func([](char **argv) {
		static bool first = true;
		if(first) {
			first = false;
		} else {
			content.push_back(' ');
		}
		content.append(argv[0]);
	});
	

	ap->register_argument(help);
	ap->register_argument(path);
	ap->register_argument(arg_content);
	ap->set_default_argument(arg_content);
	if(!ap->process(argc, argv)) {
		exit(-1);
	}
	delete ap;
}

void search_file(std::istream &&is, const string &filename) {
	size_t line = 0;
	char c;
	SearchResult sr;
	vector<SearchResult> results;
	while(c = is.get(), is) {
		if(c == '\n') {
			if(sr.search()) {
				sr.line = line;
				results.push_back(sr);
			}

			++line;
			sr.reset();
			continue;
		}
		sr.text.push_back(c);
	}
	if(!results.empty()) {
		printf("\n%s:\n", filename.c_str());
		for(const SearchResult &result : results) {
			result.print();
		}
	}
}

void search(const path &p) {
	directory_entry entry(p);

	file_type type = entry.status().type();
	try {
		if(type == file_type::directory) {
			directory_iterator iter(p);
			for(auto file : iter) {
				search(file.path());
			}
		} else if(type == file_type::regular) {
			search_file(std::ifstream(p), p.string());
		}
	} catch(filesystem_error fe) {
		log_error("Can't open %s %s", type == file_type::directory ? "directory" : "file", p.string().c_str());
	}
}

int main(int argc,char **argv) {
	process_argument(argc, argv);
	if(content.empty()) {
		log_error("Missing content. Type \"%s --help\" for more information.", argv[0]);
		end(-1);
	}
	if(root_pathnames.empty()) {
		log_error("A path is required. Type \"%s --help\" for more information.", argv[0]);
		end(-1);
	}

	for(const string &root_pathname : root_pathnames) {
		path p(root_pathname);
		if(!exists(p)) {
			log_error("Path \"%s\" does not exist. Type \"%s --help\" for more information.", root_pathname.c_str(), argv[0]);
			end(-1);
		}

		search(p);
	}
}
