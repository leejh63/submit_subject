/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Contact.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:31:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/13 13:31:53 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONTACT_HPP

# define CONTACT_HPP
# include <string>
# include <iostream>
# include <iomanip>

class Contact {
public:
	Contact();
	~Contact( void );
	
	bool isempty( void ) const ;
	void pmem( void ) const ;
	
	const std::string& first( void ) const ;
	const std::string& last( void ) const ;
	const std::string& nick( void ) const ;

	void set(const std::string conset[5]);
	
private:
	std::string _firstname;
	std::string _lastname;
	std::string _nickname;
	std::string _phonenumber;
	std::string _darkestsecret;
};

#endif
