/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Harl.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 11:31:48 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/16 11:31:49 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HARL_HPP
# define HARL_HPP
# include <string>
# include <iostream>
# include <stdint.h>

class Harl {
private:
	void debug( void );
	void info( void );
	void warning( void );
	void error( void );
	//
	void use_for( const std::string& level );
	void dump_from_claim_level( uintptr_t addr);
	/*/
	void use_switch( const std::string& level );
	//*/
public:
	void complain( std::string level );
};

#endif
