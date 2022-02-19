#ifndef __WIN32
#include "framebuffer_utils.h"


#include <string_view>
#include <memory>
#include <cstring>
#include <cassert>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

using std::string_view;
using std::unique_ptr;
using std::swap;

#define RELEASE_RESOURCE \
	if(data) munmap(data, smem_len); \
	if(fbfd) close(fbfd); \
	delete[] buffer;

Framebuffer::Framebuffer(string_view device_name_view, bool nobuffer) : Framebuffer() {
	{
		unique_ptr<char[]> device_name{ new char[device_name_view.size() + 1] };
		memcpy(device_name.get(), device_name_view.data(), device_name_view.size() * sizeof(char));
		device_name[device_name_view.size()] = '\0';

		fbfd = open(device_name.get(), O_RDWR);
	}
	if(fbfd < 0) {
		throw FramebufferError(strerror(errno));
	}
	{
		fb_var_screeninfo vinfo;
		fb_fix_screeninfo finfo;
		if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
			close(fbfd);
			throw FramebufferError(strerror(errno));
		}
		if(ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) < 0) {
			close(fbfd);
			throw FramebufferError(strerror(errno));
		}
		smem_len = finfo.smem_len;
		line_length = finfo.line_length;
		fbsize = {vinfo.xres, vinfo.yres};
		bytes_per_pixel = vinfo.bits_per_pixel / 8;
	}
	data = (uint8_t *)mmap(0, smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if(data == MAP_FAILED) {
		close(fbfd);
		fbfd = 0;
		data = nullptr;
		throw FramebufferError(strerror(errno));
	}

	if(!nobuffer) buffer = new uint8_t[smem_len];
	if_nobuffer = nobuffer;
	m_valid = true;

	reset_buffer();
}
Framebuffer::~Framebuffer() {
	RELEASE_RESOURCE
}

Framebuffer::Framebuffer(Framebuffer &&f) :
	if_blend(f.if_blend),
	if_nobuffer(f.if_nobuffer),
	m_valid(f.m_valid),
	fbfd(f.fbfd),
	data(f.data),
	buffer(f.buffer),
	fbsize(f.fbsize),
	smem_len(f.smem_len),
	bytes_per_pixel(f.bytes_per_pixel),
	line_length(f.line_length)
{
	f.fbfd = 0;
	f.data = nullptr;
	f.buffer = nullptr;
	f.m_valid = false;
}
Framebuffer &Framebuffer::operator=(Framebuffer &&f) & {
	RELEASE_RESOURCE
	if(f.valid()) {
		if_blend = f.if_blend;
		if_nobuffer = f.if_nobuffer;
		m_valid = f.m_valid;
		fbfd = f.fbfd;
		data = f.data;
		buffer = f.buffer;
		fbsize = f.fbsize;
		smem_len = f.smem_len;
		bytes_per_pixel = f.bytes_per_pixel;
		line_length = f.line_length;

		f.fbfd = 0;
		f.data = nullptr;
		f.buffer = nullptr;
		f.m_valid = false;
	} else {
		fbfd = 0;
		data = nullptr;
		buffer = nullptr;
		m_valid = false;
	}
	return *this;
}

void Framebuffer::set(UCoord pos, Color c) {
	assert(valid());
	if(pos.x < 0 || pos.y < 0 || pos.x >= fbsize.w || pos.y >= fbsize.h) {
		return;
	}
	uint32_t value;
	if(if_blend && c.a != 255) {
		Color prev = get({pos.x, pos.y});
		double source = c.a / 255.0;
		double dest = 1 - source;
		value = (255 << 24) +
			((int)(c.r * source + prev.r * dest) << 16) +
			((int)(c.g * source + prev.g * dest) << 8) +
			(int)(c.b * source + prev.b * dest);
	} else {
		value = (c.a << 24) + (c.r << 16) + (c.g << 8) + c.b;
	}
	*(uint32_t *)((if_nobuffer ? data : buffer) + pos.x * bytes_per_pixel + pos.y * line_length) = value;
}

Color Framebuffer::get(UCoord pos) const {
	assert(valid());
	if(pos.x < 0 || pos.y < 0 || pos.x >= fbsize.w || pos.y >= fbsize.h) {
		throw FramebufferError("Coordinate out of bound");
	}
	uint32_t value = *(uint32_t *)((if_nobuffer ? data : buffer) + pos.x * bytes_per_pixel + pos.y * line_length);
	Color output;
	output.r = (value & 0x00ff0000) >> 16;
	output.g = (value & 0x0000ff00) >> 8;
	output.b = value & 0x000000ff;
	output.a = value >> 24;
	return output;
}

void Framebuffer::fill(const Color c) {
	assert(valid());
	for(uint_type y = 0; y < fbsize.h; ++y) {
		for(uint_type x = 0; x < fbsize.w; ++x) {
			set({x, y}, {c.r, c.g, c.b ,c.a});
		}
	}
}

Area Framebuffer::size() const {
	assert(valid());
	return fbsize;
}

void Framebuffer::update() {
	assert(valid());
	if(if_nobuffer) return;
	memcpy(data, buffer, smem_len);
}

void Framebuffer::reset_buffer() {
	assert(valid());
	if(if_nobuffer) return;
	memcpy(buffer, data, smem_len);
}

void Framebuffer::draw_rectangle(UCoord pos, Area a, Color c) {
	assert(valid());
	for(uint_type x = 0; x < a.w; x++) {
		set({pos.x + x, pos.y}, c);
		set({pos.x + x, pos.y + a.h - 1}, c);
	}
	for(uint_type y = 0; y < a.h; y++) {
		set({pos.x, pos.y + y}, c);
		set({pos.x + a.w - 1, pos.y + y}, c);
	}
}
void Framebuffer::fill_rectangle(UCoord pos, Area a, Color c) {
	assert(valid());
	for(uint_type y = 0; y < a.h; y++) {
		for(uint_type x = 0; x < a.w; x++) {
			set({pos.x + x, pos.y + y}, c);
		}
	}
}

void Framebuffer::draw_line(UCoord c1, UCoord c2, Color color) {
	if(c1.x == c2.x) {
		// To ensure that c1.y <= c2.y
		if(c2.y < c1.y) swap(c1.y, c2.y);
		fill_rectangle({c1.x, c1.y}, {1, c2.y - c1.y + 1}, color);
		return;
	}
	// To ensure that c1.x < c2.x
	if(c1.x > c2.x) swap(c1, c2);
	if(c1.y > c2.y) {
		double width = c2.x - c1.x + 1;
		uint_type height = c1.y - c2.y + 1;
		for(uint_type x = c1.x; x <= c2.x; ++x) {
			uint_type y1 = c1.y - static_cast<uint_type>(std::floor((x - c1.x) * height / width));
			uint_type y2 = c1.y - static_cast<uint_type>(std::ceil((x + 1 - c1.x) * height / width));
			fill_rectangle({x, y2}, {1, y1 - y2}, color);
		}
	} else if(c1.y < c2.y) {
		double width = c2.x - c1.x + 1;
		uint_type height = c2.y - c1.y + 1;
		for(uint_type x = c1.x; x <= c2.x; ++x) {
			uint_type y1 = c1.y + static_cast<uint_type>(std::floor((x - c1.x) * height / width));
			uint_type y2 = c1.y + static_cast<uint_type>(std::ceil((x + 1 - c1.x) * height / width));
			fill_rectangle({x, y1}, {1, y2 - y1}, color);
		}
	} else {
		fill_rectangle({c1.x, c1.y}, {c2.x - c1.x + 1, 1}, color);
	}
}

#endif
