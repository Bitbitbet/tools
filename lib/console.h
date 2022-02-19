#ifndef ___CONSOLE__H___
#define ___CONSOLE__H___

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <utils.h>

namespace console {
	enum class ColorEnum : int8_t {RED, GREEN, BLUE, PURPLE, CYAN, WHITE, BLACK};

	void cursor_reset();
	void cursor_gotoxy(UCoord);
	void cursor_move(Coord);
	void cursor_move(UCoord);
	void cursor_down(int_type);
	void cursor_right(int_type);
	void cursor_pos_save();
	void cursor_pos_reload();
	Coord cursor_pos();
	void cursor_set_visible(bool);

	void screen_empty();
	void screen_clear();

	void foreground_color(Color);
	void background_color(Color);

	void foreground_color(ColorEnum);
	void background_color(ColorEnum);
	void color(ColorEnum fore, ColorEnum back);
	void color_reset();

	constexpr inline bool operator==(Coord lfs, Coord rfs) {return lfs.x == rfs.x && lfs.y == rfs.y;}
	constexpr inline bool operator==(UCoord lfs, UCoord rfs) {return lfs.x == rfs.x && lfs.y == rfs.y;}

	constexpr inline bool operator!=(Coord lfs, Coord rfs) {return !(lfs == rfs);}
	constexpr inline bool operator!=(UCoord lfs, UCoord rfs) {return !(lfs == rfs);}


	enum class Key : uint8_t {
		UP, DOWN, LEFT, RIGHT
	};
	struct ArrowKeyPraser {
	public:
		enum class Status : uint8_t {
			MATCHING = 0, MISMATCH, MATCH
		};
		std::pair<Status, Key> operator()(unsigned char c);
	private:
		uint8_t arrow_key_level = 0;
	};
}

#endif
