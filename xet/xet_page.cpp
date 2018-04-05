#include "stdafx.h"
#include <string>
#include <PDFWriter/PDFWriter.h>
#include <PDFWriter/PDFPage.h>
#include <PDFWriter/PDFRectangle.h>
#include <PDFWriter/PageContentContext.h>
#include "interfaces.h"

using namespace std::string_literals;

namespace xet {

std::unique_ptr<PDFPage> Page::toPDF()
{
	auto page = std::make_unique<PDFPage>();
	page->SetMediaBox(PDFRectangle(0, 0, ::toPDF(width()), ::toPDF(height())));

	return page;
}

}	// namespace xet
