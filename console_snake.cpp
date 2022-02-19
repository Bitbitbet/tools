#include <iostream>
#include <cassert>
#include <cstring>
#include <thread>
#include <chrono>
#include <random>
#include <memory>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <functional>

#include "console.h"
#define INCLUDE_ARGUMENT
#include "utils.h"

using console::ColorEnum;
using console::ArrowKeyPraser;
using console::color_reset;
using console::cursor_gotoxy;
using console::screen_clear;
using namespace std::chrono;

using std::cin;
using std::find;
using std::list;
using std::cout;
using std::endl;
using std::flush;
using std::thread;
using std::function;
using std::unique_ptr;
using std::random_device;
using std::default_random_engine;
using std::uniform_int_distribution;

inline void end(int code) { exit(code); }

inline void delay(size_t ms) {
	std::this_thread::sleep_for(milliseconds(ms));
}
inline void delay_us(size_t us) {
	std::this_thread::sleep_for(microseconds(us));
}
template <typename Rep, typename Period>
inline void delay(const duration<Rep, Period> &duration) {
	std::this_thread::sleep_for(duration);
}

enum class Key : int8_t { UP = 0, DOWN, LEFT, RIGHT, QUIT };
enum class Direction : int8_t { UP = 0, DOWN, LEFT, RIGHT };
enum class Field : int8_t { EMPTY = 0, HEAD, BODY, FOOD };

Key key_from_console_key(console::Key k) {
	switch(k) {
		case console::Key::DOWN: return Key::DOWN;
		case console::Key::UP: return Key::UP;
		case console::Key::LEFT: return Key::LEFT;
		case console::Key::RIGHT: return Key::RIGHT;
	}
	return Key::UP; // avoid warning
}

constexpr Direction operator!(Direction dir) {
	switch(dir) {
		case Direction::UP: return Direction::DOWN;
		case Direction::DOWN: return Direction::UP;
		case Direction::LEFT: return Direction::RIGHT;
		case Direction::RIGHT: return Direction::LEFT;
	}
	return Direction::UP; // avoid warning
}

constexpr size_t BLOCK_SPACE_WIDTH = 2;
constexpr char BLOCK_TEXT[BLOCK_SPACE_WIDTH + 1]{"  "};

constexpr ColorEnum HEAD_COLOR = ColorEnum::BLUE;
constexpr ColorEnum BODY_COLOR = ColorEnum::CYAN;
constexpr ColorEnum FOOD_COLOR = ColorEnum::RED;
constexpr auto DEFAULT_INITIAL_PAUSE_TIME = microseconds(1000 * 500);
constexpr auto DEFAULT_MINIMAL_PAUSE_TIME = microseconds(1000 * 100);
constexpr auto DEFAULT_PAUSE_TIME_REDUCTION = microseconds(1000 * 5);
constexpr Area DEFAULT_MAP_AREA = {20, 15};

auto initial_pause_time = DEFAULT_INITIAL_PAUSE_TIME;
auto minimal_pause_time = DEFAULT_MINIMAL_PAUSE_TIME;
auto pause_time_reduction = DEFAULT_PAUSE_TIME_REDUCTION;
Area map_area = DEFAULT_MAP_AREA;

default_random_engine engine((random_device()()));

class Map {
	friend class MapReader;
private:
	enum class Status : int8_t {
		UNINITIALIZED, INITIALIZED, FAILED, WON
	};
public:
	Map(Area a) : m_area(a), status(Status::UNINITIALIZED) {}
	Map(const Map &map) :
		m_area		(map.m_area),
		status		(map.status),
		food_pos	(map.food_pos),
		snake		(map.snake),
		m_direction	(map.m_direction) {}
	~Map() = default;
	Map &operator=(const Map &map) & {
		m_area = map.m_area;
		status = map.status;
		if(status != Status::UNINITIALIZED) {
			snake = map.snake;
			m_direction = map.m_direction;
			food_pos = map.food_pos;
		}
		return *this;
	}
	Map &operator=(Map &&) & = default;

	void clear() {
		status = Status::UNINITIALIZED;
	}
	void initialize() {
		assert(status == Status::UNINITIALIZED);
		status = Status::INITIALIZED;
		snake.clear();
		snake.push_back(random_pos());
		m_direction = random_direction();
		random_food();
	}

	void snake_forward() {
		assert(status == Status::INITIALIZED);

		UCoord head_pos = apply(snake.front(), m_direction);
		snake.push_front(head_pos);
		if(head_pos == food_pos) {
			random_food();
		} else snake.pop_back();
		
		if(find(++snake.begin(), snake.end(), head_pos) != snake.end()) {
			status = Status::FAILED;
			return;
		}
	}

