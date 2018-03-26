#include "stdafx.h"
#include <utility>
#include <limits>
#include <harfbuzz/hb.h>
#include "xet_document.h"
#include "xet_input.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, boost::intrusive_ptr<T>, true);


namespace xet {

Document::Document()
{
	m_environment["cs"] = CSDecorator(m_controlSequences);
/*	
	m_environment["cs"] = py::cpp_function(
		[this](py::object o)->py::object
		{
			if (py::isinstance<py::str>(o))
			{
				return py::cpp_function(
					[this, o](py::object arg)->py::object
					{
						auto name = py::cast<std::u32string>(o);
						this->m_controlSequences.insert_or_assign(name, ControlSequence{ arg });
						return arg;
					});
			}
			auto name = py::cast<std::u32string>(o.attr("__name__"));
			this->m_controlSequences.insert_or_assign(name, ControlSequence{o});
			return o;
		}, py::arg("o"));
*/
}

void Document::addInput(fs::path const& fileName)
{
	LinePosTextRange name;

	auto t = m_inputs.emplace_front(fileName, input::loadUTF8TextFile(fileName));
	auto const& text = std::get<1>(t);
	auto parseTokens = parser::parse(text.begin(), text.end());
	auto tokens = input::convert(parseTokens, fileName, *this);
	m_tokens->insert(m_tokens->end(), tokens->begin(), tokens->end());
/*
	std::cout << "=== t1 ===" << std::endl;
	auto t1 = m_environment["t1"];
	std::cout << py::cast<std::string>(py::str(t1)) << std::endl;
	auto t1_class = t1.get_type();
	std::cout << py::cast<std::string>(py::str(t1_class)) << std::endl;
	auto t2 = t1_class(t1);
	std::cout << py::cast<std::string>(py::str(t2)) << std::endl;
*/
}

void Document::setPageFactory(py::object& initialPageFactory)
{
	if (m_initialPageFactory.is(py::none()))
		m_initialPageFactory = initialPageFactory;
	else
		if (true)	// check whether there is already any contents
			throw std::runtime_error("Invalid attempt to set initial page factory.");
}



GlyphInfos Document::shape(Font& font, std::u32string const& text)
{
	hb_buffer_t* hb_buffer = hb_buffer_create();
	hb_buffer_add_utf32(hb_buffer, reinterpret_cast<uint32_t const*>(&text[0]), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));
	hb_buffer_guess_segment_properties(hb_buffer);
	hb_shape(font.m_hbFont, hb_buffer, NULL, 0);

	// Get glyph information and positions out of the buffer.
	unsigned int len = hb_buffer_get_length(hb_buffer);
	hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
	hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

	try
	{
		auto r = GlyphInfos(static_cast<size_t>(len));

		for (unsigned int i = 0; i < len; i++)
		{
			auto& v = r[i];
			v.codePoint = info[i].codepoint;
			v.cluster = info[i].cluster;
			// Positions come as (value in pt)*64. We convert here into nm: 2.54/100*1e9/72/64 = 5512.15277
			v.xAdvance = pos[i].x_advance * 5512;
			v.yAdvance = pos[i].y_advance * 5512;
			v.xOffset = pos[i].x_offset * 5512;
			v.yOffset= pos[i].y_offset * 5512;
		}
		hb_buffer_destroy(hb_buffer);
		return r;
	}
	catch(...)
	{
		hb_buffer_destroy(hb_buffer);
		throw;
	}
}

Document::ControlSequence::ControlSequence(py::object& callable): m_callable(callable)
{
	auto inspect = py::module::import("inspect");
	auto sig = inspect.attr("signature")(callable);
	bool kwargs = false;
	//py::dict params = py::cast<py::dict>(sig.attr("parameters"));
	auto params = sig.attr("parameters");
	for (auto param: params.attr("values")())
	{
		if (param.attr("kind").is(param.attr("VAR_KEYWORD")))
		{
			kwargs = true;
			break;
		}
	}
	m_callWithDocument = params.contains("document") || kwargs;
	if (params.contains("groups"))
	{
		m_groupsRequested = true;
		py::object param = params["groups"];
		py::object anno = param.attr("annotation");
		if (py::isinstance<py::int_>(anno))
		{
			int n = py::cast<int>(anno);
			m_minGroups = n < 0 ? -1 : n;
			m_maxGroups = m_minGroups;
		}
		else if (py::isinstance<py::sequence>(anno))
		{
			auto s = py::sequence(anno);
			if (py::len(s) != 2)
				throw std::invalid_argument("If annotation for parameter 'groups' is a sequence, its length must be 2.");
			m_minGroups = py::cast<int>(s[0]);
			m_maxGroups = py::cast<int>(s[1]);
			if (m_minGroups > m_maxGroups) std::swap(m_minGroups, m_maxGroups);
			if (m_maxGroups <= 0)
			{
				m_minGroups = -1;
				m_maxGroups = -1;
			}
		}
		else if (anno.is(param.attr("empty")))
		{
			m_minGroups = 0;
			m_maxGroups = std::numeric_limits<int>::max();
		}
		else
			throw std::invalid_argument("Invalid annotation for parameter 'groups'.");
	}
	else
	{
		m_groupsRequested = false;
		if (kwargs)
		{
			m_minGroups = 0;
			m_maxGroups = std::numeric_limits<int>::max();
		}
		else
		{
			m_minGroups = -1;
			m_maxGroups = -1;
		}
	}
}

py::object CSDecoratorFromArgs::operator()(py::object o)
{
	if (m_name.empty())
	{
		auto name = py::cast<std::u32string>(o.attr("__name__"));
		m_controlSequences.insert_or_assign(name, Document::ControlSequence{o});
	}
	else
		m_controlSequences.insert_or_assign(m_name, Document::ControlSequence{o});
	return o;
}

py::object CSDecorator::operator()(py::object o)
{
	auto name = py::cast<std::u32string>(o.attr("__name__"));
	m_controlSequences.insert_or_assign(name, Document::ControlSequence{o});
	return o;
}

CSDecoratorFromArgs CSDecorator::operator()(std::u32string const& name)
{
	return {m_controlSequences, name};
}



}	// namespace xet
