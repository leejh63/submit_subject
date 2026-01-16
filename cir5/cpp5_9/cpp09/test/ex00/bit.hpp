/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bit.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 15:35:18 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/15 15:35:26 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <map>
#include <string>

class BitcoinExchange {
public:
    BitcoinExchange();
    BitcoinExchange(const BitcoinExchange& other);
    BitcoinExchange& operator=(const BitcoinExchange& other);
    ~BitcoinExchange();

    void loadDatabase(const std::string& csvPath);     // data.csv: date,exchange_rate
    void processInput(const std::string& inputPath) const; // input: date | value

private:
    std::map<std::string, double> _db; // key = "YYYY-MM-DD"

private:
    static std::string trim(const std::string& s);
    static bool splitOnce(const std::string& line, char sep, std::string& left, std::string& right);

    static bool parseDate(const std::string& s);       // "YYYY-MM-DD" 형식 + 달/일 검증
    static bool parseDoubleStrict(const std::string& s, double& out); // 스트림 기반 엄격 파싱
    static bool isLeapYear(int y);
    static int  daysInMonth(int y, int m);

    double findRateOnOrBefore(const std::string& date) const; // lower_bound 기반
};

#endif