	void resize(Area a) {
		assert(status == Status::UNINITIALIZED);
		m_area = a;
	}
	[[nodiscard]] bool gaming() const {return status == Status::INITIALIZED;}
	[[nodiscard]] bool failed() const {return status == Status::FAILED;}
	[[nodiscard]] bool won() const {return status == Status::WON;}
	[[nodiscard]] Area area() const {return m_area;}
	[[nodiscard]] uint_type score() const {assert(status != Status::UNINITIALIZED); return snake.size() - 1;}
	[[nodiscard]] Direction direction() const {return m_direction;}
	void direction(Direction dir) {m_direction = dir;}
private:
	Area m_area;
	Status status;
	UCoord food_pos;
	list<UCoord> snake;
	Direction m_direction;

	[[nodiscard]] UCoord random_pos() const {
		uniform_int_distribution<uint_type> range_w(0, m_area.w - 1);
		uniform_int_distribution<uint_type> range_h(0, m_area.h - 1);
		return {range_w(engine), range_h(engine)};
	}
	[[nodiscard]] UCoord apply(UCoord coord, Direction dir) const {
		switch(dir) {
			case Direction::UP:
				if(coord.y == 0) coord.y = m_area.h - 1;
				else --coord.y;
				break;
			case Direction::DOWN:
				if(coord.y == m_area.h - 1) coord.y = 0;
				else ++coord.y;
				break;
			case Direction::LEFT:
				if(coord.x == 0) coord.x = m_area.w - 1;
				else --coord.x;
				break;
			case Direction::RIGHT:
				if(coord.x == m_area.w - 1) coord.x = 0;
				else ++coord.x;
				break;
		}
		return coord;
	}
	[[nodiscard]] static Direction random_direction() {
		static uniform_int_distribution<int8_t> range_dir(0, 3);
		return static_cast<Direction>(range_dir(engine));
	}
	void random_food() {
		if(snake.size() == m_area.w * m_area.h) {
			status = Status::WON;
			return;
		}
		UCoord coord;
		while((coord = random_pos()), find(snake.begin(), snake.end(), coord) != snake.end());
		food_pos = coord;
	}
};

class MapReader {
public:
	void generate(const Map &map) {
		assert(map.status == Map::Status::INITIALIZED || map.status == Map::Status::WON);

		if(data_area != map.m_area) data_reset(map);
		data_empty();

		auto iter = map.snake.begin();
		data_get(*iter) = Field::HEAD;
		++iter;
		for(; iter != map.snake.end(); ++iter) {
			data_get(*iter) = Field::BODY;
		}
		if(map.status != Map::Status::WON) data_get(map.food_pos) = Field::FOOD;
	}
	Field operator[](UCoord coord) const {
		return data_get(coord);
	}
private:
	unique_ptr<Field[]> m_data;
	Area data_area;

	void data_reset(const Map &map) {
		data_area = map.m_area;
		m_data.reset(new Field[data_area.w * data_area.h]);
	}
	void data_empty() {memset(m_data.get(), static_cast<int>(Field::EMPTY), sizeof(Field) * data_area.w * data_area.h);}
	[[nodiscard]] Field &data_get(UCoord coord) const {return m_data[coord.x + data_area.w * coord.y];}
};

class KeyEvent {
private:
	using callback_type = void(Key);
public:
	KeyEvent() : running(false) {}
	~KeyEvent() {
		stop();
		if(thrd.joinable()) {
			thrd.join();
		}
	}
	void set_callback(function<callback_type> func) {
		callback = std::move(func);
	}
	void start() {
		assert(!thrd.joinable());
		assert(callback);
		running = true;
		thrd = thread(&KeyEvent::thrd_func, this);
	}
	void stop() {
		running = false;
	}
private:
	thread thrd;
	function<callback_type> callback;
	volatile bool running;

	void thrd_func() const {
		ArrowKeyPraser arrow_key_praser;
		while(running) {
			unsigned char c = getch([]() -> unsigned char {return cin.get();});
			c = toupper(c);
			if(!running) break;
			auto result = arrow_key_praser(c);
			switch(result.first) {
				case console::ArrowKeyPraser::Status::MATCH:
					callback(key_from_console_key(result.second));
					continue;
					break;
				case console::ArrowKeyPraser::Status::MATCHING:
					continue;
					break;
				case console::ArrowKeyPraser::Status::MISMATCH:
					break;
			}

			switch(c) { //wasd and hjkl keys
				case 'W': case 'K': callback(Key::UP); break;
				case 'S': case 'J': callback(Key::DOWN); break;
				case 'A': case 'H': callback(Key::LEFT); break;
				case 'D': case 'L': callback(Key::RIGHT); break;
				case 'Q': callback(Key::QUIT); break;
			}
		}
	}
	
};

Map map(map_area);
MapReader reader, buffer_reader;
bool enable_output_map_soft = true;

void output_ground() {
	cout << "-";
	for(uint_type i = 0; i < map_area.w; ++i) {
		for(size_t j = 0; j < BLOCK_SPACE_WIDTH; ++j) cout << "-";
	}
	cout << "-";
	cout << endl;
}

