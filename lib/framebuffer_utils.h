#ifndef __WIN32
#ifndef __FRAMEBUFFER_UTILS_H__
#define __FRAMEBUFFER_UTILS_H__

#include <string>
#include <linux/fb.h>
#include <cstdint>
#include <utils.h>

class FramebufferError : std::exception {
	friend class Framebuffer;
public:
	virtual const char *what() const noexcept override {return msg.c_str();}
private:
	FramebufferError(std::string_view msg_) : std::exception(), msg(msg_) {}

	std::string msg;
};

/*
 * O-------------> x
 * |
 * |
 * |
 * |
 * |
 * \/
 * y
 */

class Framebuffer {
private:
	bool if_blend;
	bool if_nobuffer;
	bool m_valid;

	int fbfd;
	unsigned char *data, *buffer;
	Area fbsize;
	decltype(std::declval<fb_fix_screeninfo>().smem_len) smem_len;
	decltype(std::declval<fb_var_screeninfo>().bits_per_pixel) bytes_per_pixel;
	decltype(std::declval<fb_fix_screeninfo>().line_length) line_length;
public:
	Framebuffer() : if_blend(false), if_nobuffer(false), m_valid(false), fbfd(0), data(nullptr), buffer(nullptr) {}
	Framebuffer(std::string_view device_name, bool nobuffer = false);
	Framebuffer(const Framebuffer &) = delete;
	Framebuffer(Framebuffer &&);
	~Framebuffer();
	Framebuffer &operator=(const Framebuffer &) = delete;
	Framebuffer &operator=(Framebuffer &&) &;
	bool valid() const {return m_valid;}
	bool nobuffer() const {return if_nobuffer;}
	void set_blend_mode(bool if_blend) {this->if_blend = if_blend;}
	bool get_blend_mode() const {return if_blend;}
	Area size() const;
	Color get(UCoord pos) const;

	void update();
	void reset_buffer();

	void set(UCoord pos, Color c);
	void fill(Color c);
	void draw_rectangle(UCoord, Area, Color);
	void fill_rectangle(UCoord, Area, Color);
	void draw_line(UCoord, UCoord, Color);
};

#endif
#endif
