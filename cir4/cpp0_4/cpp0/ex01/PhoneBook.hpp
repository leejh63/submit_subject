/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PhoneBook.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 13:31:41 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/13 13:31:43 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHONEBOOK_HPP
# define PHONEBOOK_HPP
# include "Contact.hpp"
# include <string>
# include <iostream>
# include <iomanip>

class PhoneBook {
public :
	PhoneBook();
	~PhoneBook();

	void add( const std::string conset[5] );
	void search( void );
private :
	Contact carray[8];
	int now;
	
	void printen(const std::string word) const;
};

#endif
