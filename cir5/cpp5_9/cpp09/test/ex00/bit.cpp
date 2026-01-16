/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bit.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 15:34:57 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/15 15:35:00 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "bit.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cctype>

// ---------- Ctors / Dtors ----------
BitcoinExchange::BitcoinExchange() {}
BitcoinExchange::BitcoinExchange(const BitcoinExchange& other) : _db(other._db) {}
BitcoinExchange& BitcoinExchange::operator=(const BitcoinExchange& other) {
    if (this != &other) _db = other._db;
    return *this;
}
BitcoinExchange::~BitcoinExchange() {}

// ---------- Helpers ----------
std::string BitcoinExchange::trim(const std::string& s) {
    const char* ws = " \t\n\v\f\r";
    std::string::size_type start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    std::string::size_type last = s.find_last_not_of(ws);
    return s.substr(start, last - start + 1);
}

// sep가 딱 1개만 존재해야 true
bool BitcoinExchange::splitOnce(const std::string& line, char sep, std::string& left, std::string& right) {
    std::string::size_type pos = line.find(sep);
    if (pos == std::string::npos) return false;
    if (line.find(sep, pos + 1) != std::string::npos) return false;

    left  = line.substr(0, pos);
    right = line.substr(pos + 1);
    return true;
}

bool BitcoinExchange::isLeapYear(int y) {
    return (y % 4 == 0) && (y % 100 != 0 || y % 400 == 0);
}

int BitcoinExchange::daysInMonth(int y, int m) {
    static const int day[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    if (m == 2) return day[m] + (isLeapYear(y) ? 1 : 0);
    return day[m];
}

bool BitcoinExchange::parseDate(const std::string& s) {
    // 형식: YYYY-MM-DD
    if (s.size() != 10) return false;
    if (s[4] != '-' || s[7] != '-') return false;

    // 숫자 8자리 체크(최소한의 문자 검증)
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    }

    int y = 0, m = 0, d = 0;
    {
        std::istringstream ys(s.substr(0, 4));
        ys >> y;
        if (!ys || !ys.eof()) return false;
    }
    {
        std::istringstream ms(s.substr(5, 2));
        ms >> m;
        if (!ms || !ms.eof()) return false;
    }
    {
        std::istringstream ds(s.substr(8, 2));
        ds >> d;
        if (!ds || !ds.eof()) return false;
    }
	/* 어떤게 좀더 좋을까? 아래는 뭔가 cpp 스럽지 못하다?
    int y = std::atoi(ys.c_str());
    int m = std::atoi(ms.c_str());
    int d = std::atoi(ds.c_str());
	*/

    if (m < 1 || m > 12) return false;
    int dim = daysInMonth(y, m);
    if (d < 1 || d > dim) return false;

    return true;
}

// 공백 제거 후, "+", ".", 과학표기까지 포함해도 스트림이 엄격히 소비(eof)되면 OK
bool BitcoinExchange::parseDoubleStrict(const std::string& s, double& out) {
    std::string t = trim(s);
    if (t.empty()) return false;

    std::istringstream iss(t);
    iss >> out;
    if (!iss || !iss.eof()) return false;

    return true;
}

// date 이하(같거나 이전) 중 가장 가까운 환율
double BitcoinExchange::findRateOnOrBefore(const std::string& date) const {
    // _db는 date string이 YYYY-MM-DD라 lexicographic 정렬 == 시간 정렬
    std::map<std::string, double>::const_iterator it = _db.lower_bound(date);

    if (it == _db.end()) {
        // 마지막 데이터 사용
        if (_db.empty()) throw std::runtime_error("Error: database is empty.");
        --it;
        return it->second;
    }

    if (it->first == date) {
        return it->second;
    }

    // lower_bound가 date보다 큰 첫 원소를 가리킴 -> 이전 원소가 필요
    if (it == _db.begin()) {
        // date보다 이른 데이터가 DB에 없음
        throw std::runtime_error("Error: no earlier rate available.");
    }
    --it;
    return it->second;
}

// ---------- Core ----------
void BitcoinExchange::loadDatabase(const std::string& csvPath) {
    std::ifstream fin(csvPath.c_str());
    if (!fin.is_open()) throw std::runtime_error("Error: could not open database file.");

    std::string line;
    if (!std::getline(fin, line)) throw std::runtime_error("Error: empty database file.");

    // 헤더 검사(엄격하게 할수록 안전)
    line = trim(line);
    if (line != "date,exchange_rate") {
        throw std::runtime_error("Error: invalid database header.");
    }

    std::size_t lineNo = 1;
    while (std::getline(fin, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty()) continue;

        std::string date, rateStr;
        if (!splitOnce(line, ',', date, rateStr)) {
            // DB는 대개 "잘못된 라인은 무시"보다 "즉시 실패"가 디버깅에 유리
            throw std::runtime_error("Error: invalid database line format at line " + trim(line));
        }

        date = trim(date);
        rateStr = trim(rateStr);

        if (!parseDate(date)) {
            throw std::runtime_error("Error: invalid date in database.");
        }

        double rate = 0.0;
        if (!parseDoubleStrict(rateStr, rate)) {
            throw std::runtime_error("Error: invalid rate in database.");
        }
        if (rate < 0.0) {
            throw std::runtime_error("Error: negative rate in database.");
        }

        _db[date] = rate;
    }
}

void BitcoinExchange::processInput(const std::string& inputPath) const {
    std::ifstream fin(inputPath.c_str());
    if (!fin.is_open()) throw std::runtime_error("Error: could not open input file.");

    std::string line;
    if (!std::getline(fin, line)) return;

    // input 헤더: "date | value"
    line = trim(line);
    if (line != "date | value") {
        throw std::runtime_error("Error: invalid input header.");
    }

    while (std::getline(fin, line)) {
        line = trim(line);
        
        if (line.empty()) continue;

        std::string date, valueStr;
        if (!splitOnce(line, '|', date, valueStr)) {
            std::cout << "Error: bad input => " << line << "\n";
            continue;
        }

        date = trim(date);
        valueStr = trim(valueStr);

        if (!parseDate(date)) {
            std::cout << "Error: bad input => " << line << "\n";
            continue;
        }

        double value = 0.0;
        if (!parseDoubleStrict(valueStr, value)) {
            std::cout << "Error: bad input => " << line << "\n";
            continue;
        }

        // subject에서 흔히 요구하는 조건들
        if (value < 0.0) {
            std::cout << "Error: not a positive number.\n";
            continue;
        }
        if (value > 100000.0) {
            std::cout << "Error: too large a number.\n";
            continue;
        }

        try {
            double rate = findRateOnOrBefore(date);
            std::cout << date << " => " << value << " = " << (value * rate) << "\n";
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }
}

