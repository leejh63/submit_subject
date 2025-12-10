/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HumanA.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 18:36:15 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 18:36:16 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HUMANA_HPP
# define HUMANA_HPP
# include "Weapon.hpp"

class HumanA {
private:
	std::string name;
	Weapon& wp;

public:
	HumanA( const std::string name, Weapon& wp );
	~HumanA( void );

	void attack( void ) const ;
	void setWeapon( const std::string newwp );
};

#endif
