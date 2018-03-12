#pragma once

#include "stdafx.h"
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>


namespace xet {

class Font: public boost::intrusive_ref_counter<Font, boost::thread_unsafe_counter>
{
public:
	Font(fs::path const& path);
	virtual ~Font();
private:
	FT_Face m_ftFace;
	hb_font_t* m_hbFont;
};

typedef boost::intrusive_ptr<Font> PFont;

}	// namespace xet
