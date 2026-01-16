/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BitcoinExchange.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 11:21:20 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/06 11:21:20 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BitcoinExchange.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cctype>

// 기본 생성자 등등 
BitcoinExchange::BitcoinExchange(void) {}
BitcoinExchange::~BitcoinExchange(void) {}

// private라 미구현
//BitcoinExchange::BitcoinExchange(const BitcoinExchange& other){}
//BitcoinExchange& BitcoinExchange::operator=(const BitcoinExchange& other) {}


//  받은 입력값(s) 제거문자(ws) 앞뒤만 없애줌
std::string BitcoinExchange::trim(const std::string& s) const {

    const char* ws = " \t\n\v\f\r";
    std::string::size_type start = s.find_first_not_of(ws);
    
    // 제거 문자 존재 확인
    if (start == std::string::npos) return "";
    
    std::string::size_type last = s.find_last_not_of(ws);
    
    // 제거후 반환
    return s.substr(start, last - start + 1);
}

// line 받아서 sep기준 앙 옆 나눠줌
bool BitcoinExchange::splitOnce(const std::string& line, char sep, std::string& left, std::string& right) const {
	
    std::string::size_type pos = line.find(sep);
    
    // sep 존재 확인
    if (pos == std::string::npos) return false;

    // 갯수 확인
    if (line.find(sep, pos + 1) != std::string::npos) return false;

	// 분할
    left  = line.substr(0, pos);
    right = line.substr(pos + 1);
    return true;
}

// 윤년 계산
bool BitcoinExchange::isLeapYear(int y) const {
    return (y % 4 == 0) && (y % 100 != 0 || y % 400 == 0);
}

// 월에 따른 날짜 반환
int BitcoinExchange::daysInMonth(int y, int m) const {
    static const int day[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
	
	// 2월 이고, 윤년이면 + 1
    return day[m] + (((m == 2) && isLeapYear(y)) ? 1 : 0);
}

// 날짜 검증
bool BitcoinExchange::parseDate(const std::string& s) const {
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

    if (m < 1 || m > 12) return false;

    int dim = daysInMonth(y, m);
    if (d < 1 || d > dim) return false;

    return true;
}


// double형식 체크
bool BitcoinExchange::parseDouble(const std::string& s, double& out) const {
    std::string t = trim(s);
    if (t.empty()) return false;

    std::istringstream iss(t);

    iss >> out;
    if (!iss || !iss.eof()) return false;

    return true;
}

// date 이하(같거나 이전) 중 가장 가까운 값 반환
double BitcoinExchange::findRateOnOrBefore(const std::string& date) const {
    // _db는 date string이 YYYY-MM-DD라 사전 정렬 == 시간 정렬
    std::map<std::string, double>::const_iterator it = _db.lower_bound(date);

	// date가 가장 클때 마지막 값을 반환
    if (it == _db.end()) {
        --it;
        return it->second;
    }

	// 값이 같다면 그 값 반환
    if (it->first == date) {
        return it->second;
    }

    // date와 it가 같지 않은데 시작값이 나온다면 date가 가장 작음
    if (it == _db.begin()) {
        // date보다 이하 값을 반환 해야하기 때문에 에러 처리
        throw std::runtime_error("Error: no earlier rate available.");
    }
	
	// 값이 존재 안하지만 중간 어딘가 존재, 이하값 반환
    --it;
    return it->second;
}


// csv파일 열고 데이터 파싱 후 컨테이너에 저장
void BitcoinExchange::loadDatabase(const std::string& csv) {

    std::ifstream fin(csv.c_str());
    if (!fin.is_open()) throw std::runtime_error("Error: could not open database file.");

    std::string line;
    if (!std::getline(fin, line)) throw std::runtime_error("Error: empty database file.");

    // 헤더 검사
    line = trim(line);
    if (line != "date,exchange_rate") {
        throw std::runtime_error("Error: invalid database.");
    }

	// DB파일에서 데이터 읽기
    while (std::getline(fin, line)) {
        
        line = trim(line);
        if (line.empty()) continue;

        std::string date, rateStr;
        
        // , 를 기준 양옆 자르기
        if (!splitOnce(line, ',', date, rateStr)) {
            // DB잘못된 라인, 실패
            throw std::runtime_error("Error: invalid database line format at line ==" + line + "==");
        }

        date = trim(date);
        rateStr = trim(rateStr);
		
		//  data 검증
        if (!parseDate(date)) {
            throw std::runtime_error("Error: invalid date in database ==" + date + "==");
        }

        double rate = 0.0;
        //  rate 검증
        if (!parseDouble(rateStr, rate)) {
            throw std::runtime_error("Error: invalid rate in database ==" + rateStr + "==");
        }
        if (rate < 0.0) {
            throw std::runtime_error("Error: negative rate in database ==" + rateStr + "==");
        }
        
		// 멤버변수에 삽입
        _db[date] = rate;
    }
    // 멤버변수가 비어있다면 에러반환
    if (_db.empty()) {
    	throw std::runtime_error("Error: database is empty.");
	}
}

// 입력 데이터를 통해 멤버변수에서 값 찾기
void BitcoinExchange::processInput(const std::string& input) const {
    std::ifstream fin(input.c_str());
    if (!fin.is_open()) throw std::runtime_error("Error: could not open input file.");

	// input 첫줄: "date | value" 이 형태 아니면 에러 반환
    std::string line;
    if (!std::getline(fin, line)) return;
    line = trim(line);
    if (line != "date | value") {
        throw std::runtime_error("Error: invalid input header.");
    }
	
	// 입력값 검증 및 값 출력
    while (std::getline(fin, line)) {
        line = trim(line);
        
        // 입력 값 없으면 다음으로
        if (line.empty()) continue;

        // | 기준으로 양옆 분할
        std::string date, valueStr;
        if (!splitOnce(line, '|', date, valueStr)) {
            std::cout << "Error: bad input => " << line << "\n";
            continue;
        }

        date = trim(date);
        valueStr = trim(valueStr);
		
		// date 형식 검증
        if (!parseDate(date)) {
            std::cout << "Error: bad input => " << line << "\n";
            continue;
        }

		// value 형식 검증
        double value = 0.0;
        if (!parseDouble(valueStr, value)) {
            std::cout << "Error: bad input => " << line << "\n";
            continue;
        }

        if (value < 0.0) {
            std::cout << "Error: not a positive number.\n";
            continue;
        }
        
        if (value > 1000.0) {
            std::cout << "Error: too large a number.\n";
            continue;
        }
		
		// 멤버변수에서 값 찾은후 적절한 출력
        try {
            double rate = findRateOnOrBefore(date);
            std::cout << date << " => " << value << " = " << (value * rate) << "\n";
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }
}

