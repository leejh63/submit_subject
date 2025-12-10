/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 19:42:08 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/13 19:42:10 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <exception>
#include <list>

/* 인덱스 비트마스킹 기법!
AForm* (*fn_[5])( const std::string& target ) = { &m_null, &m_tree, &m_robo, NULL, &m_pard };
AForm* Intern::makeForm( const std::string& paper, const std::string& target ) {
	util_::log("makeForm");
	AForm* test = fn_[(int)(paper == "shrubbery creation")
					+ ((int)(paper == "robotomy request") << 1)
					+ ((int)(paper == "presidential pardon") << 2)](target);
	return (test) ? test : throw NoFormException(paper);
}
*/


class MyException : public std::exception {
public:
    const char* what() const throw() {
        return "MyException occurred!";
    }
};

int main() {
	std::string test_t[3] = {"1", "2", "3"};
	std::list<int> ilst = {1, 2, 3, 4};
    try {
        // new로 예외 객체 생성
        //throw new MyException();
    	//throw 150;
    	/*
    	char* test = new char[15];
    	for (int i = 0; i < 14; i++) {
    		test[i] = '1';
    	}
    	test[14] = '\0';
    	throw test;
    	const char text[] = "wowowowowo";
    	throw text;
    	*/
    	//throw test_t;
    	//throw ilst;
    	throw 1;
    }
    catch (MyException* e) {
        std::cout << "[Pointer Exception caught] " << e->what() << std::endl;
        delete e; // new 했으면 반드시 delete
    }
    catch (int i) {
    	std::cout << "[int Exception caught] " << i << std::endl;
    }
    catch (char* test) {
    	std::cout << "[char* Exception caught] " << test << std::endl;
    	//delete[] test;
    }
    catch (const char* test2) {
    	std::cout << "[const char* Exception caught] " << test2 << std::endl;
    }
    catch (std::string testt[]) {
    	std::cout << "[string Exception caught] " << testt[1] << std::endl;
    }
    catch (std::list<int> lst) {
    	std::cout << "[std::list<int>& {";
    	for(std::list<int>::iterator it = lst.begin(); it!=lst.end(); it++)
	    	std::cout << *it << " ";
    	std::cout << "} caught]"<< std::endl;
    }

    return 0;
}
