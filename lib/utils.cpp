#include "utils.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cxxabi.h>

void log(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	fflush(stderr);
	va_end(args);
}

#ifdef __WIN32
#include <windows.h>
void log_error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	WORD general = info.wAttributes;
	SetConsoleTextAttribute(handle, FOREGROUND_RED);
	vfprintf(stderr, fmt, args);
	SetConsoleTextAttribute(handle, general);
	fprintf(stderr, "\n");
	fflush(stderr);
	va_end(args);
}
#else
void log_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(stderr, "\033[91m\033[1m");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\033[0m\n");
	fflush(stderr);
	va_end(args);
}
#endif


int parse_int(const char *str, bool *success) {
	if(str[0] == '\0') {
		if(success != nullptr) {
			*success = false;
		}
		return 0;
	}
	char *p = (char *)str;
	int result = 0;
	bool minus = false;
	if(str[0] == '-'){
		minus = true;
		p++;
	}
	while(*p) {
		if(('0' > *p) || ('9' < *p)) {
			if(success != nullptr) {
				*success = false;
			}
			return 0;
		}
		result += *p - '0';
		result *= 10;
		p++;
	}
	result /= 10;
	if(minus) {
		result = -result;
	}
	if(success != nullptr) {
		*success = true;
	}
	return result;
}

char32_t utf8to32(const unsigned char *src, int *output_length) {
	int length = 0;
	char32_t result = '\0';
	if((src[0] & 0x80) == 0x0) {
		length = 1;
		result = src[0];
	} else if((src[0] & 0xe0) == 0xc0) {
		length = 2;
		result = ((src[0] & 0x1f) << 6) + (src[1] & 0x3f);
	} else if((src[0] & 0xf0) == 0xe0) {
		length = 3;
		result = ((src[0] & 0x0f) << 12) + ((src[1] & 0x3f) << 6) + (src[2] & 0x3f);
	} else if((src[0] & 0xf8) == 0xf0) {
		length = 4;
		result = ((src[0] & 0x07) << 18) + ((src[1] & 0x3f) << 12) + ((src[2] & 0x3f) << 6) + (src[3] & 0x3f);
	}

	if(output_length != nullptr) {
		*output_length = length;
	}

	return result;
}

char32_t utf8to32(const char *src, int *output_length) {
	return utf8to32((const unsigned char *)src, output_length);
}

std::string demangle(const std::type_info &info) {
	char *p = abi::__cxa_demangle(info.name(), nullptr ,nullptr, nullptr);
	std::string ret(p);
	free(p);
	return ret;
}



#ifdef INCLUDE_FRAMEBUFFER
#include "framebuffer_utils.cpp"
#endif

#ifdef INCLUDE_ARGUMENT
#include "argument_utils.cpp"
#endif
