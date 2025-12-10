/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 18:31:19 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/11/05 18:31:20 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Intern.hpp"
#include "Bureaucrat.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"

int main() {
	try {
		std::cout << "=== Intern Form Creation Test ===\n";

		Intern intern;
		Bureaucrat boss("Boss", 1);
		// 정상 form 생성 테스트
		std::cout << "\n--- Creating Shrubbery Form ---\n";
		AForm* f1 = intern.makeForm("shrubbery creation", "Home");
		if (f1) {
			boss.signForm(*f1);
		    boss.executeForm(*f1);
		    std::cout << *f1 << std::endl;
		    delete f1;
		}

		std::cout << "\n--- Creating Robotomy Form ---\n";
		AForm* f2 = intern.makeForm("robotomy request", "Bender");
		if (f2) {
		    boss.signForm(*f2);
		    boss.executeForm(*f2);
		    std::cout << *f2 << std::endl;
		    delete f2;
		}

		std::cout << "\n--- Creating Pardon Form ---\n";
		AForm* f3 = intern.makeForm("presidential pardon", "Ford");
		if (f3) {
		    boss.signForm(*f3);
		    boss.executeForm(*f3);
		    std::cout << *f3 << std::endl;
		    delete f3;
		}

		// 실제 sign/execute 흐름 테스트
		std::cout << "\n=== Execution Test ===\n";

		
		AForm* robo = intern.makeForm("robotomy request", "Marvin");

		if (robo) {
		    boss.signForm(*robo);
		    boss.executeForm(*robo);
		    delete robo;
		}

		// 존재하지 않는 form 테스트
		std::cout << "\n--- Creating Unknown Form ---\n";
		AForm* f4 = intern.makeForm("coffee form", "Nobody");
		if (f4)
		    delete f4;
	}
	catch (std::exception &e) {
		std::cerr << "Exception: " << e.what();
	}
    return 0;
}
















