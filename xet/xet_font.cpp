#include "stdafx.h"

#include "xet_font.h"
#include "unicode_string_support.h"
#include <string>
#include <boost/lexical_cast.hpp>


using namespace std::string_literals;

namespace xet {

FT_Library ftLibrary()
{
	static FT_Library lib  = nullptr;
	if (!lib)
	{
		auto error = FT_Init_FreeType(&lib);
		if (error)
			throw std::runtime_error("freetype error "s + boost::lexical_cast<std::string>(error));
	}
	return lib;
}

Font::Font(fs::path const& path, int size): m_path(path), m_size(size)
{
	auto s = uts::toUtf8(uts::toUtf32(path));
	auto error = FT_New_Face(ftLibrary(), s.c_str(), 0, &m_ftFace);
	if (error)
		throw std::runtime_error("freetype error "s + boost::lexical_cast<std::string>(error));
	error = FT_Set_Char_Size(m_ftFace, size*64, size*64, 0, 0);
	if (error)
		throw std::runtime_error("freetype error "s + boost::lexical_cast<std::string>(error));
	m_hbFont = hb_ft_font_create(m_ftFace, NULL);
}

Font::~Font()
{
	hb_font_destroy(m_hbFont);
	FT_Done_Face(m_ftFace);
}

PFont FontRegistry::font(fs::path const& path, int size)
{
	for (auto& f: m_fonts)
		if (f->m_path == path && f->m_size == size)
			return f;
	auto r = PFont{new Font{path, size}};
	m_fonts.push_back(r);
	return r;
}

}	// namespace xet
