/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Contact.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:31:55 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/13 13:31:56 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Contact.hpp"

Contact::Contact( void ) {
	//std::cout << "call Contact()\n";
}


Contact::~Contact( void ) {
	//std::cout << "call ~Contact\n";
}

void Contact::pmem( void ) const {
    std::cout << "First Name     : " << _firstname     << "\n"
              << "Last Name      : " << _lastname      << "\n"
              << "Nickname       : " << _nickname      << "\n"
              << "Phone Number   : " << _phonenumber   << "\n"
              << "Darkest Secret : " << _darkestsecret << "\n";
}

bool Contact::isempty( void ) const {
	if (_firstname.empty()) { return true; }
	else { return false; }
}

const std::string& Contact::first( void ) const { return _firstname; }
const std::string& Contact::last( void ) const { return _lastname; }
const std::string& Contact::nick( void ) const { return _nickname; }

void Contact::set(const std::string conset[5]) {
	//std::cout << "set Contact[5]\n";
    _firstname     = conset[0];
    _lastname      = conset[1];
    _nickname      = conset[2];
    _phonenumber   = conset[3];
    _darkestsecret = conset[4];
}

















