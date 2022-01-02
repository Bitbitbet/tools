#define INCLUDE_ARGUMENT
#include "utils.h"

#include <cstring>
#include <cstdlib>
#include <cassert>

#include <memory>
#include <chrono>
#include <thread>
#include <random>
#include <iostream>
#include <sstream>
#include <mutex>
#include <shared_mutex>
#include <limits>
#include <vector>

using std::initializer_list;
using std::unique_ptr;
using std::thread;
using std::cout;
using std::endl;
using std::flush;
using std::cin;
using std::ostream;
using std::ostringstream;
using std::shared_mutex;
using std::unique_lock;
using std::numeric_limits;
using std::vector;


inline void end(int status) {
	exit(status);
}
using std::this_thread::yield;
inline void delay(uint32_t ms) {
	 std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
inline void delay_us(uint32_t ms) {
	 std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

struct Coord {
	using coord_type = int32_t;
	coord_type x, y;
	constexpr Coord() : x(0), y(0) {}
	constexpr Coord(coord_type x_, coord_type y_) : x(x_), y(y_) {}
};
constexpr Coord operator+(const Coord &a, const Coord &b) {
	return Coord{a.x + b.x, a.y + b.y};
}

struct Area {
	using size_type = uint32_t;
	size_type width, height;

	constexpr Area() : width(0), height(0) {}
	constexpr Area(size_t w, size_t h) : width(w), height(h) {}
	constexpr size_type size() const {
		return width * height;
	}
};

constexpr bool operator==(const Area &a, const Area &b) {
	return a.width == b.width && a.height == b.height;
}

const Area DEFAULT_MAP_SIZE = {10, 20};
const Area MINIMUM_MAP_SIZE = {10, 10};
const Area::size_type BUFFER_AREA_HEIGHT = 4;
const uint32_t DEFAULT_DELAY_TIME = 500;

Area map_size = DEFAULT_MAP_SIZE;

enum class FieldEnum : int8_t {
	empty = -2,
	field = -1,//SIGN
	L = 0,//RED
	I,//GREEN
	O,//ORANGE
	S,//BLUE
	T,//PURPLE
	J,//CYAN
	Z,//YELLOW
	field_end//SIGN
};
enum class Key : uint8_t {
	DOWN, LEFT, RIGHT, ROTATE, SKIP, QUIT, /*Shouldn't be passed to the user*/NOT_ARROW_KEY, PART_OF_ARROW_KEY
};

inline namespace OperatorSystemRelativedCode {
#ifdef __WIN32
#include <windows.h>
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord_saved = {0, 0};

	struct ArrowKeyPraser {
		uint8_t arrow_key_level = 0;
		Key operator()(unsigned char c) {
			if(c == 224 && arrow_key_level == 0) {//WINDOWS CONSOLE ARROW KEY
				arrow_key_level = 1;
				return Key::PART_OF_ARROW_KEY;
			} else if(('H' == c || 'P' == c || 'K' == c || 'M' == c) && arrow_key_level == 1) {
				arrow_key_level = 0;
				switch(c) {
					case 'H':
						return Key::ROTATE;
					case 'P':
						return Key::DOWN;
					case 'M':
						return Key::RIGHT;
					case 'K':
						return Key::LEFT;
				}
			} else {
				arrow_key_level = 0;
				return Key::NOT_ARROW_KEY;
			}
			return Key::NOT_ARROW_KEY;
		}
	};
	void output_field(FieldEnum field) {//WINDOWS CONSOLE ONLY
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		WORD general = info.wAttributes;

		switch(field) {
			case FieldEnum::L:
				SetConsoleTextAttribute(handle, BACKGROUND_RED);//RED
				cout << "  " << flush;
				break;
			case FieldEnum::I:
				SetConsoleTextAttribute(handle, BACKGROUND_GREEN);//GREEN
				cout << "  " << flush;
				break;
			case FieldEnum::O:
				SetConsoleTextAttribute(handle, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY);//ORANGE
				cout << "  " << flush;
				break;
			case FieldEnum::S:
				SetConsoleTextAttribute(handle, BACKGROUND_BLUE);//BLUE
				cout << "  " << flush;
				break;
			case FieldEnum::T:
				SetConsoleTextAttribute(handle, BACKGROUND_RED | BACKGROUND_BLUE);//PURPLE
				cout << "  " << flush;
				break;
			case FieldEnum::J:
				SetConsoleTextAttribute(handle, BACKGROUND_GREEN | BACKGROUND_BLUE);//CYAN
				cout << "  " << flush;
				break;
			case FieldEnum::Z:
				SetConsoleTextAttribute(handle, BACKGROUND_RED | BACKGROUND_GREEN);//YELLOW
				cout << "  " << flush;
				break;
			default:
				cout << " .";
				return;
		}
		SetConsoleTextAttribute(handle, general);
	}
	void output_reset_cursor() {//WINDOWS CONSOLE ONLY
		SetConsoleCursorPosition(handle, {0, 0});
	}
	void output_clear() {//WINDOWS CONSOLE ONLY
		static bool first = true;
		if(first) {
			system("cls");
			first  = false;
		} else output_reset_cursor();
	}
	void gotoxy(short x, short y) {
		SetConsoleCursorPosition(handle, {x, y});
	}
	void cursor_pos_save() {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		coord_saved = info.dwCursorPosition;
	}
	void cursor_pos_load() {
		SetConsoleCursorPosition(handle, coord_saved);
	}
	void cursor_down(int32_t y) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		COORD c = info.dwCursorPosition;
		c.Y += y;
		SetConsoleCursorPosition(handle, c);
	}
	void cursor_right(int32_t x) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		COORD c = info.dwCursorPosition;
		c.X += x;
		SetConsoleCursorPosition(handle, c);
	}
#else
	struct ArrowKeyPraser {
		uint8_t arrow_key_level = 0;
		Key operator()(unsigned char c) {
			if(c == '\033' && arrow_key_level == 0) {//UNIX CONSOLE ARROW KEY
				arrow_key_level = 1;
				return Key::PART_OF_ARROW_KEY;
			} else if(c == '\x5B' && arrow_key_level == 1) {
				arrow_key_level = 2;
				return Key::PART_OF_ARROW_KEY;
			} else if('A' <= c && 'D' >= c && arrow_key_level == 2) {
				arrow_key_level = 0;
				switch(c) {
					case 'A':
						return Key::ROTATE;
					case 'B':
						return Key::DOWN;
					case 'C':
						return Key::RIGHT;
					case 'D':
						return Key::LEFT;
				}
			} else {
				arrow_key_level = 0;
				return Key::NOT_ARROW_KEY;
			}
			return Key::NOT_ARROW_KEY;
		}
	};
	void output_field(FieldEnum field) {//UNIX CONSOLE ONLY
		switch(field) {
			case FieldEnum::L:
				cout << "\033[41m  \033[0m";
				break;
			case FieldEnum::I:
				cout << "\033[42m  \033[0m";
				break;
			case FieldEnum::O:
				cout << "\033[43m  \033[0m";
				break;
			case FieldEnum::S:
				cout << "\033[44m  \033[0m";
				break;
			case FieldEnum::T:
				cout << "\033[45m  \033[0m";
				break;
			case FieldEnum::J:
				cout << "\033[46m  \033[0m";
				break;
			case FieldEnum::Z:
				cout << "\033[103m  \033[0m";
				break;
			default:
				cout << " .";
		}
	}
	void output_reset_cursor() {//UNIX CONSOLE ONLY
		cout << "\033[0;0H" << flush;
	}
	void output_clear() {//UNIX CONSOLE ONLY
		output_reset_cursor();
		cout << "\033[2J" << flush;
	}
	void gotoxy(size_t x, size_t y) {
		cout << "\033[" << y + 1 << ";" << x + 1 << "H" << flush;
	}
	void cursor_pos_save() {
		cout << "\033[s" << flush;
	}
	void cursor_pos_load() {
		cout << "\033[u" << flush;
	}
	void cursor_down(int32_t y) {
		if(y > 0) {
			cout << "\033[" << y << "B" << flush;
		} else if(y < 0) {
			cout << "\033[" << -y << "A" << flush;
		}
	}
	void cursor_right(int32_t x) {
		if(x > 0) {
			cout << "\033[" << x << "C" << flush;
		} else if(x < 0) {
			cout << "\033[" << -x << "D" << flush;
		}
	}
#endif
}

