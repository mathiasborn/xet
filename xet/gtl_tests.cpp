#include "stdafx.h"
#include <memory>
#include <boost/polygon/polygon.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>
#include <cassert>
#include <cmath>

#include <PDFWriter/PDFWriter.h>
#include <PDFWriter/PDFPage.h>
#include <PDFWriter/PDFRectangle.h>
#include <PDFWriter/PageContentContext.h>

#include "interfaces.h"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;
namespace pdf = PDFHummus;

constexpr double toPDF(int64_t u)
{
	return static_cast<double>(u)*(1.0e-9 / (2.54 / 100.0 / 72.0));	// nm -> 1/72in
}

constexpr int64_t operator "" _cm(unsigned long long int i) { return i * 10'000'000; }
constexpr int64_t operator "" _mm(unsigned long long int i) { return i * 1'000'000; }
constexpr int64_t operator "" _nm(unsigned long long int i) { return i; }

void gtl_test1()
{
	//lets construct a 10x10 rectangle shaped polygon
	typedef gtl::polygon_data<int> Polygon;
	typedef gtl::polygon_traits<Polygon>::point_type Point;
	Point pts[] = { gtl::construct<Point>(0, 0),
		gtl::construct<Point>(10, 0),
		gtl::construct<Point>(10, 10),
		gtl::construct<Point>(0, 10) };
	Polygon poly;
	gtl::set_points(poly, pts, pts + 4);

	//now lets see what we can do with this polygon
	assert(gtl::area(poly) == 100.0f);
	assert(gtl::contains(poly, gtl::construct<Point>(5, 5)));
	assert(!gtl::contains(poly, gtl::construct<Point>(15, 5)));
	gtl::rectangle_data<int> rect;
	assert(gtl::extents(rect, poly)); //get bounding box of poly
	assert(gtl::equivalence(rect, poly)); //hey, that's slick
	assert(gtl::winding(poly) == gtl::COUNTERCLOCKWISE);
	assert(gtl::perimeter(poly) == 40.0f);

	//add 5 to all coords of poly
	gtl::convolve(poly, gtl::construct<Point>(5, 5));
	//multiply all coords of poly by 2
	gtl::scale_up(poly, 2);
	gtl::set_points(rect, gtl::point_data<int>(10, 10),
		gtl::point_data<int>(30, 30));
	assert(gtl::equivalence(poly, rect));
}

void math_test1()
{
	using namespace boost::multiprecision;

	int128_t v = 1;

	// Do some fixed precision arithmetic:
	for (unsigned i = 1; i <= 20; ++i)
		v *= i;

	std::cout << v << std::endl; // prints 20!

								 // Repeat at arbitrary precision:
	cpp_int u = 1;
	for (unsigned i = 1; i <= 100; ++i)
		u *= i;

	std::cout << u << std::endl; // prints 100!
	u /= 101;
	std::cout << u << std::endl;
}

void math_test2()
{
	using namespace boost::multiprecision;

	cpp_rational v = 1;

	// Do some arithmetic:
	for (unsigned i = 1; i <= 1000; ++i)
		v *= i;
	v /= 10;

	std::cout << v << std::endl; // prints 1000! / 10
	std::cout << numerator(v) << std::endl;
	std::cout << denominator(v) << std::endl;

	cpp_rational w(2, 3);  // component wise constructor
	std::cout << w << std::endl; // prints 2/3
}

void pdf_test1()
{
	PDFWriter out;
	out.StartPDF("myFile.pdf", ePDFVersion13);
	//auto page = std::unique_ptr<PDFPage>{new PDFPage()};
	auto page = std::make_unique<PDFPage>();
	page->SetMediaBox(PDFRectangle(0, 0, 595, 842));
	auto contentContext = out.StartPageContentContext(page.get());
	AbstractContentContext::GraphicOptions pathFillOptions(AbstractContentContext::eFill, AbstractContentContext::eCMYK, 0xFF000000);
	AbstractContentContext::GraphicOptions pathStrokeOptions(AbstractContentContext::eStroke, AbstractContentContext::eRGB, AbstractContentContext::ColorValueForName("DarkMagenta"), 4);
	contentContext->DrawRectangle(375, 220, 50, 160, pathFillOptions);
	contentContext->DrawRectangle(375, 10, 50, 160, pathStrokeOptions);
	out.EndPageContentContext(contentContext);
	out.WritePage(page.get());
	out.EndPDF();
}

void exportPDF(PageContentContext& ctx, AbstractContentContext::GraphicOptions const& opt, Polygon& p)
{
	ctx.RG(0, 0, 0);
	ctx.w(toPDF(1_mm));
	auto i = gtl::begin_points(p);
	ctx.m(toPDF(i->x()), toPDF(i->y()));
	for(; i != gtl::end_points(p); i++)
	{
		ctx.l(toPDF(i->x()), toPDF(i->y()));
	}
	ctx.h();
	ctx.S();
}

void polygon_test1()
{
	using namespace gtl; //because of operators

						 //lets construct a rectangle shaped polygon
	typedef gtl::polygon_traits<Polygon>::point_type Point;
	Point pts[] = { gtl::construct<Point>(1_cm, 1_cm),
		gtl::construct<Point>(10_cm, 2_cm),
		gtl::construct<Point>(10_cm, 10_cm),
		gtl::construct<Point>(3_cm, 10_cm) };
	Polygon p1;
	gtl::set_points(p1, pts, pts + 4);

	std::vector<Point> cPoints;
	constexpr auto pi = 3.14159265358979323846;
	// calculate circle in m
	constexpr auto N = 10;
	constexpr double cx = 0.1;
	constexpr double cy = 0.1;
	constexpr double cr = 0.02;
	for (int i = 0; i < N; i++)
	{
		auto x = static_cast<int64_t>((cx + std::cos(i*2*pi/N) * cr) * 1e9);
		auto y = static_cast<int64_t>((cy + std::sin(i*2*pi/N) * cr) * 1e9);
		cPoints.push_back({x, y});
	}
	Polygon c1;
	gtl::set_points(c1, cPoints.cbegin(), cPoints.cend());

	polygon_set_data<int64_t> ps;
	ps += p1 - c1;
	
	std::vector<Polygon> combo;
	ps.get(combo);

	PDFWriter out;
	out.StartPDF("polygon_test1.pdf", ePDFVersion13);
	//auto page = std::unique_ptr<PDFPage>{new PDFPage()};
	auto page = std::make_unique<PDFPage>();
	page->SetMediaBox(PDFRectangle(0, 0, 595, 842));
	auto contentContext = out.StartPageContentContext(page.get());
	AbstractContentContext::GraphicOptions pathFillOptions(AbstractContentContext::eFill, AbstractContentContext::eCMYK, 0xFF000000);
	AbstractContentContext::GraphicOptions pathStrokeOptions(AbstractContentContext::eStroke, AbstractContentContext::eRGB, AbstractContentContext::ColorValueForName("DarkMagenta"), 4);

	//exportPDF(*contentContext, pathFillOptions, p1);
	//exportPDF(*contentContext, pathFillOptions, c1);
	for (auto& p: combo) exportPDF(*contentContext, pathFillOptions, p);

	out.EndPageContentContext(contentContext);
	out.WritePage(page.get());
	out.EndPDF();
}
