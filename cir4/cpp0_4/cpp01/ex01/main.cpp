/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 11:49:19 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 11:49:22 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Zombie.hpp"
#include <iostream>

int main( void ) {
	
{
	std::cout << "Class Zombie made by constructor\n";
	Zombie zconstor("constructor");
}
	std::cout << "\nClass Zombie made by newZombie\n";
	Zombie* zptr = newZombie("newZombie");
	
	std::cout << "\nClass Zombie made by randomChump\n";
	randomChump("randomChump");
	
	delete zptr;
	
	std::cout << "\nClass Zombie made by zombieHorde\n";
	int znum = 7;
	Zombie* zhptr = zombieHorde( znum, "zhorde0-" );
	for (int i = 0; i < znum; ++i) {
		zhptr[i].announce();
	}
	delete[] zhptr;
	return 0;
}
