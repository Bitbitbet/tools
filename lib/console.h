#ifndef ___CONSOLE__H___
#define ___CONSOLE__H___

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <utils.h>

namespace console {
	struct Area;
	struct Color3;
	struct Color {uint8_t r, g, b, a; operator Color3() const;};
	struct Color3 {uint8_t r, g, b; operator Color() const;};

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

	inline Color3::operator Color() const {return {r, g, b, 0b11111111};}
	inline Color::operator Color3() const {return {r, g, b};}

	constexpr inline bool operator==(Coord lfs, Coord rfs) {return lfs.x == rfs.x && lfs.y == rfs.y;}
	constexpr inline bool operator==(UCoord lfs, UCoord rfs) {return lfs.x == rfs.x && lfs.y == rfs.y;}
	constexpr inline bool operator==(Color lfs, Color rfs) {
		return lfs.r == rfs.r &&
			lfs.g == rfs.g &&
			lfs.b == rfs.b &&
			lfs.a == rfs.a;
	}
	constexpr bool operator==(Color3 lfs, Color3 rfs) {
		return lfs.r == rfs.r &&
			lfs.g == rfs.g &&
			lfs.b == rfs.b;
	}

	constexpr inline bool operator!=(Coord lfs, Coord rfs) {return !(lfs == rfs);}
	constexpr inline bool operator!=(UCoord lfs, UCoord rfs) {return !(lfs == rfs);}
	constexpr inline bool operator!=(Color lfs, Color rfs) {return !(lfs == rfs);}
	constexpr inline bool operator!=(Color3 lfs, Color3 rfs) {return !(lfs == rfs);}


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
