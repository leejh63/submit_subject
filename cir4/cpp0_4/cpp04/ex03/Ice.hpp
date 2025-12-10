/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Ice.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 13:26:17 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/24 16:12:52 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ICE_HPP
# define ICE_HPP
# include "AMateria.hpp"
# include <string>

class Ice : public AMateria {
public:
	void plog( int num ) const ;
protected:
public:
	Ice( void );
	Ice( const Ice& copy );
	Ice& operator=( const Ice& copy );
	virtual ~Ice( void );

	const std::string& getType( void ) const;
	Ice* clone( void ) const;
	//virtual void use( const std::string& target ) const ;
	virtual void use( ICharacter& target );
};

#endif
