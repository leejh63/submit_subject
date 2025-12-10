/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cure.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 13:26:17 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/24 17:05:18 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CURE_HPP
# define CURE_HPP
# include "AMateria.hpp"
# include <string>

class Cure : public AMateria {
public:
	void plog( int num ) const ;
protected:
public:
	Cure( void );
	Cure( const Cure& copy );
	Cure& operator=( const Cure& copy );
	virtual ~Cure( void );

	const std::string& getType( void ) const;
	AMateria* clone( void ) const;
	//virtual void use( const std::string& target ) const ;
	virtual void use( ICharacter& target );
};

#endif
