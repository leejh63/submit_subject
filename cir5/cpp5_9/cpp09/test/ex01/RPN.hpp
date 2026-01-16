/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RPN.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 11:45:01 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/16 11:45:02 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RPN_HPP
#define RPN_HPP

#include <stack>
#include <cctype>
#include <string>
#include <stdexcept>

class RPN {
	private:
		std::stack<int> _stk;
		
		int safe_pop( void );
		int safe_top( void );
		
		void plus( void );
		void minus( void );
		void multiply( void );
		void divide( void );
		
		
		RPN( const RPN& copy );
		RPN& operator=( const RPN& copy );
	public:
		RPN( void );
		~RPN( void );
		void calculate_input( const std::string& input );
		void print_stk( void );
};

#endif
