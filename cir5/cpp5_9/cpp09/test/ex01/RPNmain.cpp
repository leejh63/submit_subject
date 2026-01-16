/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RPNmain.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 11:44:59 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/16 11:44:59 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RPN.hpp"
#include <iostream>

int main( int argc, char* argv[] ) {
    if (argc != 2) {
        std::cout << "Usage: ./RPN <input_data>\n";
        return 1;
    }
    std::string  input = argv[1];
    try {   
	    RPN test;
	    test.calculate_input(input);
    }
    catch(const std::exception& e) {
    	std::cout << "Error :" << e.what() << '\n';
    }
    
    return 0;
}
