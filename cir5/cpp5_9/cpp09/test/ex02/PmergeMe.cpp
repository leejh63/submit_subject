/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PmergeMe.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/05 11:51:36 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/05 11:51:37 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PmergeMe.hpp"

#include <cstdlib>   // std::strtol
#include <climits>   // INT_MAX
#include <cctype>    // std::isdigit

// ---------------- Parsing ----------------

static bool isAllDigits(const std::string& s)
{
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    }
    return true;
}

bool PmergeMe::parsePositiveInts(int argc, char** argv, std::vector<int>& out)
{
    out.clear();
    if (argc <= 1) return false;

    for (int i = 1; i < argc; ++i)
    {
        std::string token(argv[i]);

        // Reject signs, spaces, non-digits
        if (!isAllDigits(token))
            return false;

        // Convert
        char* end = 0;
        long val = std::strtol(token.c_str(), &end, 10);
        if (end == 0 || *end != '\0')
            return false;

        // Positive integer only (strictly > 0)
        if (val <= 0 || val > INT_MAX)
            return false;

        out.push_back(static_cast<int>(val));
    }
    return true;
}

// ---------------- Jacobsthal insertion order ----------------
// Return indices in insertion order for pend[1..m-1] (pend[0] handled separately).
// Typical 42 approach:
// j_prev=1 (J2), j_curr=3 (J3), then insert ranges (j_curr ... j_prev+1) descending.
// finally insert remaining tail (m-1 ... j_prev+1).
std::vector<size_t> PmergeMe::buildJacobInsertionOrder(size_t m)
{
    // We insert pend[0] first elsewhere.
    // This function must return an order covering ALL indices in [1 .. m-1]
    // in a Jacobsthal-inspired sequence.

    std::vector<size_t> order;
    if (m <= 1)
        return order;

    const size_t maxIdx = m - 1;

    // Generate Jacobsthal numbers: J0=0, J1=1, Jn = J(n-1) + 2*J(n-2)
    // We use the subsequence starting from J2=1, J3=3, J4=5, J5=11, ...
    std::vector<size_t> J;
    {
        size_t j0 = 0;
        size_t j1 = 1;
        J.push_back(1); // J1 (we will treat as J2=1 anchor for blocks)
        // Build up until exceeding maxIdx (need some headroom)
        while (true)
        {
            size_t j2 = j1 + 2 * j0;
            j0 = j1;
            j1 = j2;
            if (j1 == 1) continue; // skip duplicate early
            J.push_back(j1);
            if (j1 > maxIdx)
                break;
        }
    }

    // Track what indices we already scheduled (avoid duplicates)
    std::vector<bool> used(m, false);
    used[0] = true; // inserted separately

    // Jacob blocks: for each Jacob number (starting from 3-ish), insert the range (end .. prev+1) descending
    size_t prev = 1; // conceptually J2 = 1
    for (size_t t = 0; t < J.size(); ++t)
    {
        size_t curr = J[t];
        if (curr <= 1)
            continue;

        if (prev >= maxIdx)
            break;

        size_t end = curr;
        if (end > maxIdx) end = maxIdx;

        for (size_t idx = end; idx > prev; --idx)
        {
            if (!used[idx])
            {
                order.push_back(idx);
                used[idx] = true;
            }
        }
        prev = curr;
    }

    // Append any missing indices in descending order to ensure full coverage [1..maxIdx]
    for (size_t idx = maxIdx; idx >= 1; --idx)
    {
        if (!used[idx])
        {
            order.push_back(idx);
            used[idx] = true;
        }
        if (idx == 1) break; // prevent size_t underflow
    }

    return order;
}

// ---------------- Vector sort ----------------

void PmergeMe::sortVectorFordJohnson(std::vector<int>& v)
{
    fordJohnsonVector(v);
}

