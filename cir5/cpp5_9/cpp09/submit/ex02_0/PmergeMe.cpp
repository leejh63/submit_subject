/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PmergeMe.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 18:34:57 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/06 18:34:58 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PmergeMe.hpp"

#include <iostream>

#include <stdexcept>
#include <cstdlib>
#include <cerrno>
#include <climits>

#include <set>
#include <algorithm>   // lower_bound
#include <sys/time.h>  // gettimeofday

// =========================
// ctor/dtor (private지만 정의는 필요)
// =========================
PmergeMe::PmergeMe() {}
PmergeMe::~PmergeMe() {}

// =========================
// timer
// =========================
static long long nowMicros()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
}

// =========================
// parsing / validation
// =========================
std::vector<int> PmergeMe::parseArgs(int argc, char** argv)
{
	if (argc < 2) throw std::runtime_error("Error");

	std::vector<int> out;
	out.reserve(argc - 1);

	for (int i = 1; i < argc; ++i) {
		const char* s = argv[i];
		if (s == 0 || *s == '\0') throw std::runtime_error("Error");

		char* end = 0;
		errno = 0;
		long v = std::strtol(s, &end, 10);

		if (*end != '\0') throw std::runtime_error("Error");
		if (errno != 0) throw std::runtime_error("Error");
		if (v <= 0) throw std::runtime_error("Error");
		if (v > INT_MAX) throw std::runtime_error("Error");

		out.push_back((int)v);
	}
	validateAndNoDup(out);
	return out;
}

void PmergeMe::validateAndNoDup(const std::vector<int>& v)
{
	std::set<int> seen;
	for (size_t i = 0; i < v.size(); ++i) {
		if (!seen.insert(v[i]).second) {
			throw std::runtime_error("Error");
		}
	}
}

// =========================
// printing
// =========================
void PmergeMe::printSequence(const char* label, const std::vector<size_t>& seq)
{
	std::cout << label;
	for (size_t i = 0; i < seq.size(); ++i) {
		std::cout << (i ? " " : " ") << seq[i];
	}
	std::cout << "\n";
}

void PmergeMe::printSequence(const char* label, const std::vector<int>& seq)
{
	std::cout << label;
	for (size_t i = 0; i < seq.size(); ++i) {
		std::cout << (i ? " " : " ") << seq[i];
	}
	std::cout << "\n";
}

// =========================
// Jacobsthal (vector order)
// =========================
// Jacobsthal: J(0)=0, J(1)=1, J(n)=J(n-1)+2*J(n-2)
// Ford-Johnson 삽입 순서(1-based):
// a1 먼저 삽입 후,
// k=3.. : j=J(k) <= m 이면 a_j, a_{j-1}, ..., a_{J(k-1)+1} 삽입
// 마지막으로 남은 것들 a_m.. 내려오며 삽입
std::vector<size_t> PmergeMe::buildJacobsthalInsertionOrder(size_t m)
{/*
	std::vector<size_t> order;
	if (m <= 1) return order;

	size_t Jkm2 = 0; // J(0)
	size_t Jkm1 = 1; // J(1)
	size_t Jk   = 1; // J(2)

	size_t prev = 1; // J(2)==1
	for (size_t k = 3; ; ++k) {
		Jk = Jkm1 + 2 * Jkm2; // J(k)
		if (Jk > m) {
			prev = Jkm1; // J(k-1)
			break;
		}
		for (size_t idx = Jk; idx > Jkm1; --idx) {
			if (idx >= 2 && idx <= m) order.push_back(idx);
		}
		Jkm2 = Jkm1;
		Jkm1 = Jk;
	}

	for (size_t idx = m; idx > prev; --idx) {
		if (idx >= 2 && idx <= m) {
			bool exists = false;
			for (size_t t = 0; t < order.size(); ++t) {
				if (order[t] == idx) { exists = true; break; }
			}
			if (!exists) order.push_back(idx);
		}
	}
	return order;
*/
	std::vector<size_t> jcob_sequance;
	if (m <= 1) { return jcob_sequance; }

	size_t jcob_n_2		= 0;
	size_t jcob_n_1		= 1;
	size_t jcob_n		= 1;
	
	for (;;)
	{
		jcob_n = jcob_n_1 + 2 * jcob_n_2;
		if (jcob_n >= m) { break; }
		for (size_t i = jcob_n; i > jcob_n_1; --i)
		{
			// 2 ~ jcob_n 
			if (i >= 2) { jcob_sequance.push_back(i - 1); }
		}
		jcob_n_2 = jcob_n_1;
		jcob_n_1 = jcob_n;

	}
	
	for (size_t i = m; i > jcob_n_1; --i) {
		jcob_sequance.push_back(i - 1);
	}
	return jcob_sequance;
}

