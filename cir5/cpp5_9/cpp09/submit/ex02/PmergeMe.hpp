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

#include <list>
#include <vector>

#include <sys/time.h>

class PmergeMe {
public:
	static void						FJA_vector( int argc, char* argv[] );
	static void                    	FJA_list( int argc, char* argv[] );
	
private:
	// time check
	static double					time_check( long long start, long long end );
	static long long				now_time( void );
	
	//==================================vertor=====================================//
	static void						print_vector( const std::vector<int>& vec );
	
	static void						print_result_vector( const std::vector<int>& input_vector,
											  		 	 const std::vector<int>& sort_vector,
											  		 	 double elapsed_time );
	
	static void						binanry_insert_vector( std::vector<int>& main_chain,
														   std::vector<int>::iterator big_value_it,
														   int insert_value );

	static void						is_sorted_vector( const std::vector<int>& input_vector,
													  const std::vector<int>& sort_vector );

	static std::vector<int>			parse_args_vector( int argc, char* argv[] );
	static std::vector<int>			FJ_sort_vector( const std::vector<int>& input_vector );
	static std::vector<std::size_t>	jacob_seq_vector( std::size_t pair_size );
	//=================================vertor_end==================================//
	
	//==================================list=====================================//
    static void                    	print_list( const std::list<int>& lst );

    static void                    	print_result_list( const std::list<int>& input_list,
                                					   const std::list<int>& sort_list,
                                					   double elapsed_time );

    static void                    	binary_insert_list( std::list<int>& main_chain,
                                						std::list<int>::iterator big_value_it,
                                						int insert_value );
                                						
    static void                    	is_sorted_list( const std::list<int>& input_list,
                                					const std::list<int>& sort_list );
                                					
    static std::list<int>          	parse_args_list( int argc, char* argv[] );
    static std::list<int>          	FJ_sort_list( const std::list<int>& input_list );
    static std::list<std::size_t>   jacob_seq_list( std::size_t pair_size );
	//=================================list_end==================================//


// 생성자 등등 미구현
	PmergeMe( void );
	PmergeMe( const PmergeMe& copy );
	PmergeMe& operator=( const PmergeMe& copy );
	~PmergeMe( void );
};

#endif