void output_field(Field f) {
	switch(f) {
		case Field::HEAD:
			background_color(HEAD_COLOR);
			break;
		case Field::BODY:
			background_color(BODY_COLOR);
			break;
		case Field::FOOD:
			background_color(FOOD_COLOR);
			break;
		case Field::EMPTY:
			color_reset();
			for(uint_type i = 0; i < BLOCK_SPACE_WIDTH - 1; ++i) cout << ' ';
			cout << '.' << flush;
			return;
			break;
	}
	cout << BLOCK_TEXT << flush;
}

void output_map() {
	cursor_gotoxy({0, 0});
	reader.generate(map);
	buffer_reader.generate(map);

	UCoord coord;
	output_ground();
	for(coord.y = 0; coord.y < map_area.h; ++coord.y) {
		cout << "|";
		for(coord.x = 0; coord.x < map_area.w; ++coord.x) {
			output_field(reader[coord]);
		}
		color_reset();
		cout << "|\n";
	}
	output_ground();
	cout << "Score: " << map.score() << endl;
}
void output_map_soft() {
	reader.generate(map);

	UCoord coord;
	for(coord.y = 0; coord.y < map_area.h; ++coord.y) {
		for(coord.x = 0; coord.x < map_area.w; ++coord.x) {
			if(reader[coord] != buffer_reader[coord]) {
				cursor_gotoxy(UCoord{coord.x * 2 + 1, coord.y + 1});
				output_field(reader[coord]);
			}
		}
	}
	cursor_gotoxy({0, map_area.h + 2});
	color_reset();
	cout << "Score: " << map.score() << endl;
	buffer_reader.generate(map);
}

bool process_argument(int argc, char **argv) {
	ArgumentProcessor ap;
	Argument help, map_area_arg, hard_mode;
	help.add_name("-h").add_name("--help").set_argc(0);
	help.set_description("Display this help and exit");
	help.set_act_func([&ap](char **argv) {
		ap.output_help({argv[0], ": A console snake game."});
		end(0);
	});

	map_area_arg.add_name("-s").add_name("--size").set_argc(2);
	map_area_arg.set_description("Specify the area of the map");
	map_area_arg.set_act_func([](char **argv) {
		constexpr Area MINIMAL_MAP_AREA = {2, 2};
		bool success;
		map_area.w = parse_int(argv[0], &success);
		if(!success) {
			log_error("The width of the map must be an integer. (\"%s\")", argv[0]);
			end(1);
		}
		if(map_area.w < MINIMAL_MAP_AREA.w) {
			log_error("Specified width should be upper than %d. (%s)", MINIMAL_MAP_AREA.w - 1, argv[0]);
			end(2);
		}
		map_area.h = parse_int(argv[1], &success);
		if(!success) {
			log_error("The height of the map must be an integer.(\"%s\")", argv[1]);
			end(1);
		}
		if(map_area.h < MINIMAL_MAP_AREA.h) {
			log_error("Specified height should be upper than %d. (%s)", MINIMAL_MAP_AREA.h - 1, argv[1]);
			end(2);
		}
	});

	hard_mode.add_name("--hard-mode").set_argc(0);
	hard_mode.set_act_func([](char **) {
		enable_output_map_soft = false;
	});

	ap.register_argument(help);
	ap.register_argument(map_area_arg);
	ap.register_argument(hard_mode);

	return ap.process(argc, argv);
}

int main(int argc, char **argv) {
	if(!process_argument(argc, argv)) {
		return 1;
	}
	screen_clear();
	bool request_stop = false;
	map.resize(map_area);
	map.initialize();
	Direction will_direction = map.direction();

	KeyEvent event;
	event.set_callback([&](Key k) {
		Direction dir = Direction::UP;
		switch(k) {
			case Key::UP: dir = Direction::UP; break;
			case Key::DOWN: dir = Direction::DOWN; break;
			case Key::LEFT: dir = Direction::LEFT; break;
			case Key::RIGHT: dir = Direction::RIGHT; break;
			case Key::QUIT: event.stop(); request_stop = true; return;
		}
		if(dir != !map.direction()) will_direction = dir;
	});
	event.start();
	size_t score = 0;
	auto pause_time = initial_pause_time;
	output_map();
	auto output_func = output_map_soft;
	if(!enable_output_map_soft) output_func = output_map;
	while(!request_stop) {
		map.direction(will_direction);
		map.snake_forward();
		if(!map.gaming()) {
			if(map.won()) output_func();
			break;
		}
		output_func();
		if(score != map.score()) {
			if(pause_time != minimal_pause_time) pause_time -= (map.score() - score) * pause_time_reduction;
			if(pause_time < minimal_pause_time) pause_time = minimal_pause_time;
			score = map.score();
		}
		delay(pause_time);
	}
	color_reset();
	cout << "Final Score: " << map.score() << ". Press any key to exit..." << endl;
	return 0;
}
