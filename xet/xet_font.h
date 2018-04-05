#pragma once

#include "stdafx.h"
#include <vector>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include "xet_geometry.h"


namespace xet {

class Font: public boost::intrusive_ref_counter<Font, boost::thread_unsafe_counter>
{
public:
	Font(fs::path const& path, int size);
	virtual ~Font();

	fs::path m_path;
	int m_size;
	FT_Face m_ftFace;
	hb_font_t* m_hbFont;
};

typedef boost::intrusive_ptr<Font> PFont;

class FontRegistry
{
public:
	PFont font(fs::path const& path, int size);
private:
	std::vector<PFont> m_fonts;
};

struct GlyphInfo
{
	uint32_t codePoint;
	uint32_t cluster;
	Size xAdvance;
	Size yAdvance;
	Size xOffset;
	Size yOffset;
	Size xBearing;	// left side of glyph from origin.
	Size yBearing;	// top side of glyph from origin.
	Size width;		// distance from left to right side.
	Size height;	// distance from top to bottom side.
};

typedef std::vector<GlyphInfo> GlyphInfos;

struct Shape
{
	Shape(GlyphInfos&&);
	
	GlyphInfos glyphInfos;
	Size width = 0;
	Size yBearing = 0;
	Size height = 0;
};


}	// namespace xet
