#include "stdafx.h"

#include <pybind11/operators.h>
#include "xet_geometry.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, boost::intrusive_ptr<T>, true);

namespace xet {

CPolygonSet::CPolygonSet(Polygon const& p)
{
	m_set = p;
}

CPolygonSet::CPolygonSet(std::vector<Point> const& v)
{
	Polygon p;
	gtl::set_points(p, v.cbegin(), v.cend());
	m_set = p;
}

CPolygonSet::CPolygonSet(Rectangle const& p)
{
	m_set = p;
}

template <typename T> 
CPolygonSet::CPolygonSet(T& v)
{
	std::vector<Point> points;
	for (auto item: v)
	{
		//if (py::isinstance<py::tuple>(item) || py::isinstance<py::list>(item))
		auto i = py::cast<py::sequence>(item);
		points.emplace_back(py::cast<Size>(i[0]), py::cast<Size>(i[0]));
	}
	Polygon poly;
	gtl::set_points(poly, points.cbegin(), points.cend());
	m_set = poly;

}

CPolygonSet& CPolygonSet::operator|=(const CPolygonSet& rhs)
{
	using namespace gtl::operators;
	m_set |= rhs.m_set;
	return *this;
}

PCPolygonSet operator|(CPolygonSet const& lhs, const CPolygonSet& rhs)
{
	PCPolygonSet r = new CPolygonSet(lhs);
	*r |= rhs;
	return r;
}

CPolygonSet& CPolygonSet::operator&=(const CPolygonSet& rhs)
{
	using namespace gtl::operators;
	m_set &= rhs.m_set;
	return *this;
}

PCPolygonSet operator&(CPolygonSet const& lhs, const CPolygonSet& rhs)
{
	PCPolygonSet r = new CPolygonSet(lhs);
	*r &= rhs;
	return r;
}

CPolygonSet& CPolygonSet::operator^=(const CPolygonSet& rhs)
{
	using namespace gtl::operators;
	m_set ^= rhs.m_set;
	return *this;
}

PCPolygonSet operator^(CPolygonSet const& lhs, const CPolygonSet& rhs)
{
	PCPolygonSet r = new CPolygonSet(lhs);
	*r ^= rhs;
	return r;
}

CPolygonSet& CPolygonSet::operator-=(const CPolygonSet& rhs)
{
	using namespace gtl::operators;
	m_set -= rhs.m_set;
	return *this;
}

PCPolygonSet operator-(CPolygonSet const& lhs, const CPolygonSet& rhs)
{
	PCPolygonSet r = new CPolygonSet(lhs);
	*r -= rhs;
	return r;
}

void pyInitGeometry(py::module& m)
{
	using namespace py::literals;
	py::class_<CPolygonSet, PCPolygonSet>(m, "PolygonSet")
		.def(py::init<>())
		.def(py::init<py::list>())
		.def(py::init<py::tuple>())
		.def(py::self |= py::self)
		.def(py::self | py::self)
		.def(py::self &= py::self)
		.def(py::self & py::self)
		.def(py::self ^= py::self)
		.def(py::self ^ py::self)
		.def(py::self -= py::self)
		.def(py::self - py::self);
	py::implicitly_convertible<py::sequence, CPolygonSet>();
	m.def("Rectangle", [](Size left, Size bottom, Size right, Size top)
	{
		return PCPolygonSet(new CPolygonSet(Rectangle(left, bottom, right, top)));
	}, "left"_a, "bottom"_a, "right"_a, "top"_a);
	m.def("Rectangle", [](Size left, Size top, Size width, Size height)
	{
		return PCPolygonSet(new CPolygonSet(Rectangle(left, top-height, left+width, top)));
	}, "left"_a, "top"_a, "width"_a, "height"_a);

}

}	// namespace xet