class KeyEvent {
private:
	thread listener_thread;
	using listener_type = std::function<void (Key)>;
	listener_type listener_func;
	volatile bool running;

	void listener_thread_func() {
		unsigned char c;
		auto charget = [] {return cin.get();};

		ArrowKeyPraser arrow_key_praser;
		while(cin) {
			if(!running) {
				return;
			}
			c = getch(charget);
			c = toupper(c);
			if(!running) {
				return;
			}

			Key result = arrow_key_praser(c);
			if(result != Key::NOT_ARROW_KEY) {
				if(result != Key::PART_OF_ARROW_KEY) {
					listener_func(result);
				}
				continue;
			}

			switch(c) {
				case 'W': case 'K':
					listener_func(Key::ROTATE);
					break;
				case 'S': case 'J':
					listener_func(Key::DOWN);
					break;
				case 'D': case 'L':
					listener_func(Key::RIGHT);
					break;
				case 'A': case 'H':
					listener_func(Key::LEFT);
					break;
				case 'Q':
					listener_func(Key::QUIT);
					break;
				case ' ':
					listener_func(Key::SKIP);
					break;
			}
		}
	}
public:
	KeyEvent() : running(false) {}
	~KeyEvent() {
		stop();
		if(listener_thread.joinable()) {
			listener_thread.join();
		}
	}
	void set_key_listener(listener_type listener_) {
		listener_func.swap(listener_);
	}
	void start() {
		assert(listener_func);
		assert(!listener_thread.joinable());
		running = true;
		listener_thread = thread(&KeyEvent::listener_thread_func, this);
	}
	void stop() {
		running = false;
	}
};

