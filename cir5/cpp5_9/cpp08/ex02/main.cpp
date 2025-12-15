/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 13:36:53 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/14 13:37:03 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <stack>
#include <vector>
#include <list>
#include <algorithm>

#include "MutantStack.hpp"

template <typename MS>
void print_iter(MS& ms) {
    typename MS::iterator it = ms.begin();
    typename MS::iterator ite = ms.end();

    if (it != ite) { ++it; --it; }

    while (it != ite) {
        std::cout << *it << std::endl;
        ++it;
    }
}

template <typename MS>
void print_const_iter(const MS& ms) {
    typename MS::const_iterator it = ms.begin();
    typename MS::const_iterator ite = ms.end();

    while (it != ite) {
        std::cout << *it << std::endl;
        ++it;
    }
}

/*
template <typename MS>
void print_stack_iter(MS& ms) {
    typename MS::riterator it = ms.rbegin();
    typename MS::riterator ite = ms.rend();

    while (it != ite) {
        std::cout << *it << std::endl;
        ++it;
    }
}
*/

template <typename MS>
void algo_test(MS& ms) {
    typename MS::iterator it = std::find(ms.begin(), ms.end(), 737);

    if (it != ms.end())
        std::cout << "[find] 737 found: " << *it << std::endl;
    else
        std::cout << "[find] 737 not found" << std::endl;
}

template <typename MS>
void compare_with_list(MS& ms) {
    std::cout << "\n========== compare MutantStack vs std::list ==========\n";

    std::list<int> lst;

    // 동일한 데이터, 동일한 순서
    int data[] = {5, 17, 3, 5, 737, 0};
    int n = sizeof(data) / sizeof(data[0]);

    for (int i = 0; i < n; ++i) {
        ms.push(data[i]);
        lst.push_back(data[i]);
    }

    // MutantStack 출력
    std::cout << "[MutantStack output]\n";
    {
        typename MS::iterator it = ms.begin();
        typename MS::iterator ite = ms.end();
        while (it != ite) {
            std::cout << *it << std::endl;
            ++it;
        }
    }

    // std::list 출력
    std::cout << "[std::list output]\n";
    {
        std::list<int>::iterator it = lst.begin();
        std::list<int>::iterator ite = lst.end();
        while (it != ite) {
            std::cout << *it << std::endl;
            ++it;
        }
    }
}

template <typename MS>
void run_test(const char* name) {
    std::cout << "\n========== " << name << " ==========\n";

    MS mstack;

    std::cout << "[stack] push 5, 17\n";
    mstack.push(5);
    mstack.push(17);

    std::cout << "[stack] top: " << mstack.top() << std::endl;
    mstack.pop();
    std::cout << "[stack] size after pop: " << mstack.size() << std::endl;

    std::cout << "[stack] push 3, 5, 737, 0\n";
    mstack.push(3);
    mstack.push(5);
    mstack.push(737);
    mstack.push(0);

    std::cout << "[iter] non-const\n";
    print_iter(mstack);
    /*
    std::cout << "[iter] stack_iter\n";
    print_stack_iter(mstack);
	*/
    std::cout << "[iter] const\n";
    
    const MS cmstack(mstack);
    print_const_iter(cmstack);

    std::cout << "[algo]\n";
    algo_test(mstack);

    std::stack<int, typename MS::container_type> s(mstack);
    (void)s;

    mstack.pop();
    mstack.push(123);
    std::cout << "[stack] top after modify: " << mstack.top() << std::endl;
    for (;!mstack.empty();) { mstack.pop(); };
    compare_with_list(mstack);
}



int main() {
    run_test< MutantStack<int> >("MutantStack<int> (default)");
    run_test< MutantStack<int, std::vector<int> > >("MutantStack<int, vector>");
    run_test< MutantStack<int, std::list<int> > >("MutantStack<int, list>");

    return 0;
}





