#include "stdafx.h"

#include <freetype/tttables.h>
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

Font::Font(fs::path const& path, int size): m_path(path), m_ptSize(size)
{
	auto s = uts::toUtf8(uts::toUtf32(path));
	auto error = FT_New_Face(ftLibrary(), s.c_str(), 0, &m_ftFace);
	if (error)
		throw std::runtime_error("freetype error "s + boost::lexical_cast<std::string>(error));
	error = FT_Set_Char_Size(m_ftFace, size*64, size*64, 0, 0);
	if (error)
		throw std::runtime_error("freetype error "s + boost::lexical_cast<std::string>(error));
	auto tth = reinterpret_cast<TT_Header*>(FT_Get_Sfnt_Table(m_ftFace, FT_SFNT_HEAD));
	if (!tth)
		throw std::runtime_error("Unable to load TrueType font header table of font '"s + s + "'."s);
	auto os2 = reinterpret_cast<TT_OS2*>(FT_Get_Sfnt_Table(m_ftFace, FT_SFNT_OS2));
	if (!os2)
		throw std::runtime_error("Unable to load OS/2 table of font '"s + s + "'."s);
	m_baselineToBaseline = (os2->sTypoAscender - os2->sTypoDescender + os2->sTypoLineGap) * nmSize() / tth->Units_Per_EM;
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
		if (f->m_path == path && f->m_ptSize == size)
			return f;
	auto r = PFont{new Font{path, size}};
	m_fonts.push_back(r);
	return r;
}

Shape::Shape(Font const& font, std::u32string const& text): baselineToBaseline(font.m_baselineToBaseline)
{
	hb_buffer_t* hb_buffer = hb_buffer_create();
	hb_buffer_add_utf32(hb_buffer, reinterpret_cast<uint32_t const*>(&text[0]), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));
	hb_buffer_guess_segment_properties(hb_buffer);
	hb_shape(font.m_hbFont, hb_buffer, NULL, 0);

	// Get glyph information and positions out of the buffer.
	unsigned int len = hb_buffer_get_length(hb_buffer);
	hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
	hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

	try
	{
		glyphInfos.resize(static_cast<size_t>(len));

		for (unsigned int i = 0; i < len; i++)
		{
			auto& v = glyphInfos[i];
			v.codePoint = info[i].codepoint;
			v.cluster = info[i].cluster;
			// Positions come as (value in pt)*64. We convert here into nm: 2.54/100*1e9/72/64 = 5512.15277
			v.xAdvance = pos[i].x_advance * 5512;
			v.yAdvance = pos[i].y_advance * 5512;
			v.xOffset = pos[i].x_offset * 5512;
			v.yOffset= pos[i].y_offset * 5512;

			hb_glyph_extents_t x;
			if (!hb_font_get_glyph_extents(font.m_hbFont, info[i].codepoint, &x))
				throw std::runtime_error("Failed to determine glyph extents.");
			v.xBearing = x.x_bearing * 5512;
			v.yBearing = x.y_bearing * 5512;
			v.width = x.width * 5512;
			v.height = x.height * 5512;

			advance += horizontal ? v.xAdvance : v.yAdvance;
		}
		hb_buffer_destroy(hb_buffer);
	}
	catch(...)
	{
		hb_buffer_destroy(hb_buffer);
		throw;
	}
}



}	// namespace xet
