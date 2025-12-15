/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MutantStack.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 12:17:15 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/14 13:03:50 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MUTANTSTACK_HPP
#define MUTANTSTACK_HPP

#include <stack>
#include <deque>

template < typename T = int , typename C = std::deque<T> >
class MutantStack : public std::stack<T, C> {
	private:

	public:
	
	    /*
	    typedef typename C::iterator               iterator;
    	*/
	    typedef typename C::reverse_iterator       iterator;
    	//typedef typename C::reverse_iterator       riterator;
    	typedef typename C::const_iterator         const_iterator;

		MutantStack( void );
		MutantStack( const MutantStack& copy );
		MutantStack& operator=( const MutantStack& copy );
		~MutantStack( void );
		/*
		riterator rbegin( void );
		riterator rend( void );
		*/
		iterator begin( void );
		iterator end( void );
		
		const_iterator begin( void ) const ;
		const_iterator end( void ) const ;


};

#include "MutantStack.tpp"

#endif
