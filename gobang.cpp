#include <memory>
#include <vector>
#include <iostream>
#include <functional>
#include <string>
#include <string_view>
#include <initializer_list>
#include <thread>

#include <cstdint>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include <SDL2/SDL.h>

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
using console::cursor_gotoxy;
using console::screen_clear;

using std::string_view;

constexpr Area DEFAULT_MAP_SIZE = {15, 15};
constexpr uint_type DEFAULT_AMOUNT_OF_ROWS = 5;

Area map_size = DEFAULT_MAP_SIZE;
uint_type amount_of_rows = DEFAULT_AMOUNT_OF_ROWS; // amount of chessmen required in a row to win


enum class Mode {
	console, graphic, all
} mode = Mode::all;

bool software_rendering = false;




class CoreGame {
public:
	enum class Unit  : uint8_t {
		EMPTY = 0, WHITE, BLACK
	};
	enum class Status : uint8_t {
		NONE, WHITE_WON, BLACK_WON
	};

	CoreGame() {
		map = new Unit[map_size.w * map_size.h];
		clear();
	}
	CoreGame(const CoreGame &c) :
		rows(c.rows),
		m_is_white_turn(c.m_is_white_turn),
		m_status(c.m_status)

	{
		map = new Unit[map_size.w * map_size.h];
		memcpy(map, c.map, map_size.w * map_size.h * sizeof(Unit));
	}
	~CoreGame() {delete[] map;}

	CoreGame &operator=(const CoreGame &c) {
		rows = c.rows;
		m_is_white_turn = c.m_is_white_turn;
		m_status = c.m_status;
		memcpy(map, c.map, map_size.w * map_size.h * sizeof(Unit));
		return *this;
	}

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
		assert(c.x < map_size.w && c.y < map_size.h);
		return map[c.y * map_size.w + c.x];
	}
	const Unit &get(UCoord c) const {
		assert(c.x < map_size.w && c.y < map_size.h);
		return map[c.y * map_size.w + c.x];
	}

	Unit *map;
	std::pair<UCoord, UCoord> rows; /* Contains the start coord and the end coord
			of a row that is long enough to win. */
	bool m_is_white_turn;
	Status m_status;
};

void CoreGame::clear() {
	memset(map, static_cast<int>(Unit::EMPTY), map_size.w * map_size.h * sizeof(Unit));
	rows = {{0, 0}, {0, 0}};
	m_is_white_turn = false;
	m_status = Status::NONE;
}

void CoreGame::place(UCoord c) {
	assert(get(c) == Unit::EMPTY);

	get(c) = m_is_white_turn ? Unit::WHITE : Unit::BLACK;

	UCoord start;
	UCoord iter = {0, c.y}; //Search in direction: -
	uint_type row_count = 0;
	for(; iter.x < map_size.w; ++iter.x) {
		if(get(iter) == get(c)) {
			if(row_count == 0) {
				start = iter;
			}
			++row_count;
			if(row_count == amount_of_rows) { // someone won
				rows.first = start;
				rows.second = iter;
				m_status = m_is_white_turn ? Status::WHITE_WON : Status::BLACK_WON;
				return;
			}
		} else {
			row_count = 0;
		}
	}
	iter = c.x > c.y ? UCoord{c.x - c.y, 0} : UCoord{0, c.y - c.x}; // Search in direction: '\'
	row_count = 0;
	for(; iter.x < map_size.w && iter.y < map_size.h; ++iter.x, ++iter.y) {
		if(get(iter) == get(c)) {
			if(row_count == 0) {
				start = iter;
			}
			++row_count;
			if(row_count == amount_of_rows) { // someone won
				rows.first = start;
				rows.second = iter;
				m_status = m_is_white_turn ? Status::WHITE_WON : Status::BLACK_WON;
				return;
			}
		} else {
			row_count = 0;
		}
	}
	iter = {c.x, 0}; // Search in direction: |
	row_count = 0;
	for(; iter.y < map_size.h; ++iter.y) {
		if(get(iter) == get(c)) {
			if(row_count == 0) {
				start = iter;
			}
			++row_count;
			if(row_count == amount_of_rows) { // someone won
				rows.first = start;
				rows.second = iter;
				m_status = m_is_white_turn ? Status::WHITE_WON : Status::BLACK_WON;
				return;
			}
		} else {
			row_count = 0;
		}
	}
	iter = (map_size.w - 1 - c.x) > c.y ? UCoord{c.x + c.y, 0} : UCoord{map_size.w - 1, c.y - (map_size.w - 1 - c.x)};
	row_count = 0; // Search in direction: /
	for(; iter.x < map_size.w && iter.y < map_size.h; --iter.x, ++iter.y) {
			if(get(iter) == get(c)) {
				if(row_count == 0) {
					start = iter;
				}
				++row_count;
				if(row_count == amount_of_rows) { // someone won
					rows.first = start;
					rows.second = iter;
					m_status = m_is_white_turn ? Status::WHITE_WON : Status::BLACK_WON;
					return;
				}
			} else {
				row_count = 0;
			}
	}

	m_is_white_turn = !m_is_white_turn;
}


namespace frontend_with_SDL2 { // ---------------- Frontend with SDL2 and SDL2_gfx
	constexpr SDL_Color WHITE_CHESSMAN_COLOR = {220, 220, 255, 255};
	constexpr SDL_Color BLACK_CHESSMAN_COLOR = {40, 40, 40, 255};

