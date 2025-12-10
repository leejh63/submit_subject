/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:31:26 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/13 13:31:26 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Contact.hpp"
#include "PhoneBook.hpp"

int print_e(int num) {
	if (num == 1) { std::cout << "input is empty! try again!\n"; return 1; }
	if (num == 2) { std::cout << "input is only space! try again!\n"; return 2; }
	if (num == 3) { std::cout << "Something wrong!\n"; return 3; }
	if (num == 4) { std::cout << "Not cmd! try again!\n"; return 4; }
	if (num == 5) { std::cout << "Close the program!\n"; return 5; }
	return -1;
}

int mkinput(std::string& input) {
	const static char* trimword = " \t\n\v\f\r";
	std::size_t start, last;
	if (input.empty()) { return print_e(1); }
	start = input.find_first_not_of(trimword);
	if (start == std::string::npos)  { return print_e(2); }
	last = input.find_last_not_of(trimword);
	input = input.substr(start, last - start + 1);
	return 0;
}

int addinfo( PhoneBook& pb ) {
	const char* prompts[5] = {
		"First name : ",
		"Last name  : ",
		"Nickname   : ",
		"Phone      : ",
		"Secret     : "
	};

	std::cout << "input information!\n";
	std::size_t i = 0;
	std::string conset[5];
	std::string input;
	while(i < 5) {
		std::cout << prompts[i]; 
		if (!std::getline(std::cin, input)) { return 1; }
		if (mkinput(input)) { continue; }
		conset[i] = input;
		i++;
	}
	pb.add(conset);
	return 0;
}

int main( void ){
	PhoneBook pb;
	std::string cmd;
	while(true) {
		std::cout << "cmd	: ";
		if (!std::getline(std::cin, cmd)) { return print_e(3); }
		if (mkinput(cmd)) { continue; }
		if (cmd == "EXIT") { return print_e(5); }
		else if (cmd == "ADD") { if (addinfo(pb)) return print_e(3); }
		else if (cmd == "SEARCH") { pb.search(); }
		else { print_e(4); continue; }
	}
	return 0;
}
