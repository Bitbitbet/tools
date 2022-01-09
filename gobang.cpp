#include "argument_utils.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <iostream>

#include <console.h>
#include <sys/types.h>
#define INCLUDE_ARGUMENT
#include <utils.h>


using console::int_type;
using console::uint_type;
using console::UCoord;
using console::Area;
using console::ArrowKeyPraser;
using console::ColorEnum;
using console::color_reset;
using console::cursor_gotoxy;
using console::screen_clear;

using std::string_view;

constexpr struct Area MAP_SIZE = {15, 15};
constexpr uint_type AMOUNT_OF_ROWS = 5; // amount of chessmen required in a row to win

class CoreGame {
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
	std::pair<UCoord, UCoord> get_rows() const {
		assert(status() != Status::NONE);
		return rows;
	}

	bool is_white_turn() const {return m_is_white_turn;}
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
	std::pair<UCoord, UCoord> rows; /* Contains the start coord and the end coord
			  of a row that is long enough to win. */
	bool m_is_white_turn;
	Status m_status;
};

void CoreGame::clear() {
	memset(map, static_cast<int>(Unit::EMPTY), sizeof(map));
	rows = {{0, 0}, {0, 0}};
	m_is_white_turn = true;
	m_status = Status::NONE;
}

void CoreGame::place(UCoord c) {
	assert(get(c) == Unit::EMPTY);

	get(c) = m_is_white_turn ? Unit::WHITE : Unit::BLACK;

	// Check whether there's rows
	bool results[8] = {true, true, true, true,
		true, true, true, true}; // Check in eight ways
	for(uint_type i = 1; i < AMOUNT_OF_ROWS; ++i) {
		for(uint_type j = 0; j < 8; ++j) {
			if(!results[j]) continue;
			UCoord condidate_coord;
			switch(j) {
				case 0: condidate_coord = {c.x - i, c.y}; break;
				case 1: condidate_coord = {c.x - i, c.y - i}; break;
				case 2: condidate_coord = {c.x, c.y - i}; break;
				case 3: condidate_coord = {c.x + i, c.y - i}; break;
				case 4: condidate_coord = {c.x + i, c.y}; break;
				case 5: condidate_coord = {c.x - i, c.y + i}; break;
				case 6: condidate_coord = {c.x, c.y + i}; break;
				case 7: condidate_coord = {c.x + i, c.y + i}; break;
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
	for(uint_type i = 0; i < 8; ++i) {
		if(results[i]) {
			m_status = m_is_white_turn ? Status::WHITE_WON : Status::BLACK_WON;
			rows.first = c;
			switch(i) {
				case 0: rows.second = {c.x - AMOUNT_OF_ROWS + 1, c.y - AMOUNT_OF_ROWS + 1}; break;
				case 1: rows.second = {c.x, c.y - AMOUNT_OF_ROWS + 1}; break;
				case 2: rows.second = {c.x + AMOUNT_OF_ROWS - 1, c.y - AMOUNT_OF_ROWS + 1}; break;
				case 3: rows.second = {c.x - AMOUNT_OF_ROWS + 1, c.y + AMOUNT_OF_ROWS - 1}; break;
				case 4: rows.second = {c.x, c.y + AMOUNT_OF_ROWS - 1}; break;
				case 5: rows.second = {c.x + AMOUNT_OF_ROWS - 1, c.y + AMOUNT_OF_ROWS - 1}; break;
			}
			return;
		}
	}

	m_is_white_turn = !m_is_white_turn;
}


namespace frontend_with_SDL2 {
	namespace background {
		constexpr struct SDL_Color BACKGROUND_COLOR = {255, 228, 181};
		constexpr uint_type LINE_WIDTH = 3;
		constexpr Area BLANK_BETWEEN_LINES_SIZE = {30, 30};
		constexpr uint_type BROAD_WIDTH = 6;
		constexpr Area BLANK_OUTOF_MAP_SIZE = {20, 80};

	}
	constexpr Area WINDOW_SIZE = {
		background::LINE_WIDTH * MAP_SIZE.w + background::BLANK_BETWEEN_LINES_SIZE.w *
			(MAP_SIZE.w + 1) +
			background::BROAD_WIDTH * 2 + background::BLANK_OUTOF_MAP_SIZE.w * 2,
		background::LINE_WIDTH * MAP_SIZE.h + background::BLANK_BETWEEN_LINES_SIZE.h *
			(MAP_SIZE.h + 1) +
			background::BROAD_WIDTH * 2 + background::BLANK_OUTOF_MAP_SIZE.h * 2};
	/*
	 * Generate the background surface.
	 * Return the surface handle, which requires to be freed by SDL_FreeSurface manually.
	 */
	SDL_Surface *generate_background_surface(SDL_PixelFormat *format) {
		using namespace background;
		const auto background_color = SDL_MapRGB(format, BACKGROUND_COLOR.r,
				BACKGROUND_COLOR.g, BACKGROUND_COLOR.b);
		const auto black_color = SDL_MapRGB(format, 50, 50, 50);

		constexpr Area INNER_MAP_SIZE = {LINE_WIDTH * 15 + BLANK_BETWEEN_LINES_SIZE.w * 16,
				LINE_WIDTH * 15 + BLANK_BETWEEN_LINES_SIZE.h * 16};
		constexpr Area REAL_MAP_SIZE = {INNER_MAP_SIZE.w + BROAD_WIDTH * 2,
				INNER_MAP_SIZE.h + BROAD_WIDTH * 2};

		SDL_Surface *background_surface = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_SIZE.w, WINDOW_SIZE.h, 0, format->format);
		SDL_Rect r = {0, 0, WINDOW_SIZE.w, WINDOW_SIZE.h};
		SDL_FillRect(background_surface, &r, background_color);
	
		r = {BLANK_OUTOF_MAP_SIZE.w, BLANK_OUTOF_MAP_SIZE.h,
			REAL_MAP_SIZE.w, REAL_MAP_SIZE.h};
		SDL_FillRect(background_surface, &r, black_color);
		for(size_t x = 0; x < MAP_SIZE.w + 1; ++x) {
			for(size_t y = 0; y < MAP_SIZE.h + 1; ++y) {
				r = {
					static_cast<int>(BLANK_OUTOF_MAP_SIZE.w + BROAD_WIDTH + x *
					(BLANK_BETWEEN_LINES_SIZE.w + LINE_WIDTH)),
					static_cast<int>(BLANK_OUTOF_MAP_SIZE.h + BROAD_WIDTH + y *
					(BLANK_BETWEEN_LINES_SIZE.h + LINE_WIDTH)),
					BLANK_BETWEEN_LINES_SIZE.w, BLANK_BETWEEN_LINES_SIZE.h
				};
				SDL_FillRect(background_surface, &r, background_color);
			}
		}
		return background_surface;
	}

	class Font {
	private:
		/*
		 * Raw font data, with characters in order 0~9A~Z
		 * Each pixel occupies 1 bit; Each character occupies
		 * FONT_CHARACTER_SIZE.w * FONT_CHARACTER_SIZE.h * 1 bits.
		 */
		constexpr static char RAW_FONT_DATA[] =
			"APMADPADAPAMDMPADPMAPPADPMAPPADPMAPDMDAPAMAPMADPAADMAAPAAPMADPAADMAAPAADMA"
			"APAADMAAPAADMAAPADPPMPPPDPPAPPMPADPMAPAAPMADPAPPADPMDPMAPPAPMADPAAPPPPPPPD"
			"PPMPPPAAPAADMADMAAPAAPPADPMAADMAAPDADMMAPDPPAPPMADPAAPMAPPADPMDMPAPDMPAPDM"
			"DMPPPPPPPAAPAADMAAPAADMPPPDPPMPAADMAAPPPDPPMAADMAAPAADMAAPPADPMAPDPPAPPMAP"
			"MADPADMAAPAAPAADMAAPPPDPPMPADPMAPPADPMAPDPPAPPMPPPPPPPPADPMAPAAPAADMADMAAP"
			"AAPAADMAAPAADMAAPAADMADPMAPPAPADDMAMPMDDPAMDPMAPPAMDPPAPPMADPAAPDPPAPPMDPP"
			"APPMPADPMAPPADPMAPDPPMPPPAADMAAPAAPAADMDPMAPPAAPMADPADMPAPDMPADPMAPPADPMAP"
			"PPPPPPPPADPMAPPADPMAPPPPDPPMPADPMAPPADPMAPPPPDPPMPADPMAPPADPMAPPPPDPPMAPPA"
			"DPMDMDMPAPPAADMAAPAADMAAPAADMAADMDMPAPAPPADPMPPMDPPAPAPDMDMPADPMAPPADPMAPP"
			"ADPMAPPAPDMDMPPMDPPADPPMPPPDMAAPAADMAAPAADPPAPPMDMAAPAADMAAPAADPPMPPPPPPPP"
			"PPPAADMAAPAADMAAPPPDPPMPAADMAAPAADMAAPAADMAAAPPMDPPDMAAPAAPAADMAAPAPPMDPPA"
			"DPMAPDMDMPAPAPPMDPPPADPMAPPADPMAPPADPMAPPPPPPPPPADPMAPPADPMAPPADPMAPDPPMPP"
			"PADMAAPAADMAAPAADMAAPAADMAAPAADMAAPADPPMPPPDPPMPPPADMAAPAADMAAPAADMAAPAADM"
			"AAPADDMAMPADPAAPMAPADPMAPPAPDMDMPDMDMPAPPADPMAPPMDPPAPDPDMPMPAPPMDPDMAAPAA"
			"DMAAPAADMAAPAADMAAPAADMAAPAADMAAPAADPPMPPPPADPMAPPMPPPDPPPPPPPPPPPPPPPPDDP"
			"MMPPADPMAPPADPMAPPADPMAPPMDPPAPPPDPPMPPPPPPPPPDPPMPPPAPPMDPPADPMAPDPPAPPMP"
			"ADPMAPPADPMAPPADPMAPPADPMAPPADPMAPDPPAPPMPPPDPPMPADPMAPPADPMAPPADPMAPPPPDP"
			"PMPAADMAAPAADMAADPMAPPAPADPMAPPADPMAPPADPMAPPADPMAPDPPAPPMAADMAAPPPPDPPMPA"
			"DPMAPPADPMAPPAPPMDPPPMDPPAPDPDMPMPAPPMDPDPMAPPAPAPDMDMPAADMAADPPAPPMAADMAA"
			"PPADPMAPDPPAPPMDPPMPPPADMAAPAADMAAPAADMAAPAADMAAPAADMAAPAADMAAPAPADPMAPPAD"
			"PMAPPADPMAPPADPMAPPADPMAPPADPMAPDPPAPPMPADPMAPPADPMAPPADPMAPPMPPPDPDPPAPPM"
			"APMADPAADAAAMAPADPMAPPADPMAPPDDPMMPPPPPPPPPPPPPPPPMPPPDPPADPMAPPADPMAPDMPA"
			"PDMAPMADPAAPMADPADMPAPDMPADPMAPMAAPAADDMDMPAPDMDMPAPDMDMPAPAPPADPMADMAAPAA"
			"DMAAPAADMAAPADPPMPPPAADMAAPAAPAADMADMAAPAAPAADMADMAAPAADPPMPPP";

		constexpr static Area FONT_CHARACTER_SIZE = {14, 14};
		constexpr static size_t SCALE_TIME = 2;
		constexpr static Area DEFAULT_EXTRA_ADVANCE = {SCALE_TIME, SCALE_TIME};

	public:
		constexpr static size_t CHARACTER_COUNT = 36;
		constexpr static Area CHARACTER_SIZE = {FONT_CHARACTER_SIZE.w * SCALE_TIME, FONT_CHARACTER_SIZE.h * SCALE_TIME};

		Font(SDL_PixelFormat *format, SDL_Renderer *render, SDL_Color color);
		~Font();
		Font(const Font &) = delete;
		Font &operator=(const Font &&) = delete;

		/*
		 * Render a string with specific string, position and advance.
		 * The Ucoord pos describes the topleft of the text.
		 * Skip the character unsupported(be seen as ' ').
		 *
		 * Return the region rendered on.
		 */
		SDL_Rect render_text(const string_view string,
				UCoord topleft_position,
				Area extra_advance = DEFAULT_EXTRA_ADVANCE) const;

		/*
		 * Same as render_text.
		 * The SDL_Surface* that is returned needs freed manually.
		 */
		SDL_Surface *create_surface_from_text(
				SDL_PixelFormat *format,
				const string_view string,
				Area extra_advance = DEFAULT_EXTRA_ADVANCE) const;

	private:

		/*
		 * See RAW_FONT_DATA as a bitset, and access it with a index.
		 */
		constexpr static uint8_t get(uint_type index) {
			constexpr uint8_t condidate[] = {
				0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
				0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
				0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
				0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
			};
			return condidate[(index % 4) * 16 + (RAW_FONT_DATA[index / 4] - 'A')];
		}

		SDL_Color fore, back;

		SDL_Surface *font_surface;
		SDL_Texture *font_texture;

		SDL_Renderer *render;
	};
	Font::Font(SDL_PixelFormat *format, SDL_Renderer *render, SDL_Color color) :
		fore(color), back(color.r == 0 && color.g == 0 && color.b == 0 ? SDL_Color{255, 255, 255} : SDL_Color{0, 0, 0}) {

		font_surface = SDL_CreateRGBSurfaceWithFormat(0,
				FONT_CHARACTER_SIZE.w * CHARACTER_COUNT * SCALE_TIME,
				FONT_CHARACTER_SIZE.h * SCALE_TIME,
				0, format->format);

		const Uint32 fore_color = SDL_MapRGB(format, color.r, color.g, color.b);
		const Uint32 back_color = SDL_MapRGB(format, back.r, back.g, back.b);
		memset(font_surface->pixels, back_color, font_surface->pitch * font_surface->h);

		for(uint_type index = 0; index < CHARACTER_COUNT; ++index) {
			SDL_Rect r{0, 0, SCALE_TIME, SCALE_TIME};
			for(uint_type y = 0; y < FONT_CHARACTER_SIZE.h; ++y) {
				r.y = y * SCALE_TIME;
				for(uint_type x = 0; x < FONT_CHARACTER_SIZE.w; ++x) {
					if(get(index * FONT_CHARACTER_SIZE.w * FONT_CHARACTER_SIZE.h + y * FONT_CHARACTER_SIZE.w + x) == 1) {
						r.x = (x + FONT_CHARACTER_SIZE.w * index) * SCALE_TIME;
						SDL_FillRect(font_surface, &r, fore_color);
					}
				}
			}
		}

		SDL_SetColorKey(font_surface, SDL_TRUE, back_color);
		/// SDL_LockSurface(font_surface);
		/// SDL_SetSurfaceRLE(font_surface, SDL_TRUE);

		font_texture = SDL_CreateTextureFromSurface(render, font_surface);
		this->render = render;
	}
	Font::~Font() {
		SDL_FreeSurface(font_surface);
		SDL_DestroyTexture(font_texture);
	}
	SDL_Rect Font::render_text(const string_view str, UCoord pos, Area extra_advance) const {
		SDL_Rect srcrect{0, 0, CHARACTER_SIZE.w, CHARACTER_SIZE.h}, dstrect{0, 0, CHARACTER_SIZE.w, CHARACTER_SIZE.h};
		dstrect.x = pos.x;
		dstrect.y = pos.y;
		SDL_Rect region{ static_cast<int>(pos.x), static_cast<int>(pos.y), 0, 1};
		size_t region_width_temp = 0;
		for(char c : str) {
			c = toupper(c);
			bool right_advance = false, down_advance = false;
			if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
				if(c >= '0' && c <= '9') {
					srcrect.x = CHARACTER_SIZE.w * (c - '0');
				} else {
					srcrect.x = CHARACTER_SIZE.w * (c - 'A' + 10);
				}

				SDL_RenderCopy(render, font_texture, &srcrect, &dstrect);
				right_advance = true;
			} else if(c == '\n') {
				down_advance = true;
			} else {
				right_advance = true;
			}
			if(right_advance) {
				dstrect.x += CHARACTER_SIZE.w + extra_advance.w;
				++region_width_temp;
				if(region_width_temp > region.w) {
					region.w = region_width_temp;
				}
			}
			if(down_advance) {
				dstrect.x = pos.x;
				dstrect.y += CHARACTER_SIZE.h + extra_advance.h;
				++region.h;
				region_width_temp = 0;
			}
		}
		if(region.w != 0) region.w = region.w * (CHARACTER_SIZE.w + extra_advance.w) - extra_advance.w;
		region.h = region.h * (CHARACTER_SIZE.h + extra_advance.h) - extra_advance.h;
		return region;
	}
	SDL_Surface *Font::create_surface_from_text(SDL_PixelFormat *format, const string_view str, Area extra_advance) const {
		Area surface_size = {0, 1};
		{
			uint_type width = 0;
			for(char c : str) {
				if(c == '\n') {
					width = 0;
					++surface_size.h;
				} else {
					++width;
					if(width > surface_size.w) {
						surface_size.w = width;
					}
				}
			}
			if(surface_size.w != 0) surface_size.w = surface_size.w * (CHARACTER_SIZE.w + extra_advance.w) - extra_advance.w;
			surface_size.h = surface_size.h * (CHARACTER_SIZE.h + extra_advance.h) - extra_advance.h;
		}
		SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, surface_size.w, surface_size.h, 0, format->format);
		SDL_Rect srcrect{0, 0, CHARACTER_SIZE.w, CHARACTER_SIZE.h}, dstrect{0, 0, CHARACTER_SIZE.w, CHARACTER_SIZE.h};
		for(char c : str) {
			c = toupper(c);
			bool right_advance = false, down_advance = false;
			if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
				if(c >= '0' && c <= '9') {
					srcrect.x = CHARACTER_SIZE.w * (c - '0');
				} else {
					srcrect.x = CHARACTER_SIZE.w * (c - 'A' + 10);
				}

				SDL_BlitSurface(font_surface, &srcrect, surface, &dstrect);
				right_advance = true;
			} else if(c == '\n') {
				down_advance = true;
			} else {
				right_advance = true;
			}
			if(right_advance) dstrect.x += CHARACTER_SIZE.w + extra_advance.w;
			if(down_advance) {
				dstrect.x = 0;
				dstrect.y += CHARACTER_SIZE.h + extra_advance.h;
			}
		}
		SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(format, back.r, back.g, back.b));
		return surface;
	}
	

	/*
	 * Frontend with SDL2.
	 */
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
		Font *font;

		CoreGame game;

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
					WINDOW_SIZE.w,
					WINDOW_SIZE.h,
					SDL_WINDOW_SHOWN);
		render = SDL_CreateRenderer(window, -1,
				SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		SDL_Surface *screen = SDL_GetWindowSurface(window);

		SDL_Surface *background_surface = generate_background_surface(screen->format);
		background_texture = SDL_CreateTextureFromSurface(render, background_surface);
		SDL_FreeSurface(background_surface);

		font = new Font(screen->format, render, {0x20, 0x20, 0x20});
	}

	Game::~Game() {
		delete font;
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

			SDL_SetRenderDrawColor(render, background::BACKGROUND_COLOR.r, background::BACKGROUND_COLOR.g, background::BACKGROUND_COLOR.b, 255);
			SDL_RenderClear(render);
			SDL_RenderCopy(render, background_texture, nullptr, nullptr);
			SDL_Rect r = font->render_text("TODO MEANS THERES NOTHING TO DO", {0, 0});
			SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
			SDL_RenderDrawRect(render, &r);


			SDL_RenderPresent(render);
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

	enum class Key : uint8_t {
		UP, DOWN, LEFT, RIGHT, ENTER, RESET/* reset the selection position */, QUIT
	};
	constexpr inline Key key_from_console_key(console::Key k) {
		switch(k) {
			case console::Key::UP: return Key::UP;
			case console::Key::DOWN: return Key::DOWN;
			case console::Key::LEFT: return Key::LEFT;
			case console::Key::RIGHT: return Key::RIGHT;
		}
		return Key::UP; //avoid warning
	}

	/*
	 * Print the content of a CoreGame.
	 * Ensure the position of cursor is topleft before calling.
	 * The position of cursor is set to {0, MAP_SIZE.h + 2} after the function finishes.
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
	 * The position of cursor is set to {0, MAP_SIZE.h + 2} after a call completes.
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
	 * The position of cursor is set to {0, MAP_SIZE.h + 2} after a call completes.
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
		printf("%s's turn.\n", game.is_white_turn() ? "White" : "Black");
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
					case 'R': key = Key::RESET; break;
					case 'Q': key = Key::QUIT; break;
					case '\n': case ' ': key = Key::ENTER; break;
					default: continue;
				}
			}


			if(key == Key::QUIT) {
				return;
			} else if(key == Key::ENTER) {
				if(game[selection_pos] == CoreGame::Unit::EMPTY) {
					game.place(selection_pos);
				}
			} else if(key == Key::RESET) {
				bool reset_success = false;
				UCoord iter = {selection_pos.x + 1, selection_pos.y};
				while(iter != selection_pos) {
					if(iter.x == MAP_SIZE.w) {
						iter.x = 0;
						++iter.y;
						if(iter.y == MAP_SIZE.h) {
							iter.y = 0;
						}
					}

					if(game[iter] == CoreGame::Unit::EMPTY) {
						selection_pos = iter;
						reset_success = true;
						break;
					}

					++iter.x;
				}
				if(!reset_success) return;
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
			if(game.status() == CoreGame::Status::NONE) {
				printf("%s's turn.\n", game.is_white_turn() ? "White" : "Black");
			} else {
				if(game.status() == CoreGame::Status::BLACK_WON) {
					printf("\nBlack won.\n");
				} else if(game.status() == CoreGame::Status::WHITE_WON) {
					printf("\nWhite won.\n");
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
		ap.output_help({argv[0], ": Gobang, can either run with SDL2 or in terminal.\nRun in terminal in default.\nConsole mode: WASDQR<Enter><Space>"});
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