class Map {
public:
	Map(Area area_) : m_area(area_), data(new FieldEnum[area_.size()]), enable_minus_data(false) {
		memset(data.get(), static_cast<int>(FieldEnum::empty), sizeof(FieldEnum) * m_area.size());
	}

	Map(Area area_, Area::size_type buffer_height) :
		m_area(area_),
		data(new FieldEnum[area_.size()]),
		m_minus_area{area_.width, buffer_height},
		minus_data(new FieldEnum[m_minus_area.size()]),
		enable_minus_data(true) {
		clear();
	}

	Map(const Map &map) : m_area(map.m_area), m_minus_area(map.m_minus_area), enable_minus_data(map.enable_minus_data) {
		data.reset(new FieldEnum[m_area.size()]);
		if(enable_minus_data) {
			minus_data.reset(new FieldEnum[m_minus_area.size()]);
		}
		copy(map);
	}

	Map(Map &&map) :
	m_area(map.m_area),
	data(std::move(map.data)),
	m_minus_area(map.m_minus_area),
	minus_data(std::move(map.minus_data)),
	enable_minus_data(map.enable_minus_data) {}

	~Map() = default;

	Map &operator=(const Map &map) & {
		if(map.m_area == m_area && map.m_minus_area == m_minus_area && map.enable_minus_data == enable_minus_data) {
			copy(map);
			return *this;
		}
		m_area = map.m_area;
		m_minus_area = map.m_minus_area;
		enable_minus_data = map.enable_minus_data;
		data.reset(new FieldEnum[m_area.size()]);
		if(enable_minus_data) {
			minus_data.reset(new FieldEnum[m_minus_area.size()]);
		}
		copy(map);
		return *this;
	}
	Map &operator=(Map &&map) & {
		swap(map);
		return *this;
	}

	void clear() {
		memset(data.get(), static_cast<int>(FieldEnum::empty), sizeof(FieldEnum) * m_area.size());
		if(enable_minus_data) {
			memset(minus_data.get(), static_cast<int>(FieldEnum::empty), sizeof(FieldEnum) * m_minus_area.size());
		}
	}
	FieldEnum &operator[](const Coord &coord) {
		return get(coord);
	}
	const FieldEnum &operator[](const Coord &coord) const {
		return static_cast<const FieldEnum &>(get(coord));
	}

	bool vaild(const Coord &coord) const {
		if(coord.x < 0 || coord.x >= m_area.width) return false;
		if(enable_minus_data && coord.y < 0 && -coord.y <= m_minus_area.height) {
			return true;
		} else if(coord.y >= 0 && coord.y < m_area.height) {
			return true;
		} else {
			return false;
		}
	}
	Area area() const {
		return m_area;
	}
	Area minus_area() const {
		assert(enable_minus_data);
		return m_minus_area;
	}
	void swap(Map &map) {
		std::swap(m_area, map.m_area);
		std::swap(m_minus_area, map.m_minus_area);
		std::swap(enable_minus_data, map.enable_minus_data);
		data.swap(map.data);
		minus_data.swap(map.minus_data);
	}

private:
	Area m_area;
	unique_ptr<FieldEnum[]> data;
	Area m_minus_area;
	unique_ptr<FieldEnum[]> minus_data;
	bool enable_minus_data;

