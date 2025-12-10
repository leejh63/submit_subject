/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PhoneBook.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:31:36 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/13 13:31:39 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PhoneBook.hpp"
#include <sstream>

PhoneBook::PhoneBook( void ) : now(0) {
	//std::cout << "PhoneBook()\n";
}

PhoneBook::~PhoneBook( void ) {
	//std::cout << "~PhoneBook()\n";
}

void PhoneBook::add( const std::string conset[5] ) {
	carray[now].set(conset);
	now = (now + 1) % 8;
}

void PhoneBook::printen( std::string word ) const {
	if (word.size() > 10) { word = word.substr(0, 9) + "."; }
	std::cout << '|' << std::setw(10) << std::setfill(' ') << std::right << word;
}

void PhoneBook::search( void ) {
	const std::string info[4] = {"index", "first name", "last name", "nick name"};
	for (std::size_t i = 0; i < 4; ++i) {	
		printen(info[i]);
	}
	std::cout << "|\n";
	
	for (std::size_t i = 0; i < 8; ++i) {
		if (carray[i].isempty()) { break; }
		std::cout << '|' << std::setw(10) << std::setfill(' ') << std::right << i;
		printen(carray[i].first());
		printen(carray[i].last());
		printen(carray[i].nick());
		std::cout << "|\n";
	}
	
	for (std::size_t i = 0; i < 4; ++i) {	
		printen("");
	}

	std::cout << "|\nEnter the index for which more information is needed!\nindex : ";
	std::string ind;
	if (!std::getline(std::cin, ind)) { 
		std::cout << "Somthing Wrong!\n"; 
		return ;
	}

	if (ind.empty()) { 
		std::cout << "Enter the index! try again!\n"; 
		return;
	}
	
	std::istringstream iss(ind);
	int index;
	if (!(iss >> index) || (iss >> std::ws, !iss.eof())) {
		std::cout << "Invalid index! try again!\n";
		return;
	}
	if (0 > index || index > 7) {
		std::cout << "Out of range! try again!\n";
		return;
	}
	if (carray[index].isempty()) {
		std::cout << "The contact at index " << index << " is empty!\n";
		return;
	}
	carray[index].pmem();
}





















