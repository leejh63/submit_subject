/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Span.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/13 15:35:42 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/13 15:35:43 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SPAN_HPP
#define SPAN_HPP

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <string>
#include <set>

class Span {
	private:
		std::multiset<int> _container;
		unsigned int _size;

	Span( void );

	public:
		Span( unsigned int _size );
		Span( const Span& copy );
		Span& operator=( const Span& copy );
		~Span( void );
		
		template <typename T>
		void addNumber( T start, T end );
		void addNumber( int arg );
		
		long long shortestSpan( void );
		long long longestSpan( void );
		
};

template <typename T>
void Span::addNumber( T start, T end ){
	 unsigned int dist = static_cast<unsigned int>(std::distance(start, end));
	 if ( _container.size() + dist > _size )
	 	throw std::runtime_error("No space in this container");
	 _container.insert(start, end);
}

#endif
