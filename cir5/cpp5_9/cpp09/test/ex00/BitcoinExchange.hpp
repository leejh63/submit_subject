/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BitcoinExchange.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 10:36:08 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/15 10:43:52 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <fstream>
#include <string>
#include <map>

class BitcoinExchange {
public:
	BitcoinExchange( void );
	~BitcoinExchange( void );
	void test_print( void );
	
private:
	std::map< std::string, double > coin_data;
	
	BitcoinExchange( const BitcoinExchange& copy );
	BitcoinExchange& operator=( const BitcoinExchange& copy );
	
};

#endif
