/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Zombie.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 11:50:22 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 11:50:24 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ZOMBIE_HPP
# define ZOMBIE_HPP
# include <string>


class Zombie {

private	:
	std::string name;

public	:
	Zombie( const std::string& name );
	~Zombie( void );
	
	void announce( void );
};

Zombie* newZombie( std::string name );
void randomChump( std::string name );

#endif
























