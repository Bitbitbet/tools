#include "argument_utils.h"
#include <SDL2/SDL.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

#include <console.h>
#define INCLUDE_ARGUMENT
#include <utils.h>


using console::int_type;
using console::uint_type;
using console::UCoord;
using console::Area;
using console::ArrowKeyPraser;
using console::ColorEnum;
using console::color_reset;
using console::cursor_pos_save;
using console::cursor_pos_reload;
using console::cursor_gotoxy;
using console::screen_clear;
using console::cursor_set_visible;


constexpr struct Area MAP_SIZE = {15, 15};
constexpr uint_type AMOUNT_OF_ROWS = 5; // amount of chessmen required in a row to win


class CoreGame {
private:
	using rows_t = const UCoord (&)[2];
public:
	enum class Unit  : uint8_t {
		EMPTY = 0, WHITE, BLACK
	};
	enum class Status : uint8_t {
		NONE, WHITE_WON, BLACK_WON
	};

	CoreGame() {clear();}
	~CoreGame() = default;

	/* 
	 * ----------------
	 * These series of functions provide the manipulation of a CoreGame.
	 */

	/*
	 * Reset a CoreGame, including clearing map and setting turn to be white's.
	 * Called by the constructor.
	 */
	void clear();

	/*
	 * Place a chessman.
	 * After a call, the status of the CoreGame may be changed.
	 * Assertion failure will cause when the coord specifed has already been occupied.
	 */
	void place(UCoord coord_of_chessman_to_be_placed);



	/* 
	 * ----------------
	 * These series of functions provide the observation of a CoreGame.
	 */

	/*
	 * Access content of map.
	 */
	const Unit &operator[](UCoord coord) const {
		return get({coord.x, coord.y});
	}
	/*
	 * Get the status.
	 * Return: The current status of a CoreGame.
	 */
	Status status() const { return m_status; }
	/*
	 * Get coords of chessmen that are in rows, which is enough to win.
	 * Assertion failed will cause when status() == Status::NONE.
	 * Return type: const Coord (&)[``AMOUNT_OF_CHESSMEN_REQUIRED_IN_A_ROW_TO_WIN``]
	 */
	rows_t get_rows() const {
		assert(status() != Status::NONE);
		return rows;
	}
private:

	Unit &get(UCoord c) {
		assert(c.x < MAP_SIZE.w && c.y < MAP_SIZE.h);
		return map[c.x * MAP_SIZE.w + c.y];
	}
	const Unit &get(UCoord c) const {
		assert(c.x < MAP_SIZE.w && c.y < MAP_SIZE.h);
		return map[c.x * MAP_SIZE.w + c.y];
	}

	Unit map[MAP_SIZE.w * MAP_SIZE.h];
	UCoord rows[2]; /* Representing the start coord and the end coord
			  of a row that is long enough to win */
	bool is_white_turn;
	Status m_status;
};

void CoreGame::clear() {
	memset(map, static_cast<int>(Unit::EMPTY), sizeof(map));
	memset(rows, 0, sizeof(rows));
	is_white_turn = true;
	m_status = Status::NONE;
}

void CoreGame::place(UCoord c) {
	assert(get(c) == Unit::EMPTY);
	
	get(c) = is_white_turn ? Unit::WHITE : Unit::BLACK;

	// Check whether there's rows
	bool results[6] = {true, true, true, true, true, true}; // Check in six ways
	for(uint_type i = 1; i < AMOUNT_OF_ROWS; ++i) {
		for(uint_type j = 0; j < 6; ++j) {
			if(!results[j]) continue;
			UCoord condidate_coord;
			switch(j) {
				case 0: condidate_coord = {c.x - i, c.y - i}; break;
				case 1: condidate_coord = {c.x, c.y - i}; break;
				case 2: condidate_coord = {c.x + i, c.y - i}; break;
				case 3: condidate_coord = {c.x - i, c.y + i}; break;
				case 4: condidate_coord = {c.x, c.y + i}; break;
				case 5: condidate_coord = {c.x + i, c.y + i}; break;
			}
			if(condidate_coord.x >= MAP_SIZE.w ||
					condidate_coord.y >= MAP_SIZE.h) {
				results[j] = false;
				continue;
			}
			if(get(condidate_coord) != get(c)) {
				results[j] = false;
			}
		}
	}
	for(uint_type i = 0; i < 6; ++i) {
		if(results[i]) {
			m_status = is_white_turn ? Status::WHITE_WON : Status::BLACK_WON;
			rows[0] = c;
			switch(i) {
				case 0: rows[1] = {c.x - AMOUNT_OF_ROWS + 1, c.y - AMOUNT_OF_ROWS + 1}; break;
				case 1: rows[1] = {c.x, c.y - AMOUNT_OF_ROWS + 1}; break;
				case 2: rows[1] = {c.x + AMOUNT_OF_ROWS - 1, c.y - AMOUNT_OF_ROWS + 1}; break;
				case 3: rows[1] = {c.x - AMOUNT_OF_ROWS + 1, c.y + AMOUNT_OF_ROWS - 1}; break;
				case 4: rows[1] = {c.x, c.y + AMOUNT_OF_ROWS - 1}; break;
				case 5: rows[1] = {c.x + AMOUNT_OF_ROWS - 1, c.y + AMOUNT_OF_ROWS - 1}; break;
			}
			return;
		}
	}

	is_white_turn = !is_white_turn;
}


