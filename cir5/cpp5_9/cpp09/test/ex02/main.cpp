/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/05 11:51:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/05 11:51:53 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PmergeMe.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <ctime>

static void printSequence(const std::vector<int>& v)
{
    for (size_t i = 0; i < v.size(); ++i)
    {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << " ";
    }
    std::cout << "\n";
}

static std::list<int> buildListFromVector(const std::vector<int>& v)
{
    std::list<int> l;
    for (size_t i = 0; i < v.size(); ++i)
        l.push_back(v[i]);
    return l;
}

// Use clock() for strict C++98 portability.
// We'll report microseconds-ish: (seconds * 1e6).
static double elapsedMicros(clock_t start, clock_t end)
{
    double sec = static_cast<double>(end - start) / static_cast<double>(CLOCKS_PER_SEC);
    return sec * 1000000.0;
}

int main(int argc, char** argv)
{
    std::vector<int> input;
    if (!PmergeMe::parsePositiveInts(argc, argv, input))
    {
        std::cerr << "Error\n";
        return 1;
    }

    // Output "Before"
    std::cout << "Before: ";
    printSequence(input);

    // ---- Vector timing: include data management (copy) + sort ----
    clock_t v_start = clock();
    std::vector<int> v = input;
    PmergeMe::sortVectorFordJohnson(v);
    clock_t v_end = clock();

    if (!PmergeMe::isSortedVector(v))
    {
        std::cerr << "Error\n";
        return 1;
    }

    // Output "After" (use sorted vector)
    std::cout << "After:  ";
    printSequence(v);

    // ---- List timing: include data management (build) + sort ----
    clock_t l_start = clock();
    std::list<int> l = buildListFromVector(input);
    PmergeMe::sortListFordJohnson(l);
    clock_t l_end = clock();

    if (!PmergeMe::isSortedList(l))
    {
        std::cerr << "Error\n";
        return 1;
    }

    // Display times (precision enough to see difference)
    const size_t n = input.size();

    std::cout << std::fixed << std::setprecision(5);
    std::cout << "Time to process a range of " << n
              << " elements with std::vector : " << elapsedMicros(v_start, v_end) << " us\n";
    std::cout << "Time to process a range of " << n
              << " elements with std::list   : " << elapsedMicros(l_start, l_end) << " us\n";

    return 0;
}

