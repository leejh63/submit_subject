/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HumanB.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 18:36:23 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 18:36:25 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HUMANB_HPP
# define HUMANB_HPP
# include "Weapon.hpp"

class HumanB {
private:
	std::string name;
	Weapon* wp;
public:
	HumanB(const std::string name, Weapon& wp);
	HumanB(const std::string name);
	~HumanB( void );
	
	void attack( void ) const ;
	void setWeapon( Weapon& newwp );
};

#endif
