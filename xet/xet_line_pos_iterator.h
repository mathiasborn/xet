// based on line_pos_iterator.hpp from boost spirit
#pragma once

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/spirit/home/support/container.hpp>

template <class Iterator>
class line_pos_iterator: public boost::iterator_adaptor<
    line_pos_iterator<Iterator>  // Derived
    , Iterator                     // Base
    , boost::use_default           // Value
    , boost::random_access_traversal_tag // CategoryOrTraversal
> {
public:
	line_pos_iterator() {};

    explicit line_pos_iterator(Iterator begin, Iterator base);

	std::size_t line() const { return m_line; }
	Iterator lineStart() const;
	std::size_t column(std::size_t tabs) const;

private:
    friend class boost::iterator_core_access;

	void advance(typename iterator_adaptor::difference_type n);
	void increment();
	void decrement();

	Iterator m_begin{0};
	std::size_t m_line{1}; // The line position.
};

typedef line_pos_iterator<std::u32string::const_iterator> LinePosIterator;
typedef boost::iterator_range<LinePosIterator> LinePosTextRange;

namespace boost::spirit::traits {
	template<> struct is_container<LinePosTextRange> : mpl::false_ {};
}


template <class Iterator>
line_pos_iterator<Iterator>::line_pos_iterator(Iterator begin, Iterator base): line_pos_iterator::iterator_adaptor_(base), m_line(1), m_begin(begin)
{}

template<class Iterator>
void line_pos_iterator<Iterator>::advance(typename iterator_adaptor::difference_type n)
{
	auto& ref = base_reference();
	if (n > 0)
		for(; n > 0; --n)
		{
			if (ref == '\n') ++m_line;
			++ref;
		}
	else
		for (; n < 0; ++n)
		{
			--ref;
			if (ref == '\n') --m_line;
		}
}

template<class Iterator>
void line_pos_iterator<Iterator>::increment()
{
    typename std::iterator_traits<Iterator>::reference
        ref = *(this->base());
      
    if (ref == '\n') ++m_line;

	++this->base_reference();
}

template<class Iterator>
void line_pos_iterator<Iterator>::decrement()
{
	typename std::iterator_traits<Iterator>::reference
		ref = *(this->base());

	--this->base_reference();

	if (ref == '\n') --m_line;
}

template<class Iterator>
Iterator line_pos_iterator<Iterator>::lineStart() const
{
	Iterator ref = this->base();
	if (ref == m_begin()) return ref;
	for (--ref;; --ref)
	{
		if (*ref == '\n') return ref + 1;
		if (ref == m_begin()) break;
	};
	return ref;
}


template<class Iterator>
std::size_t line_pos_iterator<Iterator>::column(std::size_t tabs) const
{
	std::size_t col = 0;
	auto const& ref = this->base();

	for (auto i = lineStart(); i != ref; ++i)
		switch (*i)
		{
			case '\t':
				col += tabs - col % tabs;
				break;
			default:
				++col;
		}

	return col + 1;
}
