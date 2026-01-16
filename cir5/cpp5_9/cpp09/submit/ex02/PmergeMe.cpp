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

#include <cerrno>
#include <climits>
#include <iomanip>

/*/
#include <cstdlib>
#include <cerrno>
#include <climits>
/*/
#include <sstream>
//*/

#include <utility>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <sys/time.h>



namespace
{
	const char* e_word( int num )
	{
		const char* e_corpus[] =
		{
			"Usage: ./PmergeMe <positive integers>",
			"Invalid arguments",
			"Number must be between 1 and INT_MAX",
			"Numbers must be unique",
			"Not Sorted",
		};
		return e_corpus[num];
	}
}

// time check
double
PmergeMe::time_check( long long start, long long end ) { return static_cast<double>(end - start); }

long long
PmergeMe::now_time( void )
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return static_cast<double>(tv.tv_sec) * 1000000LL + static_cast<double>(tv.tv_usec);
}

// vertor //
void
PmergeMe::print_vector( const std::vector<int>& vec )
{
	//if (1 == 1) return;
	for (std::vector<int>::const_iterator it = vec.begin(); it != vec.end(); ++it)
	{
		std::cout << *it;
		if (it + 1 != vec.end()) { std::cout << " "; }
	}
	std::cout << std::endl;
}
// list //
void
PmergeMe::print_list(const std::list<int>& lst)
{
    for (std::list<int>::const_iterator it = lst.begin(); it != lst.end(); ++it)
    {
        std::cout << *it;
        std::list<int>::const_iterator next = it;
        ++next;
        if (next != lst.end()) { std::cout << " "; }
    }
    std::cout << std::endl;
}


// vector //
std::vector<int>
PmergeMe::parse_args_vector(int argc, char* argv[])
{
    if (argc < 2)
        throw std::runtime_error(e_word(0));

    std::vector<int> input_vector;
    input_vector.reserve(argc - 1);

    for (int i = 1; i < argc; ++i) {
        const char* arg_value = argv[i];
        if (arg_value == 0 || *arg_value == '\0')
            throw std::runtime_error(e_word(1));

        std::istringstream iss(arg_value);

        int string_to_int;

        // 정수 파싱
        if (!(iss >> string_to_int))
            throw std::runtime_error(e_word(1));

        // 공백 제외 다른 문자가 남아있으면 에러
        iss >> std::ws;
        if (!iss.eof())
            throw std::runtime_error(e_word(1));

        // 양수 조건
        if (string_to_int <= 0)
            throw std::runtime_error(e_word(1));

        // 중복 검사
        if (std::find(input_vector.begin(), input_vector.end(), string_to_int) != input_vector.end())
            throw std::runtime_error(e_word(3));

        input_vector.push_back(string_to_int);
    }

    return input_vector;
}
// list //
std::list<int>
PmergeMe::parse_args_list(int argc, char* argv[])
{
    if (argc < 2)
        throw std::runtime_error(e_word(0));

    std::list<int> input_list;

    for (int i = 1; i < argc; ++i)
    {
        const char* arg_value = argv[i];
        if (arg_value == 0 || *arg_value == '\0')
            throw std::runtime_error(e_word(1));

        std::istringstream iss(arg_value);
        int value;

		 // 정수 파싱
        if (!(iss >> value))
            throw std::runtime_error(e_word(1));

		// 공백 제외 다른 문자가 남아있으면 에러
        iss >> std::ws;
        if (!iss.eof())
            throw std::runtime_error(e_word(1));

		// 양수 조건
        if (value <= 0)
            throw std::runtime_error(e_word(1));

		// 중복 검사
        if (std::find(input_list.begin(), input_list.end(), value) != input_list.end())
            throw std::runtime_error(e_word(3));

        input_list.push_back(value);
    }
    return input_list;
}

