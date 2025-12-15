/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MutantStack.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 12:17:42 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/14 12:17:44 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


template < typename T, typename C>
MutantStack<T, C>::MutantStack( void ) : std::stack<T, C>() {  }

template < typename T, typename C>
MutantStack<T, C>::MutantStack( const MutantStack<T, C>& copy ) 
: std::stack<T, C>(copy) {  }

template < typename T, typename C>
MutantStack<T, C>& MutantStack<T, C>::operator=( const MutantStack<T, C>& copy ) {
	if (this != &copy)
        std::stack<T, C>::operator=(copy);
    return *this;
}

template < typename T, typename C>
MutantStack<T, C>::~MutantStack( void ) {}


/*
// 스택처럼 읽도록 만든 코드, 
template < typename T, typename C>
typename MutantStack<T, C>::riterator MutantStack<T, C>::rbegin( void ){
	return this->c.rbegin();
}

template < typename T, typename C>
typename MutantStack<T, C>::riterator MutantStack<T, C>::rend( void ){
	return this->c.rend();
}
*/

/*//////
template < typename T, typename C>
typename MutantStack<T, C>::iterator MutantStack<T, C>::begin( void ){
	return this->c.begin();
}

template < typename T, typename C>
typename MutantStack<T, C>::iterator MutantStack<T, C>::end( void ){
	return this->c.end();
}
*//////

template < typename T, typename C>
typename MutantStack<T, C>::iterator MutantStack<T, C>::begin( void ){
	return this->c.rbegin();
}

template < typename T, typename C>
typename MutantStack<T, C>::iterator MutantStack<T, C>::end( void ){
	return this->c.rend();
}

template < typename T, typename C>
typename MutantStack<T, C>::const_iterator MutantStack<T, C>::begin( void ) const {
	return this->c.begin();
}

template < typename T, typename C>
typename MutantStack<T, C>::const_iterator MutantStack<T, C>::end( void ) const {
	return this->c.end();
}
