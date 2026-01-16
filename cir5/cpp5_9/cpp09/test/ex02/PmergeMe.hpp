/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PmergeMe.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/05 11:51:30 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/05 11:51:32 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <vector>
#include <list>
#include <string>

class PmergeMe
{
	public:
		// Parse arguments into a vector<int> (positive ints only).
		// Returns true on success, false on error.
		static bool parsePositiveInts(int argc, char** argv, std::vector<int>& out);

		// Ford–Johnson sort implementations (container-specific; no generic template).
		static void sortVectorFordJohnson(std::vector<int>& v);
		static void sortListFordJohnson(std::list<int>& l);

		// Utility: check sorted
		static bool isSortedVector(const std::vector<int>& v);
		static bool isSortedList(const std::list<int>& l);

	private:
		// --- Vector implementation helpers ---
		static void fordJohnsonVector(std::vector<int>& v);
		static std::vector<size_t> buildJacobInsertionOrder(size_t m);

		// --- List implementation helpers ---
		static void fordJohnsonList(std::list<int>& l);
		static std::list<int>::iterator lowerBoundList(std::list<int>& l, int value);
};

#endif

