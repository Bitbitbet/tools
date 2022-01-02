#include "console.h"

#include <windows.h>
#include <conio.h>
#include <cstdlib>

using console::ColorEnum;

WORD get_foreground_color_id(ColorEnum c) {
	switch(c) {
		case ColorEnum::RED:
			return FOREGROUND_RED;
		case ColorEnum::GREEN:
			return FOREGROUND_GREEN;
		case ColorEnum::BLUE:
			return FOREGROUND_BLUE;
		case ColorEnum::PURPLE:
			return FOREGROUND_RED | FOREGROUND_BLUE;
		case ColorEnum::CYAN:
			return FOREGROUND_GREEN | FOREGROUND_BLUE;
		case ColorEnum::WHITE:
			return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		case ColorEnum::BLACK:
			return 0x0;
	}
	throw "";
	return FOREGROUND_RED;
}

WORD get_background_color_id(ColorEnum c) {
	switch(c) {
		case ColorEnum::RED:
			return BACKGROUND_RED;
		case ColorEnum::GREEN:
			return BACKGROUND_GREEN;
		case ColorEnum::BLUE:
			return BACKGROUND_BLUE;
		case ColorEnum::PURPLE:
			return BACKGROUND_RED | BACKGROUND_BLUE;
		case ColorEnum::CYAN:
			return BACKGROUND_GREEN | BACKGROUND_BLUE;
		case ColorEnum::WHITE:
			return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
		case ColorEnum::BLACK:
			return 0x0;
	}
	throw "";
	return BACKGROUND_GREEN;
}

namespace console {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord_saved;
	ColorEnum fore_color = ColorEnum::WHITE, back_color = ColorEnum::BLACK;


	void cursor_reset() {
		SetConsoleCursorPosition(handle, {0, 0});
	}
	void cursor_gotoxy(UCoord coord) {
		COORD c;
		c.X = coord.x;
		c.Y = coord.y;
		SetConsoleCursorPosition(handle, c);
	}
	void cursor_move(Coord coord) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		COORD c = info.dwCursorPosition;
		c.X += coord.x;
		c.Y += coord.y;
		SetConsoleCursorPosition(handle, c);
	}
	void cursor_move(UCoord coord) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		COORD c = info.dwCursorPosition;
		c.X += coord.x;
		c.Y += coord.y;
		SetConsoleCursorPosition(handle, c);
	}
	void cursor_down(int_type i) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		COORD c = info.dwCursorPosition;
		c.Y += i;
		SetConsoleCursorPosition(handle, c);
	}
	void cursor_right(int_type i) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		COORD c = info.dwCursorPosition;
		c.X += i;
		SetConsoleCursorPosition(handle, c);
	}
	void cursor_pos_save() {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		coord_saved = info.dwCursorPosition;
	}
	void cursor_pos_reload() {
		SetConsoleCursorPosition(handle, coord_saved);
	}
	Coord cursor_pos() {
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		COORD c = info.dwCursorPosition;
		return {c.X, c.Y};
	}

	void screen_empty();
	void screen_clear() {
		system("cls");
	}

	void foreground_color(Color);
	void background_color(Color);

	void foreground_color(ColorEnum c) {
		SetConsoleTextAttribute(handle, get_foreground_color_id(c) | get_background_color_id(back_color));
		fore_color = c;
	}
	void background_color(ColorEnum c) {
		SetConsoleTextAttribute(handle, get_foreground_color_id(fore_color) | get_background_color_id(c));
		back_color = c;
	}
	void color(ColorEnum fore, ColorEnum back) {
		SetConsoleTextAttribute(handle, get_foreground_color_id(fore) | get_background_color_id(back));
		fore_color = fore;
		back_color = back;
	}
	void color_reset() {
		color(ColorEnum::WHITE, ColorEnum::BLACK);
	}
	auto ArrowKeyPraser::operator()(unsigned char c) -> std::pair<Status, Key> {
		if(c == 224 && arrow_key_level == 0) {//WINDOWS CONSOLE ARROW KEY
			arrow_key_level = 1;
			return {ArrowKeyPraser::Status::MATCHING, Key::DOWN};
		} else if(('H' == c || 'P' == c || 'K' == c || 'M' == c) && arrow_key_level == 1) {
			arrow_key_level = 0;
			switch(c) {
				case 'H':
					return {ArrowKeyPraser::Status::MATCH, Key::UP};
				case 'P':
					return {ArrowKeyPraser::Status::MATCH, Key::DOWN};
				case 'M':
					return {ArrowKeyPraser::Status::MATCH, Key::RIGHT};
				case 'K':
					return {ArrowKeyPraser::Status::MATCH, Key::LEFT};
			}
		} else {
			arrow_key_level = 0;
			return {ArrowKeyPraser::Status::MISMATCH, Key::DOWN};
		}
		return {};
	}
}