namespace frontend_with_SDL2 {
	namespace background {
		constexpr struct {uint8_t r, g, b;} BACKGROUND_COLOR = {255, 228, 181};
		constexpr uint_type LINE_WIDTH = 3;
		constexpr Area BLANK_BETWEEN_LINES_SIZE = {30, 30};
		constexpr uint_type BROAD_WIDTH = 6;
		constexpr Area BLANK_OUTOF_MAP_SIZE = {40, 40};

		constexpr Area SIZE = {
			LINE_WIDTH * MAP_SIZE.w + BLANK_BETWEEN_LINES_SIZE.w *
				(MAP_SIZE.w + 1) +
				BROAD_WIDTH * 2 + BLANK_OUTOF_MAP_SIZE.w * 2,
			LINE_WIDTH * MAP_SIZE.h + BLANK_BETWEEN_LINES_SIZE.h *
				(MAP_SIZE.h + 1) +
				BROAD_WIDTH * 2 + BLANK_OUTOF_MAP_SIZE.h * 2};
	}

	class Game {
	public:
		static Game &instance() { //Make sure that there is only one instance of Game.
			static Game g;
			return g;
		}

		~Game();
		Game(const Game &) = delete;
		Game &operator=(const Game &) = delete;

		void start();
	private:
		SDL_Window *window;
		SDL_Renderer *render;
		SDL_Texture *background_texture;

		Game();
	};

	Game::Game() {
		// Initialize SDL2
		if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
			std::cerr << SDL_GetError() << std::ends;
			exit(1);
		}
		window = SDL_CreateWindow("Gobang", SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					background::SIZE.w,
					background::SIZE.h,
					SDL_WINDOW_SHOWN);
		render = SDL_CreateRenderer(window, -1,
				SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		
	}

	Game::~Game() {
		SDL_DestroyRenderer(render);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void Game::start() {
		while(true) {
			SDL_Event event;
			while(SDL_PollEvent(&event)) {
				switch(event.type) {
					case SDL_QUIT:
						return;
				}
			}
		}
	}
}



/*
 * Gobang in terminal.
 */
namespace frontend_with_console {
	constexpr ColorEnum EMPTY_COLOR = ColorEnum::PURPLE;
	constexpr ColorEnum WHITE_COLOR = ColorEnum::WHITE;
	constexpr ColorEnum BLACK_COLOR = ColorEnum::BLACK;
	constexpr ColorEnum SELECTION_COLOR = ColorEnum::CYAN;

	enum class Key : uint8_t {UP, DOWN, LEFT, RIGHT, ENTER, QUIT};
	constexpr inline Key key_from_console_key(console::Key k) {
		switch(k) {
			case console::Key::UP: return Key::UP;
			case console::Key::DOWN: return Key::DOWN;
			case console::Key::LEFT: return Key::LEFT;
			case console::Key::RIGHT: return Key::RIGHT;
		}
	}

