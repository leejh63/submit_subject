/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PmergeMe.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 18:34:55 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/06 18:34:55 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <vector>
#include <list>
#include <string>
#include <utility> // pair
#include <cstddef> // size_t

class PmergeMe {
public:
    static std::vector<int> parseArgs(int argc, char** argv);

    static std::vector<int> fordJohnsonSortVector(const std::vector<int>& input);
    static std::list<int>   fordJohnsonSortList(const std::list<int>& input);

    static double timeSortVector(const std::vector<int>& input, std::vector<int>& outSorted);
    static double timeSortList(const std::list<int>& input, std::list<int>& outSorted);

    static void printSequence(const char* label, const std::vector<int>& seq);
    static void printSequence(const char* label, const std::vector<size_t>& seq);

private:
    PmergeMe();
    ~PmergeMe();

    static void validateAndNoDup(const std::vector<int>& v);

    static std::vector<size_t> buildJacobsthalInsertionOrder(size_t pairCount);

    // vector helpers
    static void binaryInsertVector(std::vector<int>& chain,
                                   std::vector<int>::iterator endPos,
                                   int value);

    // list helpers (기존 선언 유지)
    static std::list<int>::iterator lowerBoundListRange(std::list<int>::iterator beginIt,
                                                        std::list<int>::iterator endIt,
                                                        int value);

    static void insertListBeforeBound(std::list<int>& chain,
                                      std::list<int>::iterator endPos,
                                      int value);

    // ===== 추가된 list-only Ford-Johnson helper들 =====
    static std::list<size_t> buildJacobsthalInsertionOrderList(size_t m);
    static bool containsIndex(const std::list<size_t>& lst, size_t x);

    static std::list< std::pair<int,int> >::iterator
           nthPendingIterator(std::list< std::pair<int,int> >& lst, size_t idx1);

    static std::list<int>::iterator boundByBigValue(std::list<int>& chain, int bigVal);
};

#endif


