/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Serializer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 11:43:12 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/09 11:43:15 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <string>
#include <stdint.h>

struct Data;

class Serializer {
	public:
		static uintptr_t serialize( Data* ptr );
		static Data* deserialize( uintptr_t raw );
	private:
		Serializer( void );
		Serializer( const Serializer& copy );
		Serializer& operator=( const Serializer& copy );
		~Serializer( void );
};

#endif
