#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <memory>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include "py_xet.h"
#include "xet_geometry.h"

namespace xet {

#define GETR(name) decltype(BOOST_PP_CAT(m_, name)) const& name() const { return BOOST_PP_CAT(m_, name); }
#define GETV(name) decltype(BOOST_PP_CAT(m_, name)) name() const { return BOOST_PP_CAT(m_, name); }


class CanvasElement: public boost::intrusive_ref_counter<CanvasElement, boost::thread_unsafe_counter>
{
public:
	virtual ~CanvasElement() {};
};

typedef boost::intrusive_ptr<CanvasElement> PCanvasElement;
//typedef std::shared_ptr<CanvasElement> PCanvasElement;
typedef std::vector<PCanvasElement> CanvasElements;

class VisibleCanvasElement: public PyObjectHolder<VisibleCanvasElement>
{
public:
	VisibleCanvasElement(int32_t layer, int32_t cutOrder): m_layer(layer), m_cutOrder(cutOrder) {}
	virtual ~VisibleCanvasElement() {};
private:
	int32_t m_layer = 0;	// z-coordinate (depth), elements with higher values appear on top of elements with lower values
	int32_t m_cutOrder = 0;	// elements with higher m_cutOrder cut into elements with lower order (on the same layer)
public:
	GETV(layer)
	GETV(cutOrder)
};

/*
class TextArea: public VisibleCanvasElement
{
	TextArea(std::tuple<Size, Size> adjustment, int32_t layer, int32_t cutOrder, bool simple);
	virtual PolygonSet geometry(Size a) const;
private:
	std::tuple<Size, Size> m_adjustment;
	bool m_simple = false;
public:
	GETR(adjustment)
	GETV(simple)
};

typedef std::shared_ptr<TextArea> PTextArea;
typedef std::vector<PTextArea> TextAreas;
*/

class TypeSetter: public VisibleCanvasElement//, public PyObjectHolder<TypeSetter>
{
public:
	typedef VisibleCanvasElement Super;
	
	TypeSetter(int32_t layer, int32_t cutOrder, std::u32string name, bool simple):
		Super(layer, cutOrder), m_name(name), m_simple(simple) {};
	virtual CPolygonSets geometry(double a) = 0;
private:
	std::u32string m_name;
	bool m_simple = false;
public:
	GETR(name)
	GETV(simple)
};
//typedef std::shared_ptr<TypeSetter> PTypeSetter;
typedef boost::intrusive_ptr<TypeSetter> PTypeSetter;

class Canvas: public VisibleCanvasElement//, public PyObjectHolder<Canvas>
{
public:
	typedef VisibleCanvasElement Super;

	Canvas(int32_t layer = 0, int32_t cutOrder = 0): Super(layer, cutOrder) {};
	virtual PCPolygonSet geometry() = 0;
private:
	CanvasElements m_contents;
public:
	GETR(contents)
};

class Page;
typedef boost::intrusive_ptr<Page> PPage;

class Page: public Canvas//, public PyObjectHolder<Page>
{
public:
	typedef Canvas Super;

	Page(Size width, Size height): m_width(width), m_height(height) {};
	virtual PCPolygonSet geometry() { return {}; }
	virtual PPage nextPage() = 0;

	std::unique_ptr<PDFPage> toPDF();
private:
	Size m_width;
	Size m_height;
public:
	GETV(width)
	GETV(height)
};


/*
class ControlSequence
{
};

typedef std::shared_ptr<ControlSequence> PControlSequence;
*/

} // namespace xet

#undef GET