// vector //
void
PmergeMe::binanry_insert_vector( std::vector<int>& main_chain,
								 std::vector<int>::iterator big_value_it,
								 int insert_value )
{
	std::vector<int>::iterator insert_pos = 
		std::lower_bound(main_chain.begin(), big_value_it, insert_value);
	main_chain.insert(insert_pos, insert_value);
}
// list //
void
PmergeMe::binary_insert_list(std::list<int>& main_chain,
                             std::list<int>::iterator big_value_it,
                             int insert_value)
{
    std::list<int>::iterator insert_pos =
        std::lower_bound(main_chain.begin(), big_value_it, insert_value);
    main_chain.insert(insert_pos, insert_value);
}



// 삽입 순서
// 1 / 3 2 / 5 4 / 11 10 9 8 7 6 / 21 20 ...
// (1의 경우 이미 삽입을 진행하기에 사실상 필요없음)
// 1, 3, 5, 11, 21 .... << 야콥스탈 수열 / 수열 번호도 존재함
// Jacobsthal 공식: J(n)=J(n-1)+2*J(n-2) // J(0)=0, J(1)=1, J(2)=1
// 1을 기준으로 한다. 저장때 인덱스로 변경해서 저장
// vector //
std::vector<size_t>
PmergeMe::jacob_seq_vector( size_t pair_size )
{
	std::vector<size_t> jcob_sequance;
	if (pair_size <= 1) { return jcob_sequance; }

	size_t jcob_n_2		= 0;
	size_t jcob_n_1		= 1;
	size_t jcob_n		= 1;
	
	for (;;)
	{
		jcob_n = jcob_n_1 + 2 * jcob_n_2;
		if (jcob_n >= pair_size) { break; }
		for (size_t i = jcob_n; i > jcob_n_1; --i)
		{
			// 2 ~ jcob_n 
			if (i >= 2) { jcob_sequance.push_back(i - 1); }
		}
		jcob_n_2 = jcob_n_1;
		jcob_n_1 = jcob_n;

	}
	
	for (size_t i = pair_size; i > jcob_n_1; --i) {
		jcob_sequance.push_back(i - 1);
	}
	
	return jcob_sequance;
}
// list //
std::list<size_t>
PmergeMe::jacob_seq_list(size_t pair_size)
{
    std::list<size_t> jcob_sequance;
    if (pair_size <= 1)
        return jcob_sequance;

    size_t jcob_n_2 = 0; // J(n-2)
    size_t jcob_n_1 = 1; // J(n-1)
    size_t jcob_n   = 1; // J(n)

    for (;;)
    {
        jcob_n = jcob_n_1 + 2 * jcob_n_2;
        if (jcob_n >= pair_size)
            break;

        for (size_t i = jcob_n; i > jcob_n_1; --i)
        {
            if (i >= 2)
                jcob_sequance.push_back(i - 1);
        }
        jcob_n_2 = jcob_n_1;
        jcob_n_1 = jcob_n;
    }

    for (size_t i = pair_size; i > jcob_n_1; --i)
        jcob_sequance.push_back(i - 1);

    return jcob_sequance;
}