// =========================
// vector Ford-Johnson
// =========================
void PmergeMe::binaryInsertVector(std::vector<int>& chain,
								  std::vector<int>::iterator endPos,
								  int value)
{
	std::vector<int>::iterator it = std::lower_bound(chain.begin(), endPos, value);
	chain.insert(it, value);
}

std::vector<int> PmergeMe::fordJohnsonSortVector(const std::vector<int>& input)
{
	printSequence("\ninput = ", input);
	const size_t n = input.size();
	if (n <= 1) return input;

	const bool hasStraggler = (n % 2) != 0;
	const int  straggler    = hasStraggler ? input[n - 1] : 0;

	const size_t pairCount = n / 2;

	// (big, small) 저장. big >= small
	std::vector< std::pair<int,int> > pairs;
	pairs.reserve(pairCount);

	std::vector<int> bigs;
	bigs.reserve(pairCount);

	for (size_t i = 0; i < pairCount; ++i) {
		int x = input[2 * i];
		int y = input[2 * i + 1];

		int small = (x < y) ? x : y;
		int big   = (x < y) ? y : x;

		pairs.push_back(std::make_pair(big, small));
		bigs.push_back(big);
	}

	// 1) big들 재귀로 FJ 정렬
	std::vector<int> sortedBigs = fordJohnsonSortVector(bigs);

	// 2) sortedBigs 순서에 맞춰 pending small 구성
	std::vector<int> pendingSmalls;
	pendingSmalls.reserve(pairCount);


	for (size_t i = 0; i < sortedBigs.size(); ++i) {
		int wantBig = sortedBigs[i];

		for (size_t p = 0; p < pairCount; ++p) {
			if (pairs[p].first == wantBig) {
				pendingSmalls.push_back(pairs[p].second);
				break;
			}
		}
	}

	// 3) 메인 체인 = 정렬된 big들
	std::vector<int> chain = sortedBigs;
	printSequence("\nchai22n = ", chain);
	printSequence("\npending = ", pendingSmalls);
	// 4) a1 먼저 삽입
	chain.insert( chain.begin(), pendingSmalls[0]);
	//binaryInsertVector(chain, chain.begin(), pendingSmalls[0]);

	// 5) Jacobsthal 스케줄
	const size_t N = pairCount + (hasStraggler ? 1 : 0);
	std::vector<size_t> order = buildJacobsthalInsertionOrder(N);
	printSequence("\njcob = ", order);
	std::cout << "\nstr = " << straggler << "\n";
	// 6) 스케줄대로 삽입
	for (size_t t = 0; t < order.size(); ++t) {
		size_t idx = order[t]; // 1-based

		if (idx == pairCount) {
			binaryInsertVector(chain, chain.end(), straggler);
		}
		else if (idx < pairCount) {
			const int smallVal = pendingSmalls[idx];
			const int bigVal   = sortedBigs[idx];

			std::vector<int>::iterator posB =
				std::lower_bound(chain.begin(), chain.end(), bigVal);

			binaryInsertVector(chain, posB, smallVal);
		}
	}
	printSequence("\nchai1n = ", chain);
	return chain;
}

// =========================
// list helpers (ALL STATIC MEMBERS, no namespace)
// =========================
std::list<int>::iterator
PmergeMe::lowerBoundListRange(std::list<int>::iterator beginIt,
							  std::list<int>::iterator endIt,
							  int value)
{
	std::list<int>::iterator it = beginIt;
	while (it != endIt && *it < value) {
		++it;
	}
	return it;
}

void PmergeMe::insertListBeforeBound(std::list<int>& chain,
									std::list<int>::iterator endPos,
									int value)
{
	std::list<int>::iterator pos = lowerBoundListRange(chain.begin(), endPos, value);
	chain.insert(pos, value);
}

bool PmergeMe::containsIndex(const std::list<size_t>& lst, size_t x)
{
	for (std::list<size_t>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
		if (*it == x) return true;
	}
	return false;
}

std::list< std::pair<int,int> >::iterator
PmergeMe::nthPendingIterator(std::list< std::pair<int,int> >& lst, size_t idx1)
{
	std::list< std::pair<int,int> >::iterator it = lst.begin();
	size_t cur = 1;
	while (it != lst.end() && cur < idx1) {
		++it;
		++cur;
	}
	return it;
}

std::list<int>::iterator
PmergeMe::boundByBigValue(std::list<int>& chain, int bigVal)
{
	std::list<int>::iterator it = chain.begin();
	while (it != chain.end() && *it < bigVal) {
		++it;
	}
	return it;
}

