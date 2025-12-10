/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AMateria.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 10:41:57 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/24 16:38:19 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AMATERIA_HPP
# define AMATERIA_HPP
# include <string>

class ICharacter;

extern const char* const msg[];
extern bool skip;

enum {
	D_ctor, S_ctor, C_ctor, D_stor, 
	C_op,
	F_gtype, F_use, F_clone,
	F_eqp, F_ueqp, F_getName,
	F_learnM, F_createM,
	N_othing
};

class AMateria {
private:
	AMateria( void );
protected:
	std::string type;

	virtual void plog( int num ) const ;
public:
	AMateria( const std::string& type );
	AMateria( const AMateria& copy );
	AMateria& operator=( const AMateria& copy );
	virtual ~AMateria( void );

	const std::string& getType( void ) const;
	virtual AMateria* clone( void ) const = 0;
	//virtual void use( const std::string& target ) const ;
	virtual void use( ICharacter& target );
};

#endif
