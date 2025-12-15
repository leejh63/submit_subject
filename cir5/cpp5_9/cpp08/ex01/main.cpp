/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/13 16:19:57 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/13 16:19:58 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Span.hpp"
#include <iostream>
#include <vector>
#include <limits>
#include <cstdlib>
#include <ctime>

static void sep(const char* title) {
    std::cout << "\n==============================\n";
    std::cout << title << "\n";
    std::cout << "==============================\n";
}

static void run(Span& sp) {
    std::cout << "실제 shortestSpan = " << sp.shortestSpan() << "\n";
    std::cout << "실제 longestSpan  = " << sp.longestSpan()  << "\n";
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(NULL)));
    
	try {
	    sep("CASE 0: 과제 예제");
		Span sp = Span(5);
		sp.addNumber(6);
		sp.addNumber(3);
		sp.addNumber(17);
		sp.addNumber(9);
		sp.addNumber(11);
		std::cout << sp.shortestSpan() << std::endl;
		std::cout << sp.longestSpan() << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 0: 과제 예제");
        std::cout << "기대값: shortest = 2, longest = 14\n";
        Span sp(5);
        sp.addNumber(6);
        sp.addNumber(3);
        sp.addNumber(17);
        sp.addNumber(9);
        sp.addNumber(11);
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 1: 빈 컨테이너");
        std::cout << "기대값: 예외 발생\n";
        Span sp(5);
        run(sp);
    } catch (const std::exception& e) {
        std::cout << "예외 메시지: " << e.what() << "\n";
    }

    try {
        sep("CASE 2: 원소 1개");
        std::cout << "기대값: 예외 발생\n";
        Span sp(5);
        sp.addNumber(42);
        run(sp);
    } catch (const std::exception& e) {
        std::cout << "예외 메시지: " << e.what() << "\n";
    }

    try {
        sep("CASE 3: 용량 초과");
        std::cout << "기대값: 예외 발생 (addNumber)\n";
        Span sp(2);
        sp.addNumber(1);
        sp.addNumber(2);
        sp.addNumber(3); // 여기서 예외
    } catch (const std::exception& e) {
        std::cout << "예외 메시지: " << e.what() << "\n";
    }

    try {
        sep("CASE 4: 중복 값만 존재");
        std::cout << "기대값: shortest = 0, longest = 0\n";
        Span sp(5);
        sp.addNumber(9);
        sp.addNumber(9);
        sp.addNumber(9);
        sp.addNumber(9);
        sp.addNumber(9);
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 5: 음수와 양수 혼합");
        std::cout << "기대값: shortest = 2, longest = 30\n";
        Span sp(6);
        sp.addNumber(-10);
        sp.addNumber(-3);
        sp.addNumber(0);
        sp.addNumber(5);
        sp.addNumber(7);
        sp.addNumber(20);
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 6: 내림차순으로 입력");
        std::cout << "기대값: shortest = 10, longest = 100\n";
        Span sp(5);
        sp.addNumber(100);
        sp.addNumber(50);
        sp.addNumber(20);
        sp.addNumber(10);
        sp.addNumber(0);
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 7: INT_MIN / INT_MAX");
        std::cout << "기대값: shortest = 2147483648, longest = 4294967295\n";
        Span sp(3);
        sp.addNumber(std::numeric_limits<int>::min());
        sp.addNumber(0);
        sp.addNumber(std::numeric_limits<int>::max());
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 8: 범위 삽입 (정확한 용량)");
        std::cout << "기대값: shortest = 1, longest = 8\n";
        std::vector<int> v;
        v.push_back(8);
        v.push_back(1);
        v.push_back(3);
        v.push_back(7);
        v.push_back(9);

        Span sp(5);
        sp.addNumber(v.begin(), v.end());
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 9: 범위 삽입 용량 초과");
        std::cout << "기대값: 예외 발생\n";
        std::vector<int> v(10, 42);
        Span sp(9);
        sp.addNumber(v.begin(), v.end());
    } catch (const std::exception& e) {
        std::cout << "예외 메시지: " << e.what() << "\n";
    }

    try {
        sep("CASE 10: 10,000개 랜덤 데이터");
        std::cout << "기대값: 예외 없이 결과 출력\n";
        const int N = 10000;
        Span sp(N);
        for (int i = 0; i < N; ++i) {
            int x = std::rand();
            if (std::rand() % 2) x = -x;
            sp.addNumber(x);
        }
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 11: 연속된 값");
        std::cout << "기대값: shortest = 1, longest = 400\n";
        Span sp(6);
        sp.addNumber(100);
        sp.addNumber(101);
        sp.addNumber(200);
        sp.addNumber(300);
        sp.addNumber(400);
        sp.addNumber(500);
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    try {
        sep("CASE 12: 중복으로 shortestSpan = 0");
        std::cout << "기대값: shortest = 0, longest = 200\n";
        Span sp(6);
        sp.addNumber(5);
        sp.addNumber(1);
        sp.addNumber(9);
        sp.addNumber(5);
        sp.addNumber(100);
        sp.addNumber(-100);
        run(sp);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    return 0;
}