std::vector<int>
PmergeMe::FJ_sort_vector( const std::vector<int>& input_vector )
{
	const size_t n = input_vector.size();
	if (n <= 1) return input_vector;

	const int		odd_val		= (n % 2) ? input_vector[n - 1] : 0;
	const size_t	pairing_size	= n / 2;
	
	std::vector< std::pair<int,int> > pair_s;
	pair_s.reserve(pairing_size);
	
	std::vector<int> big_s;
	big_s.reserve(pairing_size);
	/*/
	std::vector<int>::const_iterator it = input_vector.begin();
	for (size_t i = 0; i < pairing_size; ++i)
	{
		int a = *it++;
		int b = *it++;

		int small = (a < b) ? a : b;
		int big   = (a > b) ? a : b;

		pair_s.push_back(std::make_pair(big, small));
		big_s.push_back(big);
	}
	/*/
	for (size_t i = 0; i + 1 < input_vector.size(); i += 2)
	{
		int cmp_left = input_vector[i];
		int cmp_right = input_vector[i + 1];
		
		int small = (cmp_left < cmp_right) ? cmp_left : cmp_right;
		int big = (cmp_left > cmp_right) ? cmp_left : cmp_right;
		
		pair_s.push_back(std::make_pair(big, small));
		big_s.push_back(big);
	}
	//*/
	std::vector<int> sort_big = FJ_sort_vector(big_s);
	
	// 정렬된 big 들에 맞춰서 small 세팅 진행
	// small_s를 만드는 것보다 pair_s를 big 에 맞춰서 정렬하는게 좀더 좋을 수 도?
	std::vector<int> small_s;
	small_s.reserve(pairing_size);
	
	for (size_t i = 0; i < sort_big.size(); ++i) 
	{
		int find_big = sort_big[i];
		for (size_t ii = 0; ii < pair_s.size(); ++ii)
		{
			if (pair_s[ii].first == find_big)
			{
				small_s.push_back(pair_s[ii].second);
				break;
			}
		}
	}

	std::vector<int> main_chain = sort_big;
	// 어짜피 모든 숫자 중 small_s[0]의 값이 최솟값
	main_chain.insert(main_chain.begin(), small_s[0]);
	
	const size_t j_seq_n = pairing_size + ((odd_val) ? 1 : 0);
	std::vector<size_t> j_seq_vector = jacob_seq_vector(j_seq_n);
	
	for (size_t i = 0; i < j_seq_vector.size(); ++i)
	{
		size_t j_seq_ind = j_seq_vector[i];
		if (j_seq_ind == pairing_size)
		{
			binanry_insert_vector(main_chain, main_chain.end(), odd_val);
		}
		else if (j_seq_ind < pairing_size)
		{// 여기서 pair_s를 정렬한다면 좀더 쉽게 갈 수 도? 
			const int small_value = small_s[j_seq_ind];
			const int big_key = sort_big[j_seq_ind];
			
			std::vector<int>::iterator pos_bigkey = 
					std::lower_bound(main_chain.begin(), main_chain.end(), big_key);

			binanry_insert_vector(main_chain, pos_bigkey, small_value);
		}
	}
	return main_chain;
}
// list //
std::list<int>
PmergeMe::FJ_sort_list(const std::list<int>& input_list)
{
    const size_t n = input_list.size();
    if (n <= 1)
        return input_list;

    std::list<int>::const_iterator list_start_it = input_list.begin();

	std::list<int>::const_iterator list_last_it = input_list.end();
	--list_last_it;
	const int odd_val = (n % 2) ? *list_last_it : 0;
    const size_t pairing_size = n / 2;

    std::list< std::pair<int,int> > pair_s;
    std::list<int> big_s;

    for (size_t i = 0; i < pairing_size; ++i)
    {
        int a = *list_start_it++; 
        int b = *list_start_it++;

        int small = (a < b) ? a : b;
        int big   = (a > b) ? a : b;

        pair_s.push_back(std::make_pair(big, small));
        big_s.push_back(big);
    }

    std::list<int> sort_big = FJ_sort_list(big_s);

    std::list<int> small_s;
    for (std::list<int>::const_iterator i = sort_big.begin(); i != sort_big.end(); ++i)
    {
        for (std::list< std::pair<int,int> >::const_iterator ii = pair_s.begin(); ii != pair_s.end(); ++ii)
        {
            if (ii->first == *i)
            {
                small_s.push_back(ii->second);
                break;
            }
        }
    }

    std::list<int> main_chain = sort_big;
    main_chain.push_front(*small_s.begin());
    
    /*/ list 장점 활용: main_chain 안의 "각 big 원소 위치(iterator)"를 미리 저장
    // (이후 big 위치를 lower_bound로 다시 찾지 않고, 곧바로 bound로 사용)
    std::list< std::list<int>::iterator > big_pos_s;
    {
        std::list<int>::iterator mc_it = main_chain.begin();
        ++mc_it; // push_front된 첫 small은 제외하고, 나머지는 sort_big(= big)들
        for (; mc_it != main_chain.end(); ++mc_it)
            big_pos_s.push_back(mc_it);
    }
    //*/
    
	const size_t j_seq_n = pairing_size + ((odd_val) ? 1 : 0);
	std::list<size_t> j_seq = jacob_seq_list(j_seq_n);

	for (std::list<size_t>::const_iterator j_seq_it = j_seq.begin(); j_seq_it != j_seq.end(); ++j_seq_it)
	{
		size_t j_seq_ind = *j_seq_it;

		if (j_seq_ind == pairing_size)
		{
		    binary_insert_list(main_chain, main_chain.end(), odd_val);
		}
		else if (j_seq_ind < pairing_size)
		{
		    std::list<int>::iterator small_it = small_s.begin();
		    std::advance(small_it, j_seq_ind);

		    /*/ big_value_it 을 "검색"하지 않고, 미리 저장한 iterator를 그대로 사용
            std::list< std::list<int>::iterator >::iterator bpit = big_pos_s.begin();
            std::advance(bpit, idx);

            // 제한 구간 [begin, big_value_it) 에서 삽입
            binary_insert_list(main_chain, *bpit, *small_it);
			/*/
			std::list<int>::iterator big_it = sort_big.begin();
		    std::advance(big_it, j_seq_ind);

		    std::list<int>::iterator pos_big =
		        std::lower_bound(main_chain.begin(), main_chain.end(), *big_it);

		    binary_insert_list(main_chain, pos_big, *small_it);
			//*/
		}
	}

    return main_chain;
}