	FieldEnum &get(const Coord &coord) const {
		assert(vaild(coord));
		if(coord.y < 0) {
			return minus_data[coord.x + (-coord.y - 1) * m_minus_area.width];
		}
		return data[coord.x + coord.y * m_area.width];
	}
	void copy(const Map &map) {
		assert(map.m_area == m_area && map.enable_minus_data == enable_minus_data && map.m_minus_area == m_minus_area);
		memcpy(data.get(), map.data.get(), sizeof(FieldEnum) * m_area.size());
		if(enable_minus_data) {
			memcpy(minus_data.get(), map.minus_data.get(), sizeof(FieldEnum) * m_minus_area.size());
		}
	}
};

class Tetromino {
private:
	constexpr static size_t UNITS_COUNT = 4;
	constexpr static size_t MAXIMUM_SHAPES_COUNT = 4;
public:
	constexpr Tetromino(FieldEnum fe, initializer_list<initializer_list<Coord>> ilst) : tetromino(fe) {
		shape_count = ilst.size();
		auto iter = ilst.begin();
		for(size_t shapeno = 0; shapeno < shape_count; ++shapeno) {
			auto iterator = iter->begin();
			for(size_t unit = 0; unit < UNITS_COUNT; ++unit) {
				shapes[shapeno][unit] = *iterator;
				++iterator;
			}
			++iter;
		}
	}

	constexpr Coord::coord_type left(const size_t shapeno) const {
		assert(shapeno < shape_count);
		Coord::coord_type result = numeric_limits<Coord::coord_type>::max();
		for(size_t i = 0; i < UNITS_COUNT; ++i) {
			if(shapes[shapeno][i].x < result) result = shapes[shapeno][i].x;
		}
		return result;
	}
	constexpr Coord::coord_type right(const size_t shapeno) const {
		assert(shapeno < shape_count);
		Coord::coord_type result = numeric_limits<Coord::coord_type>::min();
		for(size_t i = 0; i < UNITS_COUNT; ++i) {
			if(shapes[shapeno][i].x > result) result = shapes[shapeno][i].x;
		}
		return result;
	}
	constexpr Area area(const size_t shapeno) const {
		assert(shapeno < shape_count);
		Coord min{numeric_limits<Coord::coord_type>::max(), numeric_limits<Coord::coord_type>::max()};
		Coord max{numeric_limits<Coord::coord_type>::min(), numeric_limits<Coord::coord_type>::min()};
		for(size_t i = 0; i < UNITS_COUNT; ++i) {
			const Coord coord = shapes[shapeno][i];
			if(coord.x > max.x) max.x = coord.x;
			if(coord.x < min.x) min.x = coord.x;
			if(coord.y > max.y) max.y = coord.y;
			if(coord.y < min.y) min.y = coord.y;
		}
		Area result;
		result.width = max.x - min.x + 1;
		result.height = max.y - min.y + 1;
		return result;
	}
	constexpr size_t next_shapeno(const size_t shapeno) const {
		assert(shapeno < shape_count);
		size_t result = shapeno + 1;
		if(result == shape_count) {
			result = 0;
		}
		return result;
	}
	bool fit(const Map &map, const Coord pos, const size_t shapeno) const {
		assert(shapeno < shape_count);
		for(size_t i = 0; i < UNITS_COUNT; ++i) {
			const Coord c = pos + shapes[shapeno][i];
			if(!map.vaild(c) || map[c] != FieldEnum::empty) {
				return false;
			}
		}
		return true;
	}
	void print(Map &map, const Coord pos, const size_t shapeno) const {
		assert(shapeno < shape_count);
		for(size_t i = 0; i < UNITS_COUNT; ++i) {
			assert(map[pos + shapes[shapeno][i]] == FieldEnum::empty);
			map[pos + shapes[shapeno][i]] = tetromino;
		}
	}
	void remove(Map &map, Coord pos, size_t shapeno) const {
		assert(shapeno < shape_count);
		for(size_t i = 0; i < UNITS_COUNT; ++i) {
			assert(map[pos + shapes[shapeno][i]] != FieldEnum::empty);
			map[pos + shapes[shapeno][i]] = FieldEnum::empty;
		}
	}
private:
	Coord shapes[MAXIMUM_SHAPES_COUNT][UNITS_COUNT];
	size_t shape_count = 0;
	FieldEnum tetromino;
};

