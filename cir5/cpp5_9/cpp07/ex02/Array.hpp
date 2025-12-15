/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Array.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/10 17:11:53 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/10 17:11:59 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <stdexcept>
#include <cstdlib>
#include <ctime>

template <typename T>
class Array {
private:
    T* _data;
    size_t _size;

public:
    Array( void );
    Array( size_t n );
    Array( const Array& other );
    Array& operator=( const Array& other );
    ~Array( void );

    T& operator[]( size_t idx );
    const T& operator[]( size_t idx ) const;

    size_t size( void ) const;
};

#include "Array.tpp"

#endif
