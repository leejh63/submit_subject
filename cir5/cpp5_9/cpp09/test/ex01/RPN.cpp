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

RPN::RPN( void ) 
: _stk() {}

RPN::~RPN( void ) {}


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

void RPN::print_stk( void ) {
	for (;; safe_pop()) {
		std::cout << "==" << safe_top() << "==\n";
	}
}
		
void RPN::plus( void ){
	long long right, left, sum;
	left = safe_pop();
	right = safe_pop();
	
	sum = left + right;
	if (sum > std::numeric_limits<int>::max())
		throw std::overflow_error("overflow");
	_stk.push(static_cast<int>(sum));
}

void RPN::minus( void ){
	long long right, left, sum;
	left = safe_pop();
	right = safe_pop();

	sum = left - right;
	if (sum < std::numeric_limits<int>::min())
		throw std::overflow_error("overflow");
	_stk.push(static_cast<int>(sum));
}

void RPN::multiply( void ){
	long long right, left, sum;
	left = safe_pop();
	right = safe_pop();
	
	sum = left * right;
	if (sum > std::numeric_limits<int>::max() || sum < std::numeric_limits<int>::min())
		throw std::overflow_error("overflow");
	_stk.push(static_cast<int>(sum));
}

void RPN::divide( void ){
	long long right, left, sum;
	right = safe_pop();
	if (right == 0) throw std::runtime_error("divide zero!");
	
	left = safe_pop();
	
	sum = left / right;
	if (sum < std::numeric_limits<int>::min())
		throw std::overflow_error("overflow");
	_stk.push(left / right);
}


void RPN::calculate_input( const std::string& input ) {
	std::cout << "====|test|====\n";
	unsigned char check_char;
	for (std::size_t i = 0; i < input.size(); ++i) {
		check_char = static_cast<unsigned char>(input[i]);
		if (!std::isdigit(check_char)) {
			switch (check_char) {
				case '+':
					std::cout << "plus!=\n"; plus(); continue;
				case '-':
					std::cout << "minus!=\n"; minus(); continue;
				case '*':
					std::cout << "multi!=\n"; multiply(); continue;
				case '/':
					std::cout << "div!=\n"; divide(); continue;
				case ' ':
					//std::cout << "space! how should i handle this word? just skip? or throw the error?\n";
					continue;
				default :
					throw std::runtime_error("input is wrong!");
			}
		}
		std::cout << "number!  =" << check_char << "=\n";
		_stk.push(input[i] - '0');
	}
	if (_stk.size() != 1) { throw std::runtime_error("input is wrong!"); }
	std::cout << "====calculate is complete!! value: " << safe_pop() << "====\n"; 
}

