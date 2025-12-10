/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Converter.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 11:54:33 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/08 16:06:25 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Converter.hpp"
#include <iostream>
#include <cctype>
#include <limits>
#include <cstdlib>
#include <iomanip>

namespace {
	
	enum TYPE_ { 
		CHAR, 
		INT, 
		FLOAT, 
		DOUBLE, 
		NAN_INF_FLOAT,
		NAN_INF_DOUBLE,
		INVALID 
	};
	
	bool is_Char( const std::string& word ) {
		if (word.size() == 1 && !std::isdigit(static_cast<unsigned char>(word[0])))
			return true;
		return false;
	}
	
	bool is_Int( const std::string& word ) {
		if (word.empty()) { return false; }
		std::size_t i = 0;
		if (word[i] == '+' || word[i] == '-') { ++i; }
		for (; i < word.size(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(word[i])))
				return false;
		}
		return true;
	}
	
	bool is_Float( const std::string& word ) {
		std::size_t len = word.size();
		
		if (len < 2) { return false; }
		
		if (word[--len] != 'f') { return false; }
		
		std::size_t i = 0;
		bool dot = false;
		
		if (word[i] == '+' || word[i] == '-') { ++i; }
		if (i == len) { return false; }
		
		for (; i < len; ++i) {
			if (!std::isdigit(static_cast<unsigned char>(word[i]))) {
				if (word[i] == '.') {
					if (dot) { return false; }
					dot = true;
				}
				else { return false; }
			}
		}
		return dot;
	}
	
	bool is_Double( const std::string& word ) {
		if (word.empty()) { return false; }
		
		std::size_t len = word.size();
		std::size_t i = 0;
		bool dot = false;
		
		if (word[i] == '+' || word[i] == '-') { ++i; }
		if (i == len) { return false; }
		
		for (; i < len; ++i) {
			if (!std::isdigit(static_cast<unsigned char>(word[i]))) {
				if (word[i] == '.') {
					if (dot) { return false; }
					dot = true;
				}
				else { return false; }
			}
		}
		return dot;
	}
	
	bool is_Naninf_Float(const std::string& word) {
		return word == "nanf" || word == "inff" || word == "+inff" || word == "-inff";
    }

  	bool is_Naninf_Double(const std::string& word) {
		return word == "nan" || word == "inf" || word == "+inf" || word == "-inf" ;
    }
	
	TYPE_ check_type( const std::string& word ) {
		if (is_Char(word))
			return CHAR;
		if (is_Int(word))
			return INT;
		if (is_Float(word))
			return FLOAT;
		if (is_Double(word))
			return DOUBLE;
		if (is_Naninf_Float(word))
			return NAN_INF_FLOAT;
		if (is_Naninf_Double(word))
			return NAN_INF_DOUBLE;
		return INVALID;
	}
	
    void printChar(double value, bool impossible) {
        std::cout << "char: ";
        if (impossible || value < std::numeric_limits<char>::min()
                     || value > std::numeric_limits<char>::max()) {
            std::cout << "impossible" << std::endl;
            return;
        }
        
        // 여기서 "정수가 아니면 char 변환 안 함" 규칙 추가 필요
		if (std::floor(value) != value) {
		    std::cout << "impossible" << std::endl;
		    return;
		}
		
        char c = static_cast<char>(value);
        if (!std::isprint(static_cast<unsigned char>(c))) {
            std::cout << "Non displayable" << std::endl;
        } else {
            std::cout << "'" << c << "'" << std::endl;
        }
    }
    
    
    void printInt(double value, bool impossible) {
        std::cout << "int: ";
        if (impossible || value < std::numeric_limits<int>::min()
                     || value > std::numeric_limits<int>::max()) {
            std::cout << "impossible" << std::endl;
            return;
        }
        int i = static_cast<int>(value);
        std::cout << i << std::endl;
    }

    void printFloat(double value, bool impossible, const std::string &input) {
        std::cout << "float: ";
        if (impossible) {
            if (input == "nan" || input == "nanf")
                std::cout << "nanf" << std::endl;
            else if (input[0] == '-')
                std::cout << "-inff" << std::endl;
            else
                std::cout << "+inff" << std::endl;
            return;
        }
        float f = static_cast<float>(value);
        std::cout << std::fixed << std::setprecision(1) << f << "f" << std::endl;
    }

    void printDouble(double value, bool impossible, const std::string &input) {
        std::cout << "double: ";
        if (impossible) {
            if (input == "nan" || input == "nanf")
                std::cout << "nan" << std::endl;
            else if (input[0] == '-')
                std::cout << "-inf" << std::endl;
            else
                std::cout << "+inf" << std::endl;
            return;
        }
        std::cout << std::fixed << std::setprecision(1) << value << std::endl;
    }
}

void Converter::convert( const std::string& word ) {

	double value = 0.0;
    bool impossible = false;

	TYPE_ separater = check_type(word);
	switch (separater) {
		case CHAR:
			value = static_cast<double>(word[0]); break;
		case INT:
			//value = static_cast<int>(std::strtol(word.c_str(), 0, 10));  break;
			value = static_cast<double>(std::strtol(word.c_str(), 0, 10));  break;
		case FLOAT: {
			std::string trimmed = word.substr(0, word.size() - 1);
			value = std::strtod(trimmed.c_str(), 0);
			break;
		}
		case DOUBLE:
			value = std::strtod(word.c_str(), 0); break;
		case NAN_INF_FLOAT:
		case NAN_INF_DOUBLE:
			impossible = true;
		    if (word == "nan" || word == "nanf") {
		        value = std::numeric_limits<double>::quiet_NaN();
		    }
		    else if (word[0] == '-') {
		        value = -std::numeric_limits<double>::infinity();
		    }
		    else {
		        value = std::numeric_limits<double>::infinity();
			}
			break;
		default:
			std::cout << "char: impossible" << std::endl;
			std::cout << "int: impossible" << std::endl;
			std::cout << "float: impossible" << std::endl;
			std::cout << "double: impossible" << std::endl;
        	return;
	}
    printChar(value, impossible);
    printInt(value, impossible);
    printFloat(value, impossible, word);
    printDouble(value, impossible, word);
}











































