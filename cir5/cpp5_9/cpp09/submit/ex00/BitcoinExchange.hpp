/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BitcoinExchange.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 11:21:23 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/06 11:21:24 by Jaeholee         ###   ########.fr       */
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
	
	void processInput(const std::string& input) const;
	void loadDatabase(const std::string& csv);

private:
	std::map< std::string, double > _db;
	
	double findRateOnOrBefore(const std::string& date) const;
	bool parseDouble(const std::string& s, double& out) const;
	bool parseDate(const std::string& s) const;
	int daysInMonth(int y, int m) const;
	bool isLeapYear(int y) const;
	bool splitOnce(const std::string& line, char sep, std::string& left, std::string& right) const;
	std::string trim(const std::string& s) const;
	
	BitcoinExchange( const BitcoinExchange& copy );
	BitcoinExchange& operator=( const BitcoinExchange& copy );
};

#endif