void PmergeMe::fordJohnsonVector(std::vector<int>& v)
{
    if (v.size() <= 1)
        return;

    // 1) Pairing phase: build main (max of each pair) and pend (min of each pair)
    std::vector<int> mainChain;
    std::vector<int> pend;
    mainChain.reserve((v.size() + 1) / 2);
    pend.reserve((v.size() + 1) / 2);

    bool hasStraggler = (v.size() % 2 != 0);
    int straggler = 0;

    for (size_t i = 0; i + 1 < v.size(); i += 2)
    {
        int a = v[i];
        int b = v[i + 1];
        if (a < b)
        {
            int tmp = a; a = b; b = tmp;
        }
        // a = max, b = min
        mainChain.push_back(a);
        pend.push_back(b);
    }

    if (hasStraggler)
        straggler = v.back();

    // 2) Recursively sort main chain (Ford–Johnson recursion)
    fordJohnsonVector(mainChain);

    // 3) Insert pend elements into main chain using Jacobsthal order
    //    Start from sorted mainChain as result.
    std::vector<int> result = mainChain;

    if (!pend.empty())
    {
        // Insert pend[0] first
        {
            int x = pend[0];
            std::vector<int>::iterator pos =
                std::lower_bound(result.begin(), result.end(), x);
            result.insert(pos, x);
        }

        // Insert remaining pend in Jacobsthal-defined order
        std::vector<size_t> order = buildJacobInsertionOrder(pend.size());
        for (size_t k = 0; k < order.size(); ++k)
        {
            size_t idx = order[k];
            int x = pend[idx];
            std::vector<int>::iterator pos =
                std::lower_bound(result.begin(), result.end(), x);
            result.insert(pos, x);
        }
    }

    // 4) Insert straggler (if any)
    if (hasStraggler)
    {
        std::vector<int>::iterator pos =
            std::lower_bound(result.begin(), result.end(), straggler);
        result.insert(pos, straggler);
    }

    v.swap(result);
}

// ---------------- List sort ----------------

void PmergeMe::sortListFordJohnson(std::list<int>& l)
{
    fordJohnsonList(l);
}

std::list<int>::iterator PmergeMe::lowerBoundList(std::list<int>& l, int value)
{
    std::list<int>::iterator it = l.begin();
    while (it != l.end() && *it < value)
        ++it;
    return it;
}

void PmergeMe::fordJohnsonList(std::list<int>& l)
{
    if (l.size() <= 1)
        return;

    // 1) Pairing phase
    std::list<int> mainChain;
    std::vector<int> pend;
    pend.reserve((l.size() + 1) / 2);

    bool hasStraggler = (l.size() % 2 != 0);
    int straggler = 0;

    std::list<int>::iterator it = l.begin();
    while (it != l.end())
    {
        int first = *it;
        ++it;
        if (it == l.end())
        {
            // straggler
            straggler = first;
            break;
        }
        int second = *it;
        ++it;

        int a = first;
        int b = second;
        if (a < b)
        {
            int tmp = a; a = b; b = tmp;
        }
        mainChain.push_back(a);
        pend.push_back(b);
    }

    // 2) Recursively sort main chain
    fordJohnsonList(mainChain);

    // 3) Insert pend into main chain (linear lower_bound because list)
    if (!pend.empty())
    {
        // pend[0]
        {
            int x = pend[0];
            std::list<int>::iterator pos = lowerBoundList(mainChain, x);
            mainChain.insert(pos, x);
        }

        std::vector<size_t> order = buildJacobInsertionOrder(pend.size());
        for (size_t k = 0; k < order.size(); ++k)
        {
            size_t idx = order[k];
            int x = pend[idx];
            std::list<int>::iterator pos = lowerBoundList(mainChain, x);
            mainChain.insert(pos, x);
        }
    }

    // 4) Insert straggler
    if (hasStraggler)
    {
        std::list<int>::iterator pos = lowerBoundList(mainChain, straggler);
        mainChain.insert(pos, straggler);
    }

    l.swap(mainChain);
}

// ---------------- Sorted checks ----------------

bool PmergeMe::isSortedVector(const std::vector<int>& v)
{
    if (v.size() <= 1) return true;
    for (size_t i = 1; i < v.size(); ++i)
    {
        if (v[i - 1] > v[i]) return false;
    }
    return true;
}

bool PmergeMe::isSortedList(const std::list<int>& l)
{
    if (l.size() <= 1) return true;
    std::list<int>::const_iterator it = l.begin();
    std::list<int>::const_iterator prev = it;
    ++it;
    while (it != l.end())
    {
        if (*prev > *it) return false;
        prev = it;
        ++it;
    }
    return true;
}

