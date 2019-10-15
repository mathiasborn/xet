#include "stdafx.h"

#include <algorithm>
#include <pybind11/operators.h>
#include "xet_geometry.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, boost::intrusive_ptr<T>, true);

#if defined _MSC_VER
#pragma warning(disable:4244)
#endif


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


void PolygonCompositor::add(PPolygonShapes p)
{
	m_dirty = true;
	m_input.emplace_back(p, 1.0);
}

void PolygonCompositor::changeScale(PolygonShapes const* p, double scale)
{
	auto i = std::find_if(m_input.begin(), m_input.end(), [p](auto const& item){ return std::get<PPolygonShapes>(item) == p; });
	if (i == m_input.end())
		throw std::runtime_error("PolygonShapes object not found in input.");
	std::get<double>(*i) = scale;
	m_dirty = true;
}

std::vector<PolygonCompositor::Output> const& PolygonCompositor::output() const
{
	if (m_dirty)
	{
		// dummy implementation
		m_output.resize(m_input.size());
		for (size_t i = 0; i < m_input.size(); i++)
		{
			auto const& in = m_input[i];
			m_output[i].m_polygons = std::get<PPolygonShapes>(in)->_polygons(std::get<double>(in));
			m_output[i].m_source = std::get<PPolygonShapes>(in).get();
		}			
	}
	return m_output;
}



class PyConstantPolygonShapes: public ConstantPolygonShapes
{
public:
	using ConstantPolygonShapes::ConstantPolygonShapes;

	virtual CPolygonSets polygons() override
	{
		PYBIND11_OVERLOAD_PURE(
			CPolygonSets,			// Return type
			ConstantPolygonShapes,	// Parent class
			polygons,				// Name of function in C++ (must match Python name)
									// Argument(s)
		);
	}
};

class PyVariablePolygonShapes: public VariablePolygonShapes
{
public:
	using VariablePolygonShapes::VariablePolygonShapes;

	virtual CPolygonSets polygons(double scale) override
	{
		PYBIND11_OVERLOAD_PURE(
			CPolygonSets,			// Return type
			VariablePolygonShapes,	// Parent class
			polygons,				// Name of function in C++ (must match Python name)
			scale					// Argument(s)
			);
	}
};




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

	py::class_<PolygonShapes, PPolygonShapes>(m, "PolygonShapes")
		.def_property_readonly("layer", &PolygonShapes::layer);

	py::class_<ConstantPolygonShapes, PyConstantPolygonShapes>(m, "ConstantPolygonShapes")
		.def(py::init<int32_t>(), "layer"_a)
		.def("polygons", &ConstantPolygonShapes::polygons);

	py::class_<VariablePolygonShapes, PyVariablePolygonShapes>(m, "VariablePolygonShapes")
		.def(py::init<int32_t>(), "layer"_a)
		.def("polygons", &VariablePolygonShapes::polygons);
}

}	// namespace xet
