#include "console.h"

#include <iostream>

using std::cout;
using std::flush;

const char *get_colorenum_id(console::ColorEnum c) {
	using console::ColorEnum;
	switch(c) {
		case ColorEnum::RED:
			return "1";
		case ColorEnum::GREEN:
			return "2";
		case ColorEnum::BLUE:
			return "4";
		case ColorEnum::PURPLE:
			return "5";
		case ColorEnum::CYAN:
			return "6";
		case ColorEnum::WHITE:
			return "7";
		case ColorEnum::BLACK:
			return "0";
	}
	return "";
}
using console::ColorEnum;
ColorEnum fore_color, back_color;
bool fore_unknown = true, back_unknown = true, reset = false;

namespace console {

	void cursor_reset() {
		cout << "\033[1;1H" << flush;
	}
	void cursor_gotoxy(UCoord coord) {
		cout << "\033[" << coord.y + 1 << ";" << coord.x + 1 << "H" << flush;
	}
	void cursor_move(Coord coord) {
		if(coord == Coord{0, 0}) return;
		if(coord.x == 0) {cursor_down(coord.y); return;}
		if(coord.y == 0) {cursor_right(coord.x); return;}
		cout << "\033[";
		if(coord.x > 0) cout << coord.x << "C";
		else cout << -coord.x << "D";
		cout << "\033[";
		if(coord.y > 0) cout << coord.y << "A";
		else cout << -coord.y << "B";
		cout << flush;
	}
	void cursor_move(UCoord coord) {
		if(coord == UCoord{0, 0}) return;
		if(coord.x == 0) {cursor_down(coord.y); return;}
		if(coord.y == 0) {cursor_right(coord.x); return;}
		cout << "\033[" <<  coord.x << "C" << "\033[" << coord.y << "B";
		cout << flush;
	}
	void cursor_down(int_type i) {
		if(i > 0) {
			cout << "\033[" << i << "B" << flush;
		} else if(i < 0) {
			cout << "\033[" << -i << "A" << flush;
		}
	}
	void cursor_right(int_type i) {
		if(i > 0) {
			cout << "\033[" << i << "C" << flush;
		} else if(i < 0) {
			cout << "\033[" << -i << "D" << flush;
		}
	}
	void cursor_pos_save() {
		cout << "\033[s" << flush;
	}
	void cursor_pos_reload() {
		cout << "\033[u" << flush;
	}
	Coord cursor_pos();
	void cursor_set_visible(bool b) {
		cout << (b ? "\033[?25h" : "\033[?25l") << flush;
	}

	void screen_empty() {
		cout << "\033[2J" << flush;
	}
	void screen_clear() {
		cout << "\033[1;1H" << "\033[2J" << flush;
	}

	void foreground_color(ColorEnum c) {
		reset = false;
		if(fore_unknown) {
			fore_unknown = false;
		} else if(c == fore_color) {
			return;
		}
		fore_color = c;
		cout << "\033[3" << get_colorenum_id(c) << "m" << flush;
	}
	void background_color(ColorEnum c) {
		reset = false;
		if(back_unknown) {
			back_unknown = false;
		} else if(c == back_color) {
			return;
		}
		back_color = c;
		cout << "\033[4" << get_colorenum_id(c) << "m" << flush;
	}

	void foreground_color(Color);
	void background_color(Color);

	void color(ColorEnum fore, ColorEnum back) {
		reset = false;
		bool override_fore = true, override_back = true;
		if(!fore_unknown && fore_color == fore) override_fore = false;
		if(!back_unknown && back_color == back) override_back = false;
		fore_unknown = false;
		back_unknown = false;
		fore_color = fore;
		back_color = back;

		if(override_fore && override_back) {
			cout << "\033[3" << get_colorenum_id(fore) << ";4" << get_colorenum_id(back) << "m" << flush;
		} else if(override_fore) {
			cout << "\033[3" << get_colorenum_id(fore) << "m" << flush;
		} else if(override_back) {
			cout << "\033[4" << get_colorenum_id(back) << "m" << flush;
		}
	}
	void color_reset() {
		if(!reset) {
			cout << "\033[0m" << flush;
			reset = true;
			fore_unknown = true;
			back_unknown = true;
		}
	}
	auto ArrowKeyPraser::operator()(unsigned char c) -> std::pair<Status, Key> {
		if(c == '\033' && arrow_key_level == 0) {//UNIX CONSOLE ARROW KEY
			arrow_key_level = 1;
			return {Status::MATCHING, Key::DOWN};
		} else if(c == '\x5B' && arrow_key_level == 1) {
			arrow_key_level = 2;
			return {Status::MATCHING, Key::DOWN};
		} else if('A' <= c && 'D' >= c && arrow_key_level == 2) {
			arrow_key_level = 0;
			switch(c) {
				case 'A':return {Status::MATCH, Key::UP};
				case 'B':return {Status::MATCH, Key::DOWN};
				case 'C':return {Status::MATCH, Key::RIGHT};
				case 'D':return {Status::MATCH, Key::LEFT};
			}
		} else {
			arrow_key_level = 0;
			return {Status::MISMATCH, Key::DOWN};
		}
		return {};
	}
}
