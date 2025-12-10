/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Converter.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 11:54:38 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/08 11:54:39 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include <string>

class Converter {
private:
	Converter( void );
	Converter( const Converter& copy );
	Converter& operator=( const Converter& copy );
	~Converter( void );
public:
	static void convert( const std::string& word );
};

#endif
