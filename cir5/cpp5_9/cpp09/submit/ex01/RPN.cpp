/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RPN.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 11:45:03 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/16 11:45:04 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RPN.hpp"
#include <iostream>
#include <limits>
#include <sstream>

RPN::RPN( void ) : _stk() {}
RPN::~RPN( void ) {}

// 미구현
//RPN::RPN( const RPN& copy );
//RPN::RPN& operator=( const RPN& copy );

// stack의 경우 top, pop의 경우 비었는지 확인후 해야함
int RPN::safe_pop( void ) {
	if (_stk.empty()) throw std::runtime_error("input is wrong!");
	int val = _stk.top();
	_stk.pop();
	return val;
}

int RPN::safe_top( void ) {
	if (_stk.empty()) throw std::runtime_error("input is wrong!");
	return _stk.top();
}

// 디버깅용 함수
//void RPN::print_stk( void ) {
//	for (;; safe_pop()) {
//		std::cout << "==" << safe_top() << "==\n";
//	}
//}

// 더하기
void RPN::plus( void ){
	long long right, left, sum;
	// 여기서 입력이 잘못된 경우 에러
	right = safe_pop();
	left = safe_pop();
	
	sum = left + right;
	if (sum > std::numeric_limits<int>::max())
		throw std::overflow_error("overflow");
	_stk.push(static_cast<int>(sum));
}

// 빼기
void RPN::minus( void ){
	long long right, left, sum;
	// 여기서 입력이 잘못된 경우 에러
	right = safe_pop();
	left = safe_pop();

	sum = left - right;
	if (sum < std::numeric_limits<int>::min())
		throw std::overflow_error("overflow");
	_stk.push(static_cast<int>(sum));
}

// 곱하기
void RPN::multiply( void ){
	long long right, left, sum;
	// 여기서 입력이 잘못된 경우 에러
	right = safe_pop();
	left = safe_pop();
	
	sum = left * right;
	if (sum > std::numeric_limits<int>::max() || sum < std::numeric_limits<int>::min())
		throw std::overflow_error("overflow");
	_stk.push(static_cast<int>(sum));
}

// 나누기
void RPN::divide( void ){
	long long right, left, sum;
	// 여기서 입력이 잘못된 경우 에러
	right = safe_pop();
	
	// 0 나누기 에러
	if (right == 0) throw std::runtime_error("divide zero!");
	
	left = safe_pop();
	
	sum = left / right;
	if (sum < std::numeric_limits<int>::min())
		throw std::overflow_error("overflow");
	_stk.push(sum);
}



void RPN::calculate_input(const std::string& input) {
    while (!_stk.empty()) _stk.pop();

    std::istringstream iss(input);
    std::string tok;

    while (iss >> tok) {
        // 한 문자인지
        if (tok.size() != 1) { throw std::runtime_error("input is wrong!"); }
        
        // 숫자인지
        if (std::isdigit(static_cast<unsigned char>(tok[0]))) {
            _stk.push(tok[0] - '0');
        }
        else 
        {
        	// *+-/ 이외일 경우 에러반환
            switch (tok[0]) {
                case '+': plus(); break;
                case '-': minus(); break;
                case '*': multiply(); break;
                case '/': divide(); break;
                default : throw std::runtime_error("input is wrong!");
            }
        }
    }
	
	// 계산 후 스택에 하나가 아니라면 에러
    if (_stk.size() != 1)
        throw std::runtime_error("input is wrong!");

    std::cout << safe_pop() << "\n";
}

