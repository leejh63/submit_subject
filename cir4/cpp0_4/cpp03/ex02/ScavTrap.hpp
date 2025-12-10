/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ScavTrap.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/18 16:43:44 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/18 16:43:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCAVTRAP_HPP
# define SCAVTRAP_HPP
# include "ClapTrap.hpp"

class ScavTrap : public ClapTrap {
private:
	bool _keeper;

protected:
	const char* g_class( void ) const;

public:
    ScavTrap( void );
    ScavTrap( const std::string& name );
    ScavTrap( const ScavTrap& other );
    ScavTrap& operator=(const ScavTrap& other );

    virtual ~ScavTrap( void );

    void attack( const std::string& target );
    void guardGate( void );
};
#endif
