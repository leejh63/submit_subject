/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   brain.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 12:18:43 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/23 12:18:44 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BRAIN_HPP
# define BRAIN_HPP
# include <string>

class Brain {
private:
	std::string ideas[100];

	void p_log( const std::string& log_msg) const;
protected:
public:
	Brain( void );
	Brain( const std::string& focus );
	Brain( const Brain& copy );
	virtual ~Brain( void );
	Brain& operator=( const Brain& copy );
	
	const std::string& getIdeas( int mem ) const;
	void setIdeas( const int mem, const std::string& novel ); 
};

#endif