class TetrominoManager {
public:
	constexpr TetrominoManager() : tetrominos {
		{FieldEnum::L, {
			{{0, 0}, {1, 0}, {2, 0}, {0, 1}},
			{{0, -1}, {1, -1}, {1, 0}, {1, 1}},
			{{0, 0}, {1, 0}, {2, 0}, {2, -1}},
			{{2, 1}, {1, -1}, {1, 0}, {1, 1}}
		}},
		{FieldEnum::I, {
			{{0, 0}, {1, 0}, {2, 0}, {3, 0}},
			{{1, -2}, {1, -1}, {1, 0}, {1, 1}}
		}},
		{FieldEnum::O, {
			{{0, 0}, {1, 0}, {0, 1}, {1, 1}}
		}},
		{FieldEnum::S, {
			{{0, 1}, {1, 0}, {1, 1}, {2, 0}},
			{{1, 0}, {1, 1}, {2, 1}, {2, 2}}
		}},
		{FieldEnum::T, {
			{{0, 0}, {1, 0}, {2, 0}, {1, 1}},
			{{0, 0}, {1, 0}, {1, -1}, {1, 1}},
			{{0, 0}, {1, 0}, {2, 0}, {1, -1}},
			{{2, 0}, {1, 0}, {1, -1}, {1, 1}},
		}},
		{FieldEnum::J, {
			{{0, 0}, {0, 1}, {1, 1}, {2, 1}},
			{{1, 0}, {2, 0}, {1, 1}, {1, 2}},
			{{2, 2}, {0, 1}, {1, 1}, {2, 1}},
			{{0, 2}, {1, 0}, {1, 1}, {1, 2}}
		}},
		{FieldEnum::Z, {
			{{0, 0}, {1, 0}, {1, 1}, {2, 1}},
			{{2, -1}, {1, 0}, {2, 0}, {1, 1}}
		}},
	} {}
	constexpr const Tetromino &operator[](FieldEnum fe) const {
		return tetrominos[static_cast<size_t>(fe)];
	}
private:
	Tetromino tetrominos[static_cast<size_t>(FieldEnum::field_end)];
};

constexpr TetrominoManager tetrominos;
Map map(map_size, BUFFER_AREA_HEIGHT);
std::shared_mutex mutex;
uint32_t delay_time = DEFAULT_DELAY_TIME;

struct ScreenOutputHelper {
	const Map &map;
	const Coord &controlled_tetromino_pos;
	const FieldEnum &field_now, &field_next;
	const size_t &shapeno, &score;
};

class Screen {//UNIX-STYLE CONSOLE ONLY
public:
	Screen(const ScreenOutputHelper &helper_) :
	running(false) , will_refresh(false), helper(helper_), output_soft_helper(helper.map), output_next_tetromino_helper(FieldEnum::empty) {
		output_clear();
	}
	~Screen() {
		stop();
		if(screen_thread.joinable()) {
			screen_thread.join();
		}
	}
	void start() {
		assert(!running);
		assert(!screen_thread.joinable());

		running = true;
		screen_thread = thread(&Screen::screen_thread_func, this);
	}
	void stop() {
		running = false;
	}
	void refresh() {
		will_refresh = true;
		lock = true;
	}
	void refresh_and_wait() {
		will_refresh = true;
		lock = true;
		while(will_refresh) yield();
	}
	void refresh_unlocked_and_wait() {
		will_refresh = true;
		lock = false;
		while(will_refresh) yield();
	}
private:
	thread screen_thread;
	volatile bool running, will_refresh, lock;
	const ScreenOutputHelper helper;
	Map output_soft_helper, output_next_tetromino_map_helper = {{4, 3}};
	FieldEnum output_next_tetromino_helper;

