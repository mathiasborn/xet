#pragma once

#include "stdafx.h"
#include <vector>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>


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



}	// namespace xet