	constexpr SDL_Color BACKGROUND_COLOR = {230, 205, 163, 255};
	constexpr uint_type BACKGROUND_LINE_WIDTH = 3;
	constexpr Area BACKGROUND_BLANK_BETWEEN_LINES_SIZE = {30, 30};
	constexpr uint_type BACKGROUND_BORDER_WIDTH = 6;
	constexpr Area DEFAULT_BACKGROUND_BLANK_OUTOF_MAP_SIZE = {20, 60};
	Area background_blank_outof_map_size = DEFAULT_BACKGROUND_BLANK_OUTOF_MAP_SIZE;
	constexpr uint_type MINIMUM_WINDOW_WIDTH = 400;
	
	Area inner_map_size;
	Area real_map_size;
	Area window_size;
	void calculate() {
		inner_map_size = {BACKGROUND_LINE_WIDTH * map_size.w + BACKGROUND_BLANK_BETWEEN_LINES_SIZE.w * (map_size.w + 1),
				BACKGROUND_LINE_WIDTH * map_size.h + BACKGROUND_BLANK_BETWEEN_LINES_SIZE.h * (map_size.h + 1)};
		real_map_size = {inner_map_size.w + BACKGROUND_BORDER_WIDTH * 2,
				inner_map_size.h + BACKGROUND_BORDER_WIDTH * 2};
		if(real_map_size.w + background_blank_outof_map_size.w * 2 < MINIMUM_WINDOW_WIDTH) {
			background_blank_outof_map_size.w = (MINIMUM_WINDOW_WIDTH - real_map_size.w) / 2;
		}
		window_size = {
			real_map_size.w + background_blank_outof_map_size.w * 2,
			real_map_size.h + background_blank_outof_map_size.h * 2
		};
	}
	
	struct URect {
		UCoord coord;
		Area area;

		constexpr operator SDL_Rect() const {
			SDL_Rect r;
			r.x = coord.x;
			r.y = coord.y;
			r.w = area.w;
			r.h = area.h;
			return r;
		}
	};
	constexpr inline bool operator==(URect lfs, URect rfs) {
		return lfs.coord == rfs.coord && lfs.area == rfs.area;
	}
	constexpr inline bool operator!=(URect lfs, URect rfs) {
		return lfs.coord != rfs.coord || lfs.area != rfs.area;
	}


	/*
	 * Calculate the actual coord of chessman on the screen, according to the coord of chessman on the map.
	 */
	UCoord chessman_coord_on_screen(UCoord coord) {
		assert(coord.x < map_size.w && coord.y < map_size.h);
		return {
			background_blank_outof_map_size.w + BACKGROUND_BORDER_WIDTH + (coord.x + 1) *
				BACKGROUND_BLANK_BETWEEN_LINES_SIZE.w + coord.x * BACKGROUND_LINE_WIDTH + BACKGROUND_LINE_WIDTH / 2,
			background_blank_outof_map_size.h + BACKGROUND_BORDER_WIDTH + (coord.y + 1) *
				BACKGROUND_BLANK_BETWEEN_LINES_SIZE.h + coord.y * BACKGROUND_LINE_WIDTH + BACKGROUND_LINE_WIDTH / 2
		};
	}
	

	constexpr Area CHESSMAN_AREA = { (BACKGROUND_BLANK_BETWEEN_LINES_SIZE.w + BACKGROUND_LINE_WIDTH) * 3 / 4,
			(BACKGROUND_BLANK_BETWEEN_LINES_SIZE.h + BACKGROUND_LINE_WIDTH) * 3 / 4 };
	/*
	 * Calculate the actual rectangle the chessman occupied on the screen.
	 */
	URect chessman_rect_on_screen(UCoord coord) {
		assert(coord.x < map_size.w && coord.y < map_size.h);
		const UCoord central_point = chessman_coord_on_screen(coord);
		return {
			{central_point.x - CHESSMAN_AREA.w / 2,
				central_point.y - CHESSMAN_AREA.h / 2},
			{CHESSMAN_AREA.w, CHESSMAN_AREA.h}
		};
	}

	constexpr static bool ucoord_in_rect(UCoord coord, URect rect) {
		return coord.x >= rect.coord.x && coord.y >= rect.coord.y
			&& coord.x - rect.coord.x < rect.area.w
			&& coord.y - rect.coord.y < rect.area.h;
	}

	void filledCircleRGBA(SDL_Surface *sur, int x, int y, int radius, Uint32 color) {
		for (int w = 0; w < radius * 2; w++) {
			for (int h = 0; h < radius * 2; h++) {
				int dx = radius - w;
				int dy = radius - h;
				if ((dx*dx + dy*dy) <= (radius * radius)) {
					SDL_Rect r{x + dx, y + dy, 1, 1};
					SDL_FillRect(sur, &r, color);
				}
			}
		}
	}

	void filledCircleRGBA(SDL_Renderer *render, int x, int y, int radius) {
		for (int w = 0; w < radius * 2; w++) {
			for (int h = 0; h < radius * 2; h++) {
				int dx = radius - w;
				int dy = radius - h;
				if ((dx*dx + dy*dy) <= (radius * radius)) {
					SDL_RenderDrawPoint(render, x + dx, y + dy);
				}
			}
		}
	}