	void screen_thread_func() {
		bool first = true;
		while(true) {
			while(!will_refresh) {
				if(!running) {
					cout << "You failed. Final score: " << helper.score << ". Press any key to continue..." << endl;
					return;
				}
				delay_us(1);
 			}
			if(first) {
				output_clear();
				if(lock) mutex.lock_shared();
				output_next_tetromino();
				output();
				if(lock) mutex.unlock_shared();
				first = false;
			} else {
				output_reset_cursor();
				if(lock) mutex.lock_shared();
				output_next_tetromino();
				output_soft();
				if(lock) mutex.unlock_shared();
			}
			will_refresh = false;
		}
	}
	void output_soft() {
		const Map &map = helper.map;
		const Area area = map.area();

		cursor_pos_save();
		auto gotoxy = [](Coord coord) {
			cursor_pos_load();
			cursor_right(coord.x * 2 + 1);
			cursor_down(coord.y + 1);
		};
		output_ground();

		for(Coord::coord_type y = 0; y < area.height; ++y) {
			for(Coord::coord_type x = 0; x < area.width; ++x) {
				if(map[{x, y}] != output_soft_helper[{x, y}]) {
					gotoxy({x, y});
					output_field(map[{x, y}]);
					output_soft_helper[{x, y}] = map[{x, y}];
				}
			}
		}
		cursor_pos_load();
		cursor_down(area.height + 1);
		output_ground();
		cout << "Score: " << helper.score << "\n";
	}
	void output() {
		const Map &map = helper.map;
		const Area area = map.area();
		if(false) {
			const Area minus_area = map.minus_area();
			for(size_t i = 0; i < minus_area.width * 2 + 2; ++i) {
				cout << '-';
			}
			cout << '\n';
			for(Coord::coord_type y = -minus_area.height; y < 0; ++y) {
				cout << '|';
				for(Coord::coord_type x = 0; x < minus_area.width; ++x) {
					output_field(map[{x, y}]);
				}
				cout << "|\n";
			}
		}
		output_ground();
		for(Coord::coord_type y = 0; y < area.height; ++y) {
			cout << '|';
			for(Coord::coord_type x = 0; x < area.width; ++x) {
				output_field(map[{x, y}]);
			}
			cout << "|\n";
		}
		output_ground();
		cout << "Score: " << helper.score << "\n";
		output_soft_helper = map;
	}
	void output_ground() const {
		const auto width = helper.map.area().width;
		const auto controlled_tetromino_pos = helper.controlled_tetromino_pos;
		const size_t shapeno = helper.shapeno;
		const Tetromino &tetro = tetrominos[helper.field_now];
		const size_t left = tetro.left(shapeno) + controlled_tetromino_pos.x;
		const size_t right = tetro.right(shapeno) + controlled_tetromino_pos.x;

		cout << '|';
		for(size_t i = 0; i < width; ++i) {
			cout << ((i >= left && i <= right) ? "==" : "--");
		}
		cout << "|\n";
	}
	void output_next_tetromino() {
		if(output_next_tetromino_helper == helper.field_next) {
			cursor_down(output_next_tetromino_map_helper.area().height + 1);//one for top
			return;
		}
		output_next_tetromino_helper = helper.field_next;

		const Tetromino &tetro = tetrominos[helper.field_next];
		const Area map_area = helper.map.area();
		const Area tetro_area = tetro.area(0);
		const Area helper_area = output_next_tetromino_map_helper.area();
		Coord pos;
		pos.x = (helper_area.width - tetro_area.width) / 2;
		pos.y = (helper_area.height - tetro_area.height) / 2;

		output_next_tetromino_map_helper.clear();
		tetro.print(output_next_tetromino_map_helper, pos, 0);
		cursor_right(map_area.width - helper_area.width);
		for(Coord::coord_type i = 0; i < helper_area.width + 1; ++i) {
			cout << "--";
		}
		cout << '\n';
		for(Coord::coord_type y = 0; y < helper_area.height; ++y) {
			cursor_right(map_area.width - helper_area.width);
			cout << "|";
			for(Coord::coord_type x = 0; x < helper_area.width; ++x) {
				output_field(output_next_tetromino_map_helper[{x, y}]);
			}
			cout << "|\n";
		}
	}
};

class Tetris {
private:
	KeyEvent key_event;
	Screen screen;
	volatile bool running;

	FieldEnum field_now, field_next;
	size_t shapeno, score;
	Coord controlled_tetromino_pos;
	bool skip_helper = false;

