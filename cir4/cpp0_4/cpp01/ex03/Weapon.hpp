/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Weapon.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/14 18:36:02 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/14 18:36:03 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEAPON_HPP
# define WEAPON_HPP
# include <string>
# include <iostream>

class Weapon {

private :
	std::string type;

public :
	Weapon( const std::string type );
	Weapon( const Weapon& wp );
	~Weapon( void );
	
	const std::string& getType( void ) const ;
	void setType( const std::string& newtype );
};

#endif
