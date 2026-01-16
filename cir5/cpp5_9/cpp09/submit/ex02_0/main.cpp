/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 18:34:59 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/06 18:35:01 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PmergeMe.hpp"

#include <iostream>
#include <list>
#include <vector>
#include <stdexcept>

template <typename It>
bool isSortedAscending(It begin, It end)
{
	if (begin == end) return true;

	It prev = begin;
	It cur = begin;
	++cur;

	for (; cur != end; ++prev, ++cur) {
		if (*prev > *cur)
			return false;
	}
	return true;
}

bool isSameSequence(const std::vector<int>& v,
                    const std::list<int>& l)
{
	if (v.size() != l.size())
		return false;

	std::vector<int>::const_iterator vit = v.begin();
	std::list<int>::const_iterator  lit = l.begin();

	for (; vit != v.end(); ++vit, ++lit) {
		if (*vit != *lit)
			return false;
	}
	return true;
}

int main(int argc, char** argv)
{
	try {
		std::vector<int> input = PmergeMe::parseArgs(argc, argv);
		//
		std::cout << "Before:";
		for (size_t i = 0; i < input.size(); ++i)
			std::cout << " " << input[i];
		std::cout << "\n";
		//
		// vector sort
		std::vector<int> sortedVec;
		double tVec = PmergeMe::timeSortVector(input, sortedVec);

		// list sort
		std::list<int> inputList(input.begin(), input.end());
		std::list<int> sortedList;
		double tList = PmergeMe::timeSortList(inputList, sortedList);

		// After 출력(보통 vector 결과로 출력)
		std::cout << "After:";
		//
		for (size_t i = 0; i < sortedVec.size(); ++i)
			std::cout << " " << sortedVec[i];
		std::cout << "\n";
		//
		std::cout << "Time to process a range of " << input.size()
				  << " elements with std::vector : " << tVec << " us\n";
		std::cout << "Time to process a range of " << input.size()
				  << " elements with std::list   : " << tList << " us\n";


		// === 정렬 검증 ===
		if (input.size() != sortedVec.size()) 
			throw std::runtime_error("Vector not sorted1");

		if (!isSortedAscending(sortedVec.begin(), sortedVec.end()))
			throw std::runtime_error("Vector not sorted2");

		if (input.size() != sortedList.size()) 
			throw std::runtime_error("List not sorted1");
		
		if (!isSortedAscending(sortedList.begin(), sortedList.end()))
			throw std::runtime_error("List not sorted2");

		if (!isSameSequence(sortedVec, sortedList))
			throw std::runtime_error("Vector/List mismatch");

		// 정상일 때만 출력
		std::cout << "OK: vector & list sorted correctly\n";

		return 0;
	}
	catch (const std::exception&) {
		std::cerr << "Error\n";
		return 1;
	}
}

