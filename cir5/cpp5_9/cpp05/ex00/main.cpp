/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 18:31:19 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 18:31:20 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bureaucrat.hpp"
#include <iostream>

int main( void ) {
	try {
		std::cout << "=== Valid construction test ===" << std::endl;
		Bureaucrat a("Alice");
		std::cout << a << std::endl;

		std::cout << "\n=== Increment/Decrement test ===" << std::endl;
		a.increGrade();
		std::cout << a << std::endl;
		a.decreGrade();
		std::cout << a << std::endl;

		std::cout << "\n=== Copy and assignment test ===" << std::endl;
		Bureaucrat b = a;        // copy constructor
		std::cout << b << std::endl;

		Bureaucrat c("Charlie", 100);
		c = a;                   // assignment operator
		std::cout << c << std::endl;

		std::cout << "\n=== Exception test (too high) ===" << std::endl;
		Bureaucrat high("High", 1);
		std::cout << high << std::endl;
		high.increGrade();   // should throw
		std::cout << "This should not print!" << std::endl;
	}
	catch (std::exception &e) {
		std::cerr << "Caught exception(1 - 1): " << e.what() << std::endl;
	}

	try {
		std::cout << "\n=== Exception test (too low) ===" << std::endl;
		Bureaucrat low("Low", 150);
		std::cout << low << std::endl;
		low.decreGrade();   // should throw
		std::cout << "This should not print!" << std::endl;
	}
	catch (std::exception &e) {
		std::cerr << "Caught exception(150 + 1): " << e.what() << std::endl;
	}

	try {
		std::cout << "\n=== Invalid construction ===" << std::endl;
		Bureaucrat wrong("ErrorCase", 151);  // should throw at construction
	}
	catch (std::exception &e) {
		std::cerr << "Caught exception(151): " << e.what() << std::endl;
	}

	return 0;
}