	/*
	 * Print the content of a CoreGame.
	 * Ensure the position of cursor is topleft before calling.
	 * The position of cursor should be {0, MAP_SIZE.h + 2} after the function finishes.
	 */
	void print(const CoreGame &g) {
		putchar('|');
		for(uint_type i = 0; i < MAP_SIZE.w * 2; ++i) putchar('-');
		putchar('|');
		putchar('\n');
		for(uint_type y = 0; y < MAP_SIZE.h; ++y) {
			putchar('|');
			for(uint_type x = 0; x < MAP_SIZE.w; ++x) {
				ColorEnum color;
				switch(g[{x, y}]) {
					case CoreGame::Unit::EMPTY:
						color = EMPTY_COLOR;
						break;
					case CoreGame::Unit::WHITE:
						color = WHITE_COLOR;
						break;
					case CoreGame::Unit::BLACK:
						color = BLACK_COLOR;
						break;
				}
				if(x == 0 || g[{x - 1, y}] != g[{x, y}]) background_color(color);
				printf("  ");
				if(x == MAP_SIZE.w - 1 || g[{x, y}] != g[{x + 1, y}]) color_reset();
			}
			putchar('|');
			putchar('\n');
		}
		putchar('|');
		for(uint_type i = 0; i < MAP_SIZE.w * 2; ++i) putchar('-');
		putchar('|');
		putchar('\n');
	}
	/*
	 * Print the difference between the output of ``print(game)`` and ``print(bufgame)``.
	 * Ensure the position of cursor is topleft before calling.
	 * The position of cursor should be {0, MAP_SIZE.h + 2} after a call completes.
	 */
	void print_diff(const CoreGame &game, const CoreGame &bufgame) {
		for(uint_type x = 0; x < MAP_SIZE.w; ++x) {
			for(uint_type y = 0; y < MAP_SIZE.h; ++y) {
				if(game[{x, y}] != bufgame[{x, y}]) {
					ColorEnum color;
					switch(game[{x, y}]) {
						case CoreGame::Unit::EMPTY:
							color = EMPTY_COLOR;
							break;
						case CoreGame::Unit::WHITE:
							color = WHITE_COLOR;
							break;
						case CoreGame::Unit::BLACK:
							color = BLACK_COLOR;
							break;
					}
					cursor_gotoxy({x * 2 + 1, y + 1});
					background_color(color);
					printf("  ");
					color_reset();
				}
			}
		}
		cursor_gotoxy({0, MAP_SIZE.h + 2});
	}

	/*
	 * Print selection.
	 * Ensure the position of cursor is topleft before calling.
	 * The position of cursor should be {0, MAP_SIZE.h + 2} after a call completes.
	 */
	void print_selection(const CoreGame &game, UCoord selection_pos, bool has_old_selection_pos = false, UCoord old_selection_pos = {0, 0}) {
		assert(selection_pos.x < MAP_SIZE.w || selection_pos.y < MAP_SIZE.h);
		if(game[selection_pos] != CoreGame::Unit::EMPTY) {
			cursor_gotoxy({0, MAP_SIZE.h + 2});
			return;
		};

		if(has_old_selection_pos) { // Remove old selection
			cursor_gotoxy({old_selection_pos.x * 2 + 1, old_selection_pos.y + 1});
			switch(game[old_selection_pos]) {
				case CoreGame::Unit::BLACK: background_color(BLACK_COLOR); break;
				case CoreGame::Unit::WHITE: background_color(WHITE_COLOR); break;
				case CoreGame::Unit::EMPTY: background_color(EMPTY_COLOR); break;
			}
		}
		printf("  ");
		cursor_gotoxy({selection_pos.x * 2 + 1, selection_pos.y + 1}); // Print new selecion
		background_color(SELECTION_COLOR);
		printf("  ");
		color_reset();
		cursor_gotoxy({0, MAP_SIZE.h + 2});
	}


	class Game {
	public:
		static Game &instance() {
			static Game g;
			return g;
		}

		~Game() = default;

		void start();

	private:
		Game();

		CoreGame game, bufgame;
		UCoord selection_pos, buf_selection_pos;
	};

	Game::Game() {
		selection_pos = {MAP_SIZE.w / 2, MAP_SIZE.h / 2};
		buf_selection_pos = selection_pos;
	}