	void random_field() {
		static std::default_random_engine engine{std::random_device()()};
		static std::uniform_int_distribution<int> range(static_cast<int>(FieldEnum::field) + 1, static_cast<int>(FieldEnum::field_end) - 1);
		static bool first = true;
		if(first) {
			field_next = static_cast<FieldEnum>(range(engine));
			first = false;
		}
		field_now = field_next;
		field_next = static_cast<FieldEnum>(range(engine));
	}
	void check() {
		const Area area = map.area();
		vector<bool> clears(area.height);
		size_t clear_count = 0;
		for(Coord::coord_type y = area.height - 1; y >= 0; --y) {
			bool clear = true;
			for(Coord::coord_type x = 0; x < area.width; ++x) {
				if(map[{x, y}] == FieldEnum::empty) {
					clear = false;
					break;
				}
			}
			if(clear) ++clear_count;
			clears[y] = clear;
		}
		if(clear_count == 0) {
			return;
		}
		switch(clear_count) {
			case 1: score += 10;break;
			case 2: score += 40;break;
			case 3: score += 80;break;
			case 4: score += 160;break;
		}
		for(Coord::coord_type i = 0; i < (area.width + 1) / 2; ++i) {
			for(Coord::coord_type y = 0; y < area.height; ++y) {
				if(clears[y]) {
					Coord a, b;
					a.y = y;
					b.y = y;
					a.x = (area.width - 1) / 2 - i;
					if(area.width % 2 == 0) {
						b.x = area.width / 2 + i;
					} else {
						b.x = (area.width - 1) / 2 + i;
					}
					map[a] = FieldEnum::empty;
					map[b] = FieldEnum::empty;
				}
			}
			screen.refresh_unlocked_and_wait();
			delay(delay_time / area.width);
		}
		Coord::coord_type fall_line = 0;
		for(Coord::coord_type y = area.height - 1; y >= 0; --y) {
			if(clears[y]) {
				++fall_line;
				continue;
			}
			if(fall_line > 0) {
				for(Coord::coord_type x = 0; x < area.width; ++x) {
					map[{x, y + fall_line}] = map[{x, y}];
					map[{x, y}] = FieldEnum::empty;
				}
			}
		}
	}
	void next_tetromino(bool lock = true) {
		if(lock)mutex.lock();
		skip_helper = true;

		check();
		random_field();
		shapeno = 0;
		const Tetromino &tetro = tetrominos[field_now];
		const Area tarea = tetro.area(shapeno);
		assert(tarea.height <= map.minus_area().height);
		controlled_tetromino_pos = {
			static_cast<Coord::coord_type>((map.minus_area().width - tarea.width)/ 2),
			-static_cast<Coord::coord_type>(tarea.height)
		};

		if(!tetro.fit(map, controlled_tetromino_pos, shapeno)) {
			stop();
			if(lock)mutex.unlock();
			return;
		}
		tetro.print(map, controlled_tetromino_pos, shapeno);
		if(lock)mutex.unlock();
	}
	void rotate() {
		unique_lock guard(mutex);
		const Tetromino &tetro = tetrominos[field_now];
		if(shapeno == tetro.next_shapeno(shapeno)) return;
		tetro.remove(map, controlled_tetromino_pos, shapeno);
		size_t temp = tetro.next_shapeno(shapeno);
		if(!tetro.fit(map, controlled_tetromino_pos, temp)) {
			tetro.print(map, controlled_tetromino_pos, shapeno);
			return;
		}
		shapeno = temp;
		tetro.print(map, controlled_tetromino_pos, shapeno);
	}
	void move_down(bool lock = true) {
		if(lock) mutex.lock();
		const Tetromino &tetro = tetrominos[field_now];
		tetro.remove(map, controlled_tetromino_pos, shapeno);
		++controlled_tetromino_pos.y;
		if(!tetro.fit(map, controlled_tetromino_pos, shapeno)) {
			--controlled_tetromino_pos.y;
			tetro.print(map, controlled_tetromino_pos, shapeno);
			next_tetromino(false);	//disable lock
			if(lock) mutex.unlock();
			return;
		}
		tetro.print(map, controlled_tetromino_pos, shapeno);
		if(lock) mutex.unlock();
	}
	void move_left() {
		unique_lock guard(mutex);
		const Tetromino &tetro = tetrominos[field_now];
		tetro.remove(map, controlled_tetromino_pos, shapeno);
		--controlled_tetromino_pos.x;
		if(!tetro.fit(map, controlled_tetromino_pos, shapeno)) {
			++controlled_tetromino_pos.x;
			tetro.print(map, controlled_tetromino_pos, shapeno);
			return;
		}
		tetro.print(map, controlled_tetromino_pos, shapeno);
	}
	void move_right() {
		unique_lock guard(mutex);
		const Tetromino &tetro = tetrominos[field_now];
		tetro.remove(map, controlled_tetromino_pos, shapeno);
		++controlled_tetromino_pos.x;
		if(!tetro.fit(map, controlled_tetromino_pos, shapeno)) {
			--controlled_tetromino_pos.x;
			tetro.print(map, controlled_tetromino_pos, shapeno);
			return;
		}
		tetro.print(map, controlled_tetromino_pos, shapeno);
	}
	void skip() {
		unique_lock guard(mutex);
		skip_helper = false;
		while(!skip_helper) {
			move_down(false);	//disable lock
		}
		skip_helper = false;
	}
public:
	Tetris() : screen({map, controlled_tetromino_pos, field_now, field_next, shapeno, score}) {
		key_event.set_key_listener([this](Key key) {
			switch(key) {
				case Key::ROTATE:
					rotate();
					break;
				case Key::LEFT:
					move_left();
					break;
				case Key::RIGHT:
					move_right();
					break;
				case Key::DOWN:
					move_down();
					break;
				case Key::SKIP:
					skip();
					break;
				case Key::QUIT:
					stop();
					break;
				default:
					break;
			}
			screen.refresh();
		});
	}
	~Tetris() {stop();}
	void start() {
		running = true;
		key_event.start();
		screen.start();
		next_tetromino();
		screen.refresh();
		score = 0;
		while(true) {
			delay(delay_time);
			if(!running) {
				return;
			}

			move_down();
			screen.refresh();
		}
	}
	void stop() {
		screen.stop();
		key_event.stop();
		running = false;
	}
};



