#ifndef __WIN32
#ifndef __FRAMEBUFFER_UTILS_H__
#define __FRAMEBUFFER_UTILS_H__

#include <string>
#include <linux/fb.h>
#include <cstdint>

struct Coord {
	using coord_type = int32_t;
	coord_type x, y;
};

struct UCoord {
	using coord_type = uint32_t;
	coord_type x, y;
};

struct Area {
	using area_type = uint32_t;
	area_type width, height;
};

struct Color {
	uint8_t r = 0, g = 0, b = 0, a = 255;
};

class Framebuffer {
private:
	bool is_bind = false, if_blend = false;
	int fbfd;
	fb_var_screeninfo vinfo;
	fb_fix_screeninfo finfo;
	unsigned char *data, *buffer;

	Area fbsize;
	int bytes_per_pixel;

	mutable int error_code;
public:
	Framebuffer() = default;
	Framebuffer(const std::string &device_name);
	Framebuffer(const char *device_name);
	~Framebuffer();
	bool bind() const;
	bool bind(const std::string &device_name);
	bool bind(const char *device_name);
	bool unbind();
	void set_if_blend(bool if_blend);
	bool get_if_blend();
	Area size() const;
	bool get(UCoord pos, Color &output) const;

	void update();
	void reset_buffer();

	bool set(UCoord pos, Color c);
	bool fill(Color c);
	//void draw_line(int x1, int y1, int x2, int y2);
	void draw_rectangle(UCoord, Area, Color);
	void fill_rectangle(UCoord, Area, Color);

	const char *get_error_message();
};

#endif
#endif