	void Game::start() {
		screen_clear();
		print(game);
		print_selection(game, selection_pos);

		ArrowKeyPraser praser;
		while(true) {
			unsigned char input = getch();
			Key key;
			auto praser_result = praser(input);
			switch(praser_result.first) {
				case ArrowKeyPraser::Status::MATCH:
					key = key_from_console_key(praser_result.second);
					break;
				case ArrowKeyPraser::Status::MATCHING:
					continue;
				case ArrowKeyPraser::Status::MISMATCH:
					break;
			}
			if(praser_result.first == ArrowKeyPraser::Status::MISMATCH) {
				switch(toupper(input)) {
					case 'A': case 'H': key = Key::LEFT; break;
					case 'S': case 'J': key = Key::DOWN; break;
					case 'W': case 'K': key = Key::UP; break;
					case 'D': case 'L': key = Key::RIGHT; break;
					case 'Q': key = Key::QUIT; break;
					case '\n': case ' ': key = Key::ENTER; break;
					default: continue;
				}
			}


			if(key == Key::QUIT) {
				return;
			} else if(key == Key::ENTER) {
				game.place(selection_pos);
			} else {
				UCoord new_selection_pos = selection_pos;
				auto inboard = [](UCoord c) -> bool {
					return c.x < MAP_SIZE.w && c.y < MAP_SIZE.h;
				};
				auto available = [this](UCoord c) -> bool {
					return c.x < MAP_SIZE.w && c.y < MAP_SIZE.h && game[c] == CoreGame::Unit::EMPTY;
				};
				if(key == Key::LEFT) {
					bool move_success = true;
					--new_selection_pos.x;
					while(!available(new_selection_pos)) {
						if(!inboard(new_selection_pos)) {
							move_success = false;
							break;
						}
						--new_selection_pos.x;
					}
					if(move_success) selection_pos = new_selection_pos;
				} else if(key == Key::RIGHT) {
					bool move_success = true;
					++new_selection_pos.x;
					while(!available(new_selection_pos)) {
						if(!inboard(new_selection_pos)) {
							move_success = false;
							break;
						}
						++new_selection_pos.x;
					}
					if(move_success) selection_pos = new_selection_pos;
				} else if(key == Key::UP) {
					bool move_success = true;
					--new_selection_pos.y;
					while(!available(new_selection_pos)) {
						if(!inboard(new_selection_pos)) {
							move_success = false;
							break;
						}
						--new_selection_pos.y;
					}
					if(move_success) selection_pos = new_selection_pos;
				} else if(key == Key::DOWN) {
					bool move_success = true;
					++new_selection_pos.y;
					while(!available(new_selection_pos)) {
						if(!inboard(new_selection_pos)) {
							move_success = false;
							break;
						}
						++new_selection_pos.y;
					}
					if(move_success) selection_pos = new_selection_pos;
				}
			}

			print_diff(game, bufgame);
			print_selection(game, selection_pos, true, buf_selection_pos);
			bufgame = game;
			buf_selection_pos = selection_pos;

			// Check game status
			if(game.status() != CoreGame::Status::NONE) {
				if(game.status() == CoreGame::Status::BLACK_WON) {
					printf("Black won.\n");
				} else if(game.status() == CoreGame::Status::WHITE_WON) {
					printf("White won.\n");
				}
				return;
			}
		}
	}
}


enum class Mode {
	console, graphic
} mode = Mode::console;

/*
 * Process command line.
 * Return: 0 for success, a non-zero integer for failures.
 */
int process_argument(size_t argc, char **argv) {
	ArgumentProcessor ap;

	Argument help, switch_mode;

	help.add_name("-h").add_name("--help").add_name("--usage");
	help.set_argc(0);
	help.set_description("Display this help and exit.");
	help.set_act_func([&ap](char **argv) {
		ap.output_help({argv[0], ": Gobang, can either run with SDL2 or in terminal.\nRun in terminal in default."});
		exit(0);
	});

	switch_mode.add_name("-m").add_name("--mode");
	switch_mode.set_argc(1);
	switch_mode.set_description("Set the display mode of gobang. Possible option: console, graphic.");
	switch_mode.set_act_func([] (char **argv) {
		if(strcmp(argv[0], "console") == 0) {
			mode = Mode::console;
		} else if(strcmp(argv[0], "graphic") == 0) {
			mode = Mode::graphic;
		} else {
			log_error("Type \"%s --help\" for usage.", argv[0]);
			exit(1);
		}
	});

	ap.register_argument(help);
	ap.register_argument(switch_mode);

	return ap.process(argc, argv) ? 0 : 1;
}

int main(int argc, char **argv) {
	if(process_argument(argc, argv) != 0) {
		return 1;
	}
	if(mode == Mode::console) {
		auto &g = frontend_with_console::Game::instance();
		g.start();
	} else if(mode == Mode::graphic) {
		auto &g = frontend_with_SDL2::Game::instance();
		g.start();
	}
	return 0;
}
