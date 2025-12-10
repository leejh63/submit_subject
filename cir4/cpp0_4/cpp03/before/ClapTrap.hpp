/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClapTrap.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/18 12:56:09 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/18 13:12:39 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLAPTRAP_HPP
# define CLAPTRAP_HPP
# include <string>

class ClapTrap {
private:
	std::string _name;
	std::size_t _h_point, _e_point, _damage;

	void state_print( int num ) const;
public:
	ClapTrap( void );
	ClapTrap( const std::string name );
	ClapTrap( const ClapTrap& copy );
	ClapTrap& operator=( const ClapTrap& copy );
	~ClapTrap( void );

	void attack( const std::string& target );
	void takeDamage( unsigned int amount );
	void beRepaired( unsigned int amount );
};

#endif
 
