/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DiamondTrap.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/19 22:07:18 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/19 22:07:18 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIAMONDTRAP_HPP
# define DIAMONDTRAP_HPP
# include "FragTrap.hpp"
# include "ScavTrap.hpp"
# include <string>

class DiamondTrap
: public FragTrap, public ScavTrap 
{
private:
    std::string _name;

protected:
    virtual const char* g_class(void) const;

public:
    DiamondTrap(void);
    explicit DiamondTrap(const std::string& name);
    DiamondTrap(const DiamondTrap& other);
    DiamondTrap& operator=(const DiamondTrap& other);
    virtual ~DiamondTrap(void);

    void attack(const std::string& target);

    void whoAmI(void);
};

#endif

