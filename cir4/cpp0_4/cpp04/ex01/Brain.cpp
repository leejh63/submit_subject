/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   brain.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 12:18:39 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/23 12:18:40 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Brain.hpp"
#include <iostream>

namespace {
	//
	bool skip = false;
	/*/
	bool skip = true;
	//*/
	static const std::string what = "I don’t have any ideas.";
}

void Brain::p_log( const std::string& log_msg) const {
	if (skip) { return; }
    std::cout << "[Brain] " << log_msg << "\n";
}

Brain::Brain( void ) {
	p_log("Default Constructor");
}

Brain::Brain( const std::string& only ) {
	p_log("String Constructor");
	for (int i = 0; i < 100; ++i) {
		this->ideas[i] = only;
	}
}

Brain::Brain( const Brain& copy ) {
	p_log("Copy Constructor");
	for (int i = 0; i < 100; ++i) {
		this->ideas[i] = copy.ideas[i];
	}
}

Brain::~Brain( void ) {
	p_log("Destructor");
}

Brain& Brain::operator=( const Brain& copy ){
	p_log("Assign Operator");
	for (int i = 0; i < 100; ++i) {
		this->ideas[i] = copy.ideas[i];
	}
	return *this;
}

const std::string& Brain::getIdeas( int mem ) const{
	p_log("getIdeas");
	if (mem < 0 || 100 <= mem) { return what; }
	if (ideas[mem].empty()) { return what; }
	return this->ideas[mem];
}

void Brain::setIdeas( int mem_ind, const std::string& novel ) {
	p_log("setIdeas");
	if (mem_ind < 0 || 100 <= mem_ind) { std::cout << what << "\n"; return; }
	if (novel.empty()) { std::cout << what << "\n"; return; }
	this->ideas[mem_ind] = novel;
}

