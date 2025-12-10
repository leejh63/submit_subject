/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FragTrap.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/19 19:43:41 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/19 19:43:42 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FRAGTRAP_HPP
# define FRAGTRAP_HPP
# include "ClapTrap.hpp"

class FragTrap : public ClapTrap {
private:
protected:
	const char* g_class( void ) const;
public:
	FragTrap( void );
	FragTrap( const std::string& name );
	FragTrap( const FragTrap& copy );
	FragTrap& operator=( const FragTrap& copy );
	virtual ~FragTrap( void );

	void attack( const std::string& target );
	void highFivesGuys( void );
};
#endif