bool process_argument(int argc, char **argv) {
	ArgumentProcessor ap;

	Argument help, map_size_arg, delay_time_arg;

	help.add_name("-h").add_name("--help");
	help.set_argc(0);
	help.set_description("Display this help and exit.");
	help.set_act_func([&ap](char **argv) {
		ap.output_help({
			argv[0],
			": Console tetris game.\n",
			"Press \"LEFT\" \"DOWN\" \"RIGHT\" or \"A\" \"S\" \"D\" to move the tetromino.\n",
			"Press \"UP\" or \"W\"to rotate the tetromino.\nPress \"SPACE\" to skip the tetromino.\n",
			"Press \"Q\" to quit."
		});
		end(0);
	});

	map_size_arg.add_name("-s").add_name("--map-size");
	map_size_arg.set_description("[width] [height]: Set the size of the map");
	map_size_arg.set_argc(2);
	map_size_arg.set_called_limit(1);
	map_size_arg.set_act_func([](char **argv) {
		bool success;
		Area a;

		a.width = parse_int(argv[0], &success);
		if(!success) {
			log_error("The width of the map should be an integer, but got %s", argv[0]);
			end(-1);
		}
		a.height = parse_int(argv[1], &success);
		if(!success) {
			log_error("The height of the map should be an integer, but got %s", argv[1]);
			end(-1);
		}
		if(a.width < MINIMUM_MAP_SIZE.width) {
			log_error("The width of the map \"%d\" is too small.", a.width);
			end(-1);
		}
		if(a.height < MINIMUM_MAP_SIZE.height) {
			log_error("The height of the map \"%d\" is too small.", a.height);
			end(-1);
		}
		map_size = a;
		map = Map(map_size, BUFFER_AREA_HEIGHT);
	});

	delay_time_arg.add_name("-t").add_name("--delay-time");
	delay_time_arg.set_description("[TIME(ms)]: Specify how long to delay between frames");
	delay_time_arg.set_argc(1);
	delay_time_arg.set_called_limit(1);
	delay_time_arg.set_act_func([](char **argv){
		bool success;

		delay_time = parse_int(argv[0], &success);
		if(!success) {
			log_error("The delaying time should be an integer, but got %s", argv[0]);
			end(-1);
		}
	});

	ap.register_argument(help);
	ap.register_argument(map_size_arg);
	ap.register_argument(delay_time_arg);

	return ap.process(argc, argv);
}

int main(int argc, char **argv) {
	if(!process_argument(argc, argv)) {
		return -1;
	}
	cin.tie(nullptr);
	//std::ios::sync_with_stdio(false); //Strangest thing ever

	Tetris tetris;
	tetris.start();
	return 0;
}
