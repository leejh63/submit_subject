/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 18:04:20 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 18:04:21 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>

int main( void ) {
	std::string brain = "HI THIS IS BRAIN";
	std::string* stringPTR = &brain;
	std::string& stringREF = brain;

	std::cout << "-----------------------------\n"
			  << "STR Address :" << &brain << "\n"
			  << "PTR Address :" << stringPTR << "\n"
			  << "RFE Address :" << &stringREF << "\n"
			  << "-----------------------------\n"
			  << "STR value :" << brain << "\n"
			  << "PTR value :" << *stringPTR << "\n"
			  << "RFE value :" << stringREF << "\n"
			  << "-----------------------------\n";
	return 0;
}
