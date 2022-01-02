#include <cstdio>
#include <cstdint>
#include <type_traits>

namespace __print_h {
	template<typename T> struct traits;
	template<> struct traits<short> {using type = short;};
	template<> struct traits<int> {using type = int;};
	template<> struct traits<long> {using type = long;};
	template<> struct traits<long long> {using type = long long;};
	template<> struct traits<unsigned> {using type = unsigned;};
	template<> struct traits<unsigned long> {using type = unsigned long;};
	template<> struct traits<unsigned long long> {using type = unsigned long long;};
	template<> struct traits<float> {using type = float;};
	template<> struct traits<double> {using type = double;};
	template<> struct traits<char> {using type = char;};
	template<> struct traits<const char *> {using type = const char *;};
	template<size_t N> struct traits<char [N]> {using type = char[N];};
	template<> struct traits<std::nullptr_t> {using type = std::nullptr_t;};
	template<typename Tp> struct traits<Tp*> {using type = Tp*;};

	template<typename T>
	concept printable_type = requires(T) {
		typename traits<std::remove_const_t<std::remove_reference_t<T>>>::type;
	};
}

inline void print() {}

inline void print(short i) {
	printf("%hd", i);
}
inline void print(int i) {
	printf("%d", i);
}
inline void print(long i) {
	printf("%ld", i);
}
inline void print(long long i) {
	printf("%lld", i);
}
inline void print(unsigned i) {
	printf("%u", i);
}
inline void print(unsigned long i) {
	printf("%lu", i);
}
inline void print(unsigned long long i) {
	printf("%llu", i);
}
inline void print(float f) {
	printf("%f", f);
}
inline void print(double d) {
	printf("%f", d);
}
inline void print(char c) {
	putchar(c);
}
inline void print(const char *str) {
	printf("%s", str);
}
inline void print(void *ptr) {
	printf("%p", ptr);
}
inline void print(std::nullptr_t ptr) {
	printf("%p", ptr);
}

inline void println() {print('\n');}
void println(auto &&data) {
	print(data);
	print('\n');
}

void print(__print_h::printable_type auto&&ta, __print_h::printable_type auto&&tb, __print_h::printable_type auto&&... args) {
	print(ta);
	print(tb, args...);
}

void println(__print_h::printable_type auto&&... args) {
	print(args...);
	print('\n');
}

inline void printb(uint8_t byte) {
	putchar(byte & 0b10000000 ? '1' : '0');
	putchar(byte & 0b01000000 ? '1' : '0');
	putchar(byte & 0b00100000 ? '1' : '0');
	putchar(byte & 0b00010000 ? '1' : '0');
	putchar(byte & 0b00001000 ? '1' : '0');
	putchar(byte & 0b00000100 ? '1' : '0');
	putchar(byte & 0b00000010 ? '1' : '0');
	putchar(byte & 0b00000001 ? '1' : '0');
}