	/*
	 * Generate the background surface.
	 * Return the surface handle, which requires to be freed by SDL_FreeSurface manually.
	 */
	SDL_Surface *generate_background_surface(SDL_PixelFormat *format) {
		const auto background_color = SDL_MapRGB(format, BACKGROUND_COLOR.r,
				BACKGROUND_COLOR.g, BACKGROUND_COLOR.b);
		const auto black_color = SDL_MapRGB(format, 50, 50, 50);


		SDL_Surface *background_surface = SDL_CreateRGBSurfaceWithFormat(0, window_size.w, window_size.h, 0, format->format);
		SDL_Rect r = {0, 0, static_cast<int>(window_size.w), static_cast<int>(window_size.h)};
		SDL_FillRect(background_surface, &r, background_color);
	
		r.x = background_blank_outof_map_size.w;
		r.y = background_blank_outof_map_size.h;
		r.w = real_map_size.w;
		r.h = real_map_size.h;
		SDL_FillRect(background_surface, &r, black_color);
		for(size_t x = 0; x < map_size.w + 1; ++x) {
			for(size_t y = 0; y < map_size.h + 1; ++y) {
				r = {
					static_cast<int>(background_blank_outof_map_size.w + BACKGROUND_BORDER_WIDTH + x *
					(BACKGROUND_BLANK_BETWEEN_LINES_SIZE.w + BACKGROUND_LINE_WIDTH)),
					static_cast<int>(background_blank_outof_map_size.h + BACKGROUND_BORDER_WIDTH + y *
					(BACKGROUND_BLANK_BETWEEN_LINES_SIZE.h + BACKGROUND_LINE_WIDTH)),
					BACKGROUND_BLANK_BETWEEN_LINES_SIZE.w, BACKGROUND_BLANK_BETWEEN_LINES_SIZE.h
				};
				SDL_FillRect(background_surface, &r, background_color);
			}
		}
		if(map_size.w > 2 && map_size.h > 2) {
			UCoord coords[4] = {
				chessman_coord_on_screen({2, 2}),
				chessman_coord_on_screen({map_size.w - 3, 2}),
				chessman_coord_on_screen({2, map_size.h - 3}),
				chessman_coord_on_screen({map_size.w - 3, map_size.h - 3}),
			};
			for(UCoord coord : coords) {
				filledCircleRGBA(background_surface, coord.x, coord.y, BACKGROUND_BLANK_BETWEEN_LINES_SIZE.w / 10, black_color);
			}
		}
		return background_surface;
	}


	class Font {
	private:
		/*
		 * Raw font data, with characters !... ~
		 * Each pixel occupies 1 bit; Each character occupies
		 * FONT_CHARACTER_SIZE.w * FONT_CHARACTER_SIZE.h * 1 bits.
		 */
		constexpr static char RAW_FONT_DATA[] =
			"AHAABMAAHAABMAAHAABMAAHAABMAAHAAAAAAAAABMAAHAABMAAAAADJMAOHADJMA"
			"OHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADDAAMMAGGAPPPDPPMDDAAMMAGGAPP"
			"PDPPMBJIAMMADDAAMMAADAAHPMDPPBPPMHDABPPADPOAHPMADDIAMODPPIPPMDPO"
			"AAMADIAFPADGMBJLAMGMGBPDADJIAAMOAGHMDBLBIGMMBLGAHNAAOBPIAPPAHPMB"
			"MHAHBIAMMABPGAPNIHDOBMHAHPOBPPIDMGAGAABMAAHAABMAAHAAAAAAAAAAAAAA"
			"AAAAAAAAAAAAAAAAAAAAAAAAAOAAHIADIAAMAAHAABMAAHAABMAAHAABMAAHAAAO"
			"AABOAADIBMAAHIAAHAAAMAADIAAOAADIAAOAADIAAOAADIABMABOAAHAAAAAAAAA"
			"AAAAAOAADIAAOADPPIPPOAHMADLIBMHAOAMDABAAAAAAAAAAAAAAABMAAHAABMAA"
			"HAAPPMDPPABMAAHAABMAAHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAO"
			"AADIABOAAHIAAAAAAAAAAAAAAAAAAAAAAAAAAPPMDPPAAAAAAAAAAAAAAAAAAAAA"
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADMAAPAADMAABOAAHIABOAAHIADOA"
			"APIADIAAOAAPIADOAAPAADMAAPAADMAAPMADPADAPAMDMPADPMAPPADPMAPPADPM"
			"APDMDAPAMAPMADPAADMAAPAAPMADPAADMAAPAADMAAPAADMAAPAADMAAPADPPMPP"
			"PDPPAPPMPADPMAPAAPMADPAPPADPMDPMAPPAPMADPAAPPPPPPPDPPMPPPAAPAADM"
			"ADMAAPAAPPADPMAADMAAPDADMMAPDPPAPPMADPAAPMAPPADPMDMPAPDMPAPDMDMP"
			"PPPPPPAAPAADMAAPAADMPPPDPPMPAADMAAPPPDPPMAADMAAPAADMAAPPADPMAPDP"
			"PAPPMAPMADPADMAAPAAPAADMAAPPPDPPMPADPMAPPADPMAPDPPAPPMPPPPPPPPAD"
			"PMAPAAPAADMADMAAPAAPAADMAAPAADMAAPAADMADPMAPPAPADDMAMPMDDPAMDPMA"
			"PPAMDPPAPPMADPAAPDPPAPPMDPPAPPMPADPMAPPADPMAPDPPMPPPAADMAAPAAPAA"
			"DMDPMAPPAAAAAAAAAAAAHAABMAAHAAAAAAAAABMAAHAABMAAAAAAAAAAAAAAAAAA"
			"AAAAAHAABMAAHAAAAAAAAABMAAPAADMAAPAAAAAAAAAAAAAAAAABOAAPAAHAADIA"
			"BMAAOAABMAADIAAHAAAPAABOAAAAAAAAAAAAAAAAAABPOAHPIAAAAAAABPOAHPIA"
			"AAAAAAAAAAAAAAAAAAAABOAADMAADIAAHAAAOAABMAAOAAHAADIADMABOAAAAAAA"
			"AABOAAPMAHBIBIGAEBIAAOAAHAADIAAMAADAAAAAADAAAMAAAAABPAAPOAHAMDJP"
			"JMMGHDBJMMGHBPAOAABOBIDPMAHOAAAAAPMADPADMPAPDMPADPMAPPADPMAPPPPP"
			"PPPPADPMAPPADPMAPPPPDPPMPADPMAPPADPMAPPPPDPPMPADPMAPPADPMAPPPPDP"
			"PMAPPADPMDMDMPAPPAADMAAPAADMAAPAADMAADMDMPAPAPPADPMPPMDPPAPAPDMD"
			"MPADPMAPPADPMAPPADPMAPPAPDMDMPPMDPPADPPMPPPDMAAPAADMAAPAADPPAPPM"
			"DMAAPAADMAAPAADPPMPPPPPPPPPPPAADMAAPAADMAAPPPDPPMPAADMAAPAADMAAP"
			"AADMAAAPPMDPPDMAAPAAPAADMAAPAPPMDPPADPMAPDMDMPAPAPPMDPPPADPMAPPA"
			"DPMAPPADPMAPPPPPPPPPADPMAPPADPMAPPADPMAPDPPMPPPADMAAPAADMAAPAADM"
			"AAPAADMAAPAADMAAPADPPMPPPDPPMPPPADMAAPAADMAAPAADMAAPAADMAAPADDMA"
			"MPADPAAPMAPADPMAPPAPDMDMPDMDMPAPPADPMAPPMDPPAPDPDMPMPAPPMDPDMAAP"
			"AADMAAPAADMAAPAADMAAPAADMAAPAADMAAPAADPPMPPPPADPMAPPMPPPDPPPPPPP"
			"PPPPPPPPPDDPMMPPADPMAPPADPMAPPADPMAPPMDPPAPPPDPPMPPPPPPPPPDPPMPP"
			"PAPPMDPPADPMAPDPPAPPMPADPMAPPADPMAPPADPMAPPADPMAPPADPMAPDPPAPPMP"
			"PPDPPMPADPMAPPADPMAPPADPMAPPPPDPPMPAADMAAPAADMAADPMAPPAPADPMAPPA"
			"DPMAPPADPMAPPADPMAPDPPAPPMAADMAAPPPPDPPMPADPMAPPADPMAPPAPPMDPPPM"
			"DPPAPDPDMPMPAPPMDPDPMAPPAPAPDMDMPAADMAADPPAPPMAADMAAPPADPMAPDPPA"
			"PPMDPPMPPPADMAAPAADMAAPAADMAAPAADMAAPAADMAAPAADMAAPAPADPMAPPADPM"
			"APPADPMAPPADPMAPPADPMAPPADPMAPDPPAPPMPADPMAPPADPMAPPADPMAPPMPPPD"
			"PDPPAPPMAPMADPAADAAAMAPADPMAPPADPMAPPDDPMMPPPPPPPPPPPPPPPPMPPPDP"
			"PADPMAPPADPMAPDMPAPDMAPMADPAAPMADPADMPAPDMPADPMAPMAAPAADDMDMPAPD"
			"MDMPAPDMDMPAPAPPADPMADMAAPAADMAAPAADMAAPADPPMPPPAADMAAPAAPAADMAD"
			"MAAPAAPAADMADMAAPAADPPMPPPAAAAAHMABPAAGAABIAAGAABIAAGAABIAAGAABI"
			"AAHMABPAAAAAPAADMAAPAADMAAPIADOAADIAAOAADOAAPIABOAAHIABOAAHIAAAA"
			"PIADOAABIAAGAABIAAGAABIAAGAABIAAGAAPIADOAAAAAAAAAAMAAHIADDABIGAM"
			"AMGABIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAAAPPMDPPAAAAAAAABIAAHAABOAADIAAGAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAAABOAAPIADIAAOAADIABOAAHIAAOAADIAAOAADOAAHIAAAADAAAMAADAAAMAA"
			"DAAAMAADAAAMAADAAAMAADAAAMAADAAAMAAAABOAAHMAAHAABMAAHAABOAAHIABM"
			"AAHAABMABPAAHIAAAAAAAAADIEBPDAMPICBMAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			"AAAA";

