#pragma once

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/polygon/polygon.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/preprocessor/cat.hpp>
#include <pybind11/pybind11.h>
#include "py_xet.h"

namespace gtl = boost::polygon;
namespace mp = boost::multiprecision;
namespace py = pybind11;

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
typedef gtl::rectangle_data<Size> Rectangle;
typedef gtl::polygon_set_data<Size> PolygonSet;
typedef gtl::polygon_traits<Polygon>::point_type Point;

class CPolygonSet;
typedef boost::intrusive_ptr<CPolygonSet> PCPolygonSet;
typedef std::vector<PCPolygonSet> CPolygonSets;


class CPolygonSet: public boost::intrusive_ref_counter<CPolygonSet, boost::thread_unsafe_counter>
{
public:
	CPolygonSet() = default;
	CPolygonSet(CPolygonSet const&) = default;
	CPolygonSet(PolygonSet const& set): m_set(set) {};
	CPolygonSet(Polygon const&);
	CPolygonSet(std::vector<Point> const&);
	CPolygonSet(Rectangle const&);
	template <typename T> CPolygonSet(T&);
	
	CPolygonSet& operator|=(const CPolygonSet& rhs);
	friend PCPolygonSet operator|(CPolygonSet const& lhs, const CPolygonSet& rhs);

	CPolygonSet& operator&=(const CPolygonSet& rhs);
	friend PCPolygonSet operator&(CPolygonSet const& lhs, const CPolygonSet& rhs);

	CPolygonSet& operator^=(const CPolygonSet& rhs);
	friend PCPolygonSet operator^(CPolygonSet const& lhs, const CPolygonSet& rhs);

	CPolygonSet& operator-=(const CPolygonSet& rhs);
	friend PCPolygonSet operator-(CPolygonSet const& lhs, const CPolygonSet& rhs);
private:
	PolygonSet m_set;
//	int32_t m_z = 0;	// z-coordinate (depth). In a Stack (see below), elements with higher values appear on top of elements with lower values.
//public:
//	int32_t z() const { return m_z; }
};

class PolygonShapes: public PyObjectHolder<PolygonShapes>
{
public:
	PolygonShapes(int32_t layer) : m_layer(layer) {}
	virtual ~PolygonShapes() {};
private:
	int32_t m_layer = 0;	// higher m_layer cut into shapes with lower m_layer
public:
	int32_t layer() const { return m_layer; }
};

class ConstantPolygonShapes: public PolygonShapes
{
public:
	typedef PolygonShapes Super;

	ConstantPolygonShapes(int32_t layer) : Super(layer) {}

	virtual CPolygonSets polygons() = 0;
};

class VariablePolygonShapes: public PolygonShapes
{
public:
	typedef PolygonShapes Super;

	VariablePolygonShapes(int32_t layer) : Super(layer) {}

	virtual CPolygonSets polygons(double scale) = 0;
};


class PolygonCompositor: public boost::intrusive_ref_counter<PolygonCompositor, boost::thread_unsafe_counter>
{
public:
	PolygonCompositor() = default;


	// check how this will be wrapped by pybind11
	void add(PolygonShapes);
private:
	typedef std::unordered_map<int32_t, CPolygonSets> Input;
	Input m_input;
	m_output;

};



void pyInitGeometry(py::module&);




}	// namespace xet