// 함수인자를 이터레이터로 안보낸이유, 혹시나중에 추가로 필요하지않을까? 생각
// vector //
void
PmergeMe::is_sorted_vector( const std::vector<int>& input_vector, const std::vector<int>& sort_vector )
{
    std::vector<int> sorting(input_vector);
    std::sort(sorting.begin(), sorting.end());

    if (sorting != sort_vector)
        throw std::runtime_error(e_word(4));
}
// list //
void
PmergeMe::is_sorted_list(const std::list<int>& input_list, const std::list<int>& sort_list)
{
	std::list<int> sorting = input_list;
	sorting.sort();

	if (!std::equal(sorting.begin(), sorting.end(), sort_list.begin()))
		throw std::runtime_error(e_word(4));
}


// vector //
void
PmergeMe::print_result_vector( const std::vector<int>& input_vector,
							   const std::vector<int>& sort_vector,
							   double elapsed_time )
{
	std::cout << "\n=====================Vector=========================\nBefore: ";
	print_vector(input_vector);
	std::cout << "\nAfter:  ";
	print_vector(sort_vector);
	std::cout << "\nTime to process a range of "
			  << sort_vector.size()
			  << " elements with std::vector : "
			  << std::fixed << std::setprecision(6)
			  << elapsed_time << " us\n";
}
// list //
void
PmergeMe::print_result_list(const std::list<int>& input_list,
                            const std::list<int>& sort_list,
                            double elapsed_time)
{
    std::cout << "\n======================list==========================\nBefore: ";
    print_list(input_list);
    std::cout << "\nAfter:  ";
    print_list(sort_list);
    std::cout << "\nTime to process a range of "
              << sort_list.size()
              << " elements with std::list : "
              << std::fixed << std::setprecision(6)
              << elapsed_time << " us\n";
}

// vector //
void
PmergeMe::FJA_vector( int argc, char* argv[] )
{
	long long start = now_time();
	
	// 인자 검증
	std::vector<int> input_vector = parse_args_vector(argc, argv);
	
	// 정렬
	std::vector<int> sort_vector = FJ_sort_vector(input_vector);

	// 정렬 검증
	is_sorted_vector(input_vector, sort_vector);
	
	long long end = now_time();
	
	double elapsed_time = time_check(start, end);
	
	// 출력
	print_result_vector(input_vector, sort_vector, elapsed_time);
}
// list //
void
PmergeMe::FJA_list(int argc, char* argv[])
{
    long long start = now_time();

    std::list<int> input_list = parse_args_list(argc, argv);
    
    std::list<int> sort_list  = FJ_sort_list(input_list);

    is_sorted_list(input_list, sort_list);

    long long end = now_time();
    
    double elapsed_time = time_check(start, end);

    print_result_list(input_list, sort_list, elapsed_time);
}






