		constexpr static Area FONT_CHARACTER_SIZE = {14, 14};
		constexpr static size_t SCALE_TIME = 2;

		constexpr static Area DEFAULT_EXTRA_ADVANCE = {SCALE_TIME, SCALE_TIME};
	public:
		constexpr static size_t CHARACTER_COUNT = '~' - '!' - 26 + 1;
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
		URect render_text(SDL_Renderer *render,
				const string_view string,
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

		constexpr static Area text_size(string_view str, Area extra_advance = DEFAULT_EXTRA_ADVANCE);

		static uint_type get_character_position(char c) {
			assert(c >= '!' && c <= '~' && (c < 'a' || c > 'z'));

			return c < 'a'? c - '!' : c - 26 - '!';
		}

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

		SDL_Color back;

		SDL_Surface *font_surface;
		SDL_Texture *font_texture;
	};
	Font::Font(SDL_PixelFormat *format, SDL_Renderer *render, SDL_Color color) :
		back(color.r == 0 && color.g == 0 && color.b == 0 ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 255}) {

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
	}
	Font::~Font() {
		SDL_FreeSurface(font_surface);
		SDL_DestroyTexture(font_texture);
	}
	URect Font::render_text(SDL_Renderer *render, const string_view str, UCoord pos, Area extra_advance) const {
		SDL_Rect srcrect{0, 0, CHARACTER_SIZE.w, CHARACTER_SIZE.h}, dstrect{0, 0, CHARACTER_SIZE.w, CHARACTER_SIZE.h};
		dstrect.x = pos.x;
		dstrect.y = pos.y;
		URect region{ {pos.x, pos.y}, {0, 1} };
		uint_type region_width_temp = 0;
		for(char c : str) {
			c = toupper(c);
			bool right_advance = false, down_advance = false;
			if(c >= '!' && c <= '~') {
				srcrect.x = get_character_position(c) * CHARACTER_SIZE.w;
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
				if(region_width_temp > region.area.w) {
					region.area.w = region_width_temp;
				}
			}
			if(down_advance) {
				dstrect.x = pos.x;
				dstrect.y += CHARACTER_SIZE.h + extra_advance.h;
				++region.area.h;
				region_width_temp = 0;
			}
		}
		if(region.area.w != 0) {
			region.area.w = region.area.w * (CHARACTER_SIZE.w + extra_advance.w) - extra_advance.w;
		}
		region.area.h = region.area.h * (CHARACTER_SIZE.h + extra_advance.h) - extra_advance.h;
		return region;
	}
	SDL_Surface *Font::create_surface_from_text(SDL_PixelFormat *format, const string_view str, Area extra_advance) const {
		Area surface_size = text_size(str, extra_advance);
		SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, surface_size.w, surface_size.h, 0, format->format);
		SDL_Rect srcrect{0, 0, CHARACTER_SIZE.w, CHARACTER_SIZE.h}, dstrect{0, 0, CHARACTER_SIZE.w, CHARACTER_SIZE.h};
		for(char c : str) {
			c = toupper(c);
			bool right_advance = false, down_advance = false;
			if(c >= '!' && c <= '~') {
				srcrect.x = get_character_position(c) * CHARACTER_SIZE.w;

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

	constexpr Area Font::text_size(string_view str, Area extra_advance) {
		Area area = {0, 0}, text_area = {0, 1};
		uint_type text_width_temp = 0;
		for(char c : str) {
			if(c == '\n') {
				text_area.h += 1;
				text_width_temp = 0;
			} else {
				++text_width_temp;
				if(text_width_temp > text_area.w) text_area.w = text_width_temp;
			}
		}
		if(text_area.w != 0) area.w = text_area.w * (FONT_CHARACTER_SIZE.w * SCALE_TIME + extra_advance.w) - extra_advance.w;
		area.h = text_area.h * (FONT_CHARACTER_SIZE.h * SCALE_TIME + extra_advance.h) - extra_advance.h;
		return area;
	}

	class WidgetManager;

	class Widget {
		friend class WidgetManager;
	public:
		Widget();
		Widget(URect r) : region(r) {}
		virtual ~Widget() {}

		/*
		 * Should be called when a mouse click event occurs.
		 * Requires the relative position of mouse.
		 */
		void on_click(UCoord c) {on_click_function(c);}

		/*
		 * Should be called when a mouse hovers on the widget.
		 * Requires the relative position of mouse.
		 */
		void on_mouse_move_on(UCoord c) {on_mouse_move_on_function(c);}

		void on_mouse_move_out() {on_mouse_move_out_function();}

		void draw(bool mouse_hovering) {draw_function(mouse_hovering);}

	protected:
		URect region;

		virtual void on_click_function(UCoord) = 0;
		virtual void on_mouse_move_on_function(UCoord) = 0;
		virtual void on_mouse_move_out_function() = 0;
		virtual void draw_function(bool mouse_hovering) = 0;
	
	};

	class WidgetManager {
	public:
		WidgetManager() = default;

		template<typename T>
		void register_widget(T &&widget) {
			widgets.emplace_back(widget);
		}

		void draw();

		bool mouse_button_down(UCoord mouse_coord);
		void mouse_move(UCoord mouse_coord);
	private:
		struct WidgetNode {
			template<typename T>
			WidgetNode(T &&widget_): widget(std::make_unique<std::decay_t<T>>(widget_)), mouse_hovering(false) {}

			std::unique_ptr<Widget> widget;
			bool mouse_hovering;
		};
		std::vector<WidgetNode> widgets;
	};

	bool WidgetManager::mouse_button_down(UCoord c) {
		for(WidgetNode &widget_node: widgets) {
			if(ucoord_in_rect(c, widget_node.widget->region)) {
				UCoord coord = widget_node.widget->region.coord;
				widget_node.widget->on_click({c.x - coord.x, c.y - coord.y});
				return true;
			}
		}
		return false;
	}
	void WidgetManager::mouse_move(UCoord c) {
		for(WidgetNode &widget_node: widgets) {
			if(ucoord_in_rect(c, widget_node.widget->region)) {
				widget_node.mouse_hovering = true;
				UCoord coord = widget_node.widget->region.coord;
				widget_node.widget->on_mouse_move_on({c.x - coord.x, c.y - coord.y});
			} else {
				if(widget_node.mouse_hovering) {
					widget_node.mouse_hovering = false;
					widget_node.widget->on_mouse_move_out();
				}
			}
		}
	}
	void WidgetManager::draw() {
		for(WidgetNode &widget_node: widgets) {
			widget_node.widget->draw(widget_node.mouse_hovering);
		}
	}



	class Button : public Widget {
		friend class ButtonManager;

		using on_click_callback_t = void(UCoord);
	public:
		Button(SDL_Renderer *render_, const Font &font_, std::string_view button_title, const URect button_region) :
			Widget(button_region), title(button_title), render(render_), font(font_) {}
		Button(SDL_Renderer *render_, const Font &font_, Button &&b) :
			Widget(b.region),
			title(std::move(b.title)),
			on_click_callback(std::move(b.on_click_callback)),
			render(render_),
			font(font_) {}

		Button(const Button &button) :
			Widget(button.region), title(button.title), on_click_callback(button.on_click_callback), render(button.render), font(button.font) {}
		Button &operator=(const Button &) = delete;

		void set_on_click(std::function<on_click_callback_t> func) {
			on_click_callback = std::move(func);
		}
	protected:
		virtual void on_click_function(UCoord c) override {
			on_click_callback(c);
		}

		virtual void on_mouse_move_on_function(UCoord) override {}

		virtual void on_mouse_move_out_function() override {}

		virtual void draw_function(bool mouse_hovering) override {
			font.render_text(render, title, {
				region.coord.x + region.area.w / 2 - Font::text_size(title).w / 2,
				region.coord.y
			});
			// SDL_SetRenderDrawColor(render, BUTTON_BORDER_COLOR.r,
			// 		BUTTON_BORDER_COLOR.g,
			// 		BUTTON_BORDER_COLOR.b,
			// 		BUTTON_BORDER_COLOR.a);
			// SDL_RenderDrawRect(render, &region);
			if(mouse_hovering) {
				SDL_SetRenderDrawColor(render, 255, 255, 255, 60);
				SDL_Rect rect = region;
				SDL_RenderFillRect(render, &rect);
			}

		}
		
	private:
		std::string_view title;
		std::function<on_click_callback_t> on_click_callback;

		SDL_Renderer *render;
		const Font &font;
	};

	/*
	 * Frontend with SDL2.
	 */
        class Game {
	public:
		Game();
		~Game();
		Game(const Game &) = delete;
		Game &operator=(const Game &) = delete;

		void start();
	private:
		SDL_Window *window;
		SDL_Renderer *render;
		SDL_Texture *background_texture;
		Font *font;
		WidgetManager widget_manager;

		SDL_Texture *black_chessman_texture, *black_chessman_transparent_texture;
		SDL_Texture *white_chessman_texture, *white_chessman_transparent_texture;

		bool request_stop;


		CoreGame game;

	};

	Game::Game() {
		request_stop = false;
		// Initialize SDL2
		if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
			log_error("Error initializing SDL2: %s.", SDL_GetError());
			exit(1);
		}
		window = SDL_CreateWindow("Gobang", SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					window_size.w,
					window_size.h,
					SDL_WINDOW_SHOWN);
		if(!window) {
			log_error("Error occurred creating the main window: %s.", SDL_GetError());
			exit(1);
		}
		SDL_Surface *screen;
		if(software_rendering) {
			screen = SDL_GetWindowSurface(window);
			render = SDL_CreateSoftwareRenderer(screen);
			if(!render) {
				log_error("Can't create software renderer.");
				exit(1);
			}
		} else {
			render = SDL_CreateRenderer(window, -1,
					SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			screen = SDL_GetWindowSurface(window);
			if(!render) {
				log_error("Warning: cannot create renderer with hardware acceleration: %s.", SDL_GetError());
				render = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
				if(!render) {
					log_error("Can't create renderer.");
					exit(1);
				}
			}
		}
		SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);

		SDL_Surface *background_surface = generate_background_surface(screen->format);
		background_texture = SDL_CreateTextureFromSurface(render, background_surface);
		SDL_FreeSurface(background_surface);

		font = new Font(screen->format, render, {0x20, 0x20, 0x20, 0xFF});


		constexpr Area reset_area = Font::text_size("reset");
		Button reset(render, *font, "reset", {
			{ DEFAULT_BACKGROUND_BLANK_OUTOF_MAP_SIZE.w,
				background_blank_outof_map_size.h * 4 / 3 + real_map_size.h },
			{ reset_area.w, reset_area.h }
		});
		reset.set_on_click([this] (UCoord) {
			game.clear();
		});
		widget_manager.register_widget(reset);

		constexpr Area exit_area = Font::text_size("exit");
		Button exit(render, *font, "exit", {
			{ window_size.w - DEFAULT_BACKGROUND_BLANK_OUTOF_MAP_SIZE.w - exit_area.w,
				background_blank_outof_map_size.h * 4 / 3 + real_map_size.h },
			{exit_area.w, exit_area.h}
		});
		exit.set_on_click([this](UCoord) {
			request_stop = true;
		});
		widget_manager.register_widget(exit);



		SDL_Surface *sur = SDL_CreateRGBSurface(0, CHESSMAN_AREA.w, CHESSMAN_AREA.h, 32,
				0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		SDL_FillRect(sur, nullptr, SDL_MapRGBA(sur->format, 255, 255, 255, 0));
		SDL_Renderer *sur_render = SDL_CreateSoftwareRenderer(sur);

		// BLACK CHESSMAN TEXTURE
		SDL_SetRenderDrawColor(sur_render, BLACK_CHESSMAN_COLOR.r,
				BLACK_CHESSMAN_COLOR.g, BLACK_CHESSMAN_COLOR.b, BLACK_CHESSMAN_COLOR.a);
		filledCircleRGBA(sur_render, CHESSMAN_AREA.w / 2, CHESSMAN_AREA.h / 2, CHESSMAN_AREA.w / 2);
		black_chessman_texture = SDL_CreateTextureFromSurface(render, sur);

		// BLACK CHESSMAN TRANSPARENT TEXTURE
		SDL_SetRenderDrawColor(sur_render, BLACK_CHESSMAN_COLOR.r,
				BLACK_CHESSMAN_COLOR.g, BLACK_CHESSMAN_COLOR.b, 160);
		filledCircleRGBA(sur_render, CHESSMAN_AREA.w / 2, CHESSMAN_AREA.h / 2, CHESSMAN_AREA.w / 2);
		black_chessman_transparent_texture = SDL_CreateTextureFromSurface(render, sur);

		// WHITE CHESSMAN TEXTURE
		SDL_SetRenderDrawColor(sur_render, WHITE_CHESSMAN_COLOR.r,
				WHITE_CHESSMAN_COLOR.g, WHITE_CHESSMAN_COLOR.b, WHITE_CHESSMAN_COLOR.a);
		filledCircleRGBA(sur_render, CHESSMAN_AREA.w / 2, CHESSMAN_AREA.h / 2, CHESSMAN_AREA.w / 2);
		white_chessman_texture = SDL_CreateTextureFromSurface(render, sur);

		// WHITE CHESSMAN TRANSPARENT TEXTURE
		SDL_SetRenderDrawColor(sur_render, WHITE_CHESSMAN_COLOR.r,
				WHITE_CHESSMAN_COLOR.g, WHITE_CHESSMAN_COLOR.b, 190);
		filledCircleRGBA(sur_render, CHESSMAN_AREA.w / 2, CHESSMAN_AREA.h / 2, CHESSMAN_AREA.w / 2);
		white_chessman_transparent_texture = SDL_CreateTextureFromSurface(render, sur);

		SDL_DestroyRenderer(sur_render);
		SDL_FreeSurface(sur);
	}

	Game::~Game() {
		delete font;

		SDL_DestroyTexture(black_chessman_texture);
		SDL_DestroyTexture(black_chessman_transparent_texture);
		SDL_DestroyTexture(white_chessman_texture);
		SDL_DestroyTexture(white_chessman_transparent_texture);

		SDL_DestroyRenderer(render);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void Game::start() {
		while(!request_stop) {
			SDL_SetRenderDrawColor(render, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 255);
			SDL_RenderCopy(render, background_texture, nullptr, nullptr);

			UCoord mouse_coord; // Get mouse state.
			{
				int x, y;
				SDL_GetMouseState(&x, &y);
				mouse_coord.x = x;
				mouse_coord.y = y;
			}

			bool mouse_down = false; // Handle events.
			SDL_Event event;
			while(SDL_PollEvent(&event)) {
				switch(event.type) {
					case SDL_QUIT:
						return;
					case SDL_MOUSEBUTTONDOWN:
						mouse_down = true;
						break;
				}
			}

			bool selected_chessman = false;
			UCoord selected_chessman_coord;
			for(uint_type y = 0; y < map_size.h; ++y) {
				for(uint_type x = 0; x < map_size.w; ++x) {
					CoreGame::Unit unit = game[{x, y}];
					URect chessman_rect = chessman_rect_on_screen({x, y});
					SDL_Rect r = chessman_rect;
					// SDL_SetRenderDrawColor(render, 0, 0, 255, 255);
					// SDL_RenderFillRect(render, &chessman_rect);
					if(ucoord_in_rect(mouse_coord, chessman_rect)) {
						selected_chessman = true;
						selected_chessman_coord = {x, y};
					}

					if(unit == CoreGame::Unit::EMPTY) {
						if(selected_chessman && selected_chessman_coord == UCoord{x, y}) {
							if(game.is_white_turn())
								SDL_RenderCopy(render, white_chessman_transparent_texture, nullptr, &r);
							else
								SDL_RenderCopy(render, black_chessman_transparent_texture, nullptr, &r);
						}
					} else {
						if(unit == CoreGame::Unit::WHITE)
							SDL_RenderCopy(render, white_chessman_texture, nullptr, &r);
						else
							SDL_RenderCopy(render, black_chessman_texture, nullptr, &r);
					}
				}
			}


			widget_manager.mouse_move(mouse_coord);
			if(mouse_down) {
				if(!widget_manager.mouse_button_down(mouse_coord)) {
					if(game.status() == CoreGame::Status::NONE) {
						if(selected_chessman && game[selected_chessman_coord] == CoreGame::Unit::EMPTY) {
							game.place(selected_chessman_coord);
						}
					}
				}
			}

			uint_type TEXT_Y_POS = background_blank_outof_map_size.h / 3;
			if(game.status() == CoreGame::Status::NONE) {
				const char *prompt = game.is_white_turn() ? "white's turn" : "black's turn";
				const Area prompt_size = Font::text_size(prompt);
				font->render_text(render, prompt, {window_size.w / 2 - prompt_size.w / 2, TEXT_Y_POS});
			} else { //Some one won
				std::pair<UCoord, UCoord> rows = {
					chessman_coord_on_screen(game.get_rows().first),
					chessman_coord_on_screen(game.get_rows().second)
				};
				SDL_SetRenderDrawColor(render, 255, 100, 100, 255);
				SDL_RenderDrawLine(render, rows.first.x, rows.first.y, rows.second.x, rows.second.y);
				const char *prompt = game.is_white_turn() ? "white won!" : "black won!";
				const Area prompt_size = Font::text_size(prompt);
				font->render_text(render, prompt, {window_size.w / 2 - prompt_size.w / 2, TEXT_Y_POS});
			}


			SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
			widget_manager.draw();

			if(software_rendering) {
				SDL_UpdateWindowSurface(window);
			} else {
				SDL_RenderPresent(render);
			}
			SDL_Delay(1);
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
		for(uint_type i = 0; i < map_size.w * 2; ++i) putchar('-');
		putchar('|');
		putchar('\n');
		for(uint_type y = 0; y < map_size.h; ++y) {
			putchar('|');
			for(uint_type x = 0; x < map_size.w; ++x) {
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
					default:
						color = EMPTY_COLOR; //AVOID STUPID WARNING FROM GCC
				}
				if(x == 0 || g[{x - 1, y}] != g[{x, y}]) background_color(color);
				printf("  ");
				if(x == map_size.w - 1 || g[{x, y}] != g[{x + 1, y}]) color_reset();
			}
			putchar('|');
			putchar('\n');
		}
		putchar('|');
		for(uint_type i = 0; i < map_size.w * 2; ++i) putchar('-');
		putchar('|');
		putchar('\n');
	}
	/*
	 * Print the difference between the output of ``print(game)`` and ``print(bufgame)``.
	 * Ensure the position of cursor is topleft before calling.
	 * The position of cursor is set to {0, MAP_SIZE.h + 2} after a call completes.
	 */
	void print_diff(const CoreGame &game, const CoreGame &bufgame) {
		for(uint_type x = 0; x < map_size.w; ++x) {
			for(uint_type y = 0; y < map_size.h; ++y) {
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
						default:
							color = EMPTY_COLOR; //AVOID STUPID WARNING FROM GCC AS WELL
					}
					cursor_gotoxy({x * 2 + 1, y + 1});
					background_color(color);
					printf("  ");
					color_reset();
				}
			}
		}
		cursor_gotoxy({0, map_size.h + 2});
	}

	/*
	 * Print selection.
	 * The position of cursor is set to {0, MAP_SIZE.h + 2} after a call completes.
	 */
	void print_selection(const CoreGame &game, UCoord selection_pos, bool has_old_selection_pos = false, UCoord old_selection_pos = {0, 0}) {
		assert(selection_pos.x < map_size.w || selection_pos.y < map_size.h);
		if(game[selection_pos] != CoreGame::Unit::EMPTY) {
			cursor_gotoxy({0, map_size.h + 2});
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
		cursor_gotoxy({0, map_size.h + 2});
	}


	class Game {
	public:
		Game();
		~Game() = default;

		void start();

	private:

		CoreGame game, bufgame;
		UCoord selection_pos, buf_selection_pos;
	};

	Game::Game() {
		selection_pos = {map_size.w / 2, map_size.h / 2};
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
			Key key = Key::UP; //Why is this defaultly set with Key::UP? to AVOID STUPID WARNING FROM GCC ONCE AGAIN
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
					if(iter.x == map_size.w) {
						iter.x = 0;
						++iter.y;
						if(iter.y == map_size.h) {
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
					return c.x < map_size.w && c.y < map_size.h;
				};
				auto available = [this](UCoord c) -> bool {
					return c.x < map_size.w && c.y < map_size.h && game[c] == CoreGame::Unit::EMPTY;
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

/*
 * Process command line.
 * Return: 0 for success, a non-zero integer for failures.
 */
int process_argument(size_t argc, char **argv) {
	ArgumentProcessor ap;

	Argument help, map_size_arg, rows, switch_mode, enable_software_rendering;

	help.add_name("-h").add_name("--help").add_name("--usage");
	help.set_argc(0);
	help.set_description("Display this help and exit.");
	help.set_act_func([&ap](char **argv) {
		ap.output_help({argv[0], ": Gobang, can either run with SDL2 or in terminal.\nRun in terminal in default.\nConsole mode: WASDQR<Enter><Space>"});
		exit(0);
	});

	map_size_arg.add_name("-s").add_name("--map-size");
	map_size_arg.set_argc(2);
	map_size_arg.set_description("Specify the size of map.");
	map_size_arg.set_act_func([](char **argv) {
		bool success;
		int i = parse_int(argv[0], &success);

		if(!success) {
			log_error("Require an integer(\"%s\").", argv[0]);
			exit(1);
		}
		if(i <= 0) {
			log_error("Require an integer greater than 0(\"%d\").", i);
			exit(1);
		}
		map_size.w = i;

		i = parse_int(argv[1], &success);
		if(!success) {
			log_error("Require an integer(\"%s\").", argv[0]);
			exit(1);
		}
		if(i <= 0) {
			log_error("Require an integer greater than 0(\"%d\").", i);
			exit(1);
		}
		map_size.h = i;

		uint_type limit = map_size.w < map_size.h ? map_size.h : map_size.w;
		if(amount_of_rows > limit) amount_of_rows = limit;
	});

	rows.add_name("-a").add_name("--amount");
	rows.set_argc(1);
	rows.set_description("Specify the amount of chessman in a rows that is enough to win.");
	rows.set_act_func([](char **argv) {
		bool success;
		int i = parse_int(argv[0], &success);

		if(!success) {
			log_error("Require an integer(\"%s\").", argv[0]);
			exit(1);
		}

		const int long_limit = map_size.w < map_size.h ? map_size.h : map_size.w;
		if(i <= 0 || i > long_limit) {
			log_error("Require an integer from 1 to %d(\"%d\").", long_limit, i);
			exit(1);
		}
		amount_of_rows = i;
	});

	switch_mode.add_name("-m").add_name("--mode");
	switch_mode.set_argc(1);
	switch_mode.set_description("Set the display mode of gobang. Possible option: console, graphic, all.");
	switch_mode.set_act_func([argv] (char **argvv) {
		if(strcmp(argvv[0], "console") == 0) {
			mode = Mode::console;
		} else if(strcmp(argvv[0], "graphic") == 0) {
			mode = Mode::graphic;
		} else if(strcmp(argvv[0], "all") == 0) {
			mode = Mode::all;
		} else {
			log_error("Type \"%s --help\" for usage.", argv[0]);
			exit(1);
		}
	});

	enable_software_rendering.add_name("--enable-software-rendering");
	enable_software_rendering.set_argc(0);
	enable_software_rendering.set_description("Enable software rendering. Available in graphic(all) mode.");
	enable_software_rendering.set_act_func([] (char **) {
		software_rendering = true;
	});

	ap.register_argument(help);
	ap.register_argument(map_size_arg);
	ap.register_argument(rows);
	ap.register_argument(switch_mode);
	ap.register_argument(enable_software_rendering);

	return ap.process(argc, argv) ? 0 : 1;
}

int main(int argc, char **argv) {
	if(process_argument(argc, argv) != 0) {
		return 1;
	}
	if(mode == Mode::console) {
		frontend_with_console::Game g;
		g.start();
	} else if(mode == Mode::graphic) {
		frontend_with_SDL2::calculate();
		frontend_with_SDL2::Game g;
		g.start();
	} else if(mode == Mode::all) {
		auto console_func = []() {
			frontend_with_console::Game g;
			g.start();
		};
		std::thread console_thread(console_func);

		{
			frontend_with_SDL2::calculate();
			frontend_with_SDL2::Game g;
			g.start();
		}

		console_thread.join();
	}
	return 0;
}