// Jacobsthal order (list version)
std::list<size_t> PmergeMe::buildJacobsthalInsertionOrderList(size_t m)
{
	std::list<size_t> order;
	if (m <= 1) return order;

	size_t Jkm2 = 0; // J(0)
	size_t Jkm1 = 1; // J(1)
	size_t Jk   = 1; // J(2)

	size_t prev = 1;

	for (size_t k = 3; ; ++k) {
		Jk = Jkm1 + 2 * Jkm2; // J(k)

		if (Jk > m) {
			prev = Jkm1; // J(k-1)
			break;
		}

		for (size_t idx = Jk; idx > Jkm1; --idx) {
			if (idx >= 2 && idx <= m) order.push_back(idx);
		}

		Jkm2 = Jkm1;
		Jkm1 = Jk;
	}

	for (size_t idx = m; idx > prev; --idx) {
		if (idx >= 2 && idx <= m && !containsIndex(order, idx)) {
			order.push_back(idx);
		}
	}

	return order;
}

// =========================
// list Ford-Johnson (NO vector/map usage inside)
// =========================
std::list<int> PmergeMe::fordJohnsonSortList(const std::list<int>& input)
{
	const size_t n = input.size();
	if (n <= 1) return input;

	const bool hasStraggler = (n % 2) != 0;
	int straggler = 0;

	if (hasStraggler) {
		std::list<int>::const_iterator last = input.end();
		--last;
		straggler = *last;
	}

	const size_t pairCount = n / 2;

	// 1) pairs (big, small), bigs
	std::list< std::pair<int,int> > pairs;
	std::list<int> bigs;

	std::list<int>::const_iterator it = input.begin();
	for (size_t i = 0; i < pairCount; ++i) {
		int x = *it; ++it;
		int y = *it; ++it;

		int small = (x < y) ? x : y;
		int big   = (x < y) ? y : x;

		pairs.push_back(std::make_pair(big, small));
		bigs.push_back(big);
	}

	// 2) sort bigs recursively
	std::list<int> sortedBigs = fordJohnsonSortList(bigs);

	// 3) pendingPairs in sortedBigs order
	std::list< std::pair<int,int> > pendingPairs;
	for (std::list<int>::const_iterator sb = sortedBigs.begin();
		 sb != sortedBigs.end(); ++sb)
	{
		int wantBig = *sb;
		bool found = false;

		for (std::list< std::pair<int,int> >::iterator p = pairs.begin();
			 p != pairs.end(); ++p)
		{
			if (p->first == wantBig) {
				pendingPairs.push_back(*p);
				pairs.erase(p);
				found = true;
				break;
			}
		}

		if (!found) throw std::runtime_error("internal error");
	}

	// 4) chain = sorted bigs
	std::list<int> chain = sortedBigs;

	// 5) insert a1 before b1
	if (pendingPairs.empty()) throw std::runtime_error("internal error");
	insertListBeforeBound(chain, chain.begin(), pendingPairs.begin()->second);

	// 6) Jacobsthal schedule on N pending items
	const size_t N = pairCount + (hasStraggler ? 1 : 0);
	std::list<size_t> order = buildJacobsthalInsertionOrderList(N);

	// 7) insert by schedule
	for (std::list<size_t>::const_iterator oi = order.begin();
		 oi != order.end(); ++oi)
	{
		size_t idx = *oi; // 1-based

		// straggler index = pairCount+1
		if (hasStraggler && idx == pairCount + 1) {
			std::list<int>::iterator pos =
				lowerBoundListRange(chain.begin(), chain.end(), straggler);
			chain.insert(pos, straggler);
			continue;
		}

		if (idx >= 2 && idx <= pairCount) {
			std::list< std::pair<int,int> >::iterator pendIt =
				nthPendingIterator(pendingPairs, idx);

			if (pendIt == pendingPairs.end()) throw std::runtime_error("internal error");

			int bigVal   = pendIt->first;
			int smallVal = pendIt->second;

			std::list<int>::iterator posB = boundByBigValue(chain, bigVal);
			if (posB == chain.end()) throw std::runtime_error("internal error");

			insertListBeforeBound(chain, posB, smallVal);
		}
	}

	return chain;
}

// =========================
// timing wrappers
// =========================
double PmergeMe::timeSortVector(const std::vector<int>& input, std::vector<int>& outSorted)
{
	long long t0 = nowMicros();
	outSorted = fordJohnsonSortVector(input);
	long long t1 = nowMicros();
	return (double)(t1 - t0);
}

double PmergeMe::timeSortList(const std::list<int>& input, std::list<int>& outSorted)
{
	long long t0 = nowMicros();
	outSorted = fordJohnsonSortList(input);
	long long t1 = nowMicros();
	return (double)(t1 - t0);
}























