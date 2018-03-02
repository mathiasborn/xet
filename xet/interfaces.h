#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <memory>
#include <boost/polygon/polygon.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/preprocessor/cat.hpp>

namespace gtl = boost::polygon;
namespace mp = boost::multiprecision;

namespace xet {
	typedef int64_t Size;
};

namespace boost {
	namespace polygon {
/*
		template <>
		struct coordinate_traits<xet::Size> {
			typedef int64_t coordinate_type;
			typedef long double area_type;
			typedef mp::int128_t manhattan_area_type;
			typedef mp::uint128_t unsigned_area_type;
			typedef mp::int128_t coordinate_difference;
			typedef mp::cpp_rational coordinate_distance;
		};
*/
		template <>
		struct high_precision_type<int64_t> {
			typedef mp::cpp_rational type;
		};

	};
};

namespace xet {

typedef gtl::polygon_data<Size> Polygon;
typedef gtl::polygon_set_data<Size> PolygonSet;
typedef std::vector<PolygonSet> PolygonSets;
typedef gtl::polygon_traits<Polygon>::point_type Point;

#define GETR(name) decltype(BOOST_PP_CAT(m_, name)) const& name() const { return BOOST_PP_CAT(m_, name); }
#define GETV(name) decltype(BOOST_PP_CAT(m_, name)) name() const { return BOOST_PP_CAT(m_, name); }


class CanvasElement
{
public:
	virtual ~CanvasElement() {};
};

typedef std::shared_ptr<CanvasElement> PCanvasElement;
typedef std::vector<PCanvasElement> CanvasElements;

class VisibleCanvasElement
{
public:
	VisibleCanvasElement(int32_t layer, int32_t cutOrder);
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

class TypeSetter: public VisibleCanvasElement
{
public:
	typedef VisibleCanvasElement Super;
	
	TypeSetter(int32_t layer, int32_t cutOrder, std::u32string name, bool simple):
		Super(layer, cutOrder), m_name(name), m_simple(simple) {};
	virtual PolygonSets geometry(double a) = 0;
private:
	std::u32string m_name;
	bool m_simple = false;
public:
	GETR(name)
	GETV(simple)
};

class Canvas: public VisibleCanvasElement
{
public:
	typedef VisibleCanvasElement Super;

	Canvas(int32_t layer, int32_t cutOrder): Super(layer, cutOrder) {};
	virtual PolygonSet geometry() = 0;
private:
	CanvasElements m_contents;
public:
	GETR(contents)
};

class Page;
typedef std::shared_ptr<Page> PPage;

class Page: public Canvas
{
public:
	typedef Canvas Super;

	Page(Size width, Size height): m_width(width), m_height(height) {};
	virtual PolygonSet geometry() { return {}; }
	virtual PPage nextPage() = 0;
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
