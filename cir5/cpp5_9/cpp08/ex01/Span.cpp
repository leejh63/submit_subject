/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Span.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/13 15:35:46 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/14 12:07:55 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Span.hpp"
#include <limits>

Span::Span( unsigned int _size )
: _container(), _size(_size) {}

Span::Span( const Span& copy )
: _container(copy._container), _size(copy._size) {}

Span& Span::operator=( const Span& copy ) {
	if (this != &copy) {
		this->_container = copy._container;
		this->_size = copy._size;
	}
	return *this;
}

Span::~Span( void ){}

void Span::addNumber( int arg ){
	 if ( _container.size() >= _size )
	 	throw std::runtime_error("No space in this container");
	 _container.insert(arg);
}

long long Span::shortestSpan( void ) {
	if (_container.size() <= 1)
		throw std::runtime_error("Can't calculate distance");

	std::multiset<int>::iterator prev = _container.begin();
	std::multiset<int>::iterator curr = prev;
	++curr;

	long long min = std::numeric_limits<long long>::max();
	
	for ( ; curr != _container.end(); ++curr, ++prev) {
		long long diff = static_cast<long long>(*curr) - static_cast<long long>(*prev);
		if ( min > diff) {
			min = diff;
		}
		if (min == 0)
            break;
	}
	
	return min;
}

long long Span::longestSpan( void ) {
	if (_container.size() <= 1)
		throw std::runtime_error("Can't calculate distance");
	long long minValue = static_cast<long long>(*_container.begin());
	long long maxValue = static_cast<long long>(*_container.rbegin());
	return maxValue - minValue;

}
































