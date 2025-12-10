/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 13:26:55 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/09 13:26:55 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "A.hpp"
#include "B.hpp"
#include "C.hpp"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <exception>

Base* generate( void ) {
	std::srand(std::time(0));
	int r = std::rand() % 3;

	if (r == 0) { return new A; }
	else if (r == 1) { return new B; }
	return new C;
}

void identify(Base* p) {
	std::cout << "the type of this ptr is ";
	if (dynamic_cast<A*>(p)) {  std::cout << "\'A\'!\n"; return; }
	else if (dynamic_cast<B*>(p)) {  std::cout << "\'B\'!\n"; return; }
	else if (dynamic_cast<C*>(p)) {  std::cout << "\'C\'!\n"; return; }
	std::cout << "\'nullptr\'!\n";
}

void identify(Base& p) {
	try {
		(void)dynamic_cast<A&>(p);
		std::cout << "the type of this ref is \'A\'!\n";
		return ;
	} catch (...) {}
	try {
		(void)dynamic_cast<B&>(p);
		std::cout << "the type of this ref is \'B\'!\n";
		return ;
	} catch (...) {}
	try {
		(void)dynamic_cast<C&>(p);
		std::cout << "the type of this ref is \'C\'!\n";
		return ;
	}
	catch (const std::exception& e){
		std::cout << e.what() << ": Something is wrong!\n";
	}
}

int main( void ) {
	
	Base* gen_ptr = generate();
	Base& gen_ref = *gen_ptr;
	identify(gen_ptr);
	identify(gen_ref);
	delete gen_ptr;
	return 0;
}
