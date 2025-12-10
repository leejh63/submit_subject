/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Serializer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 11:43:08 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/09 12:16:48 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Serializer.hpp"
#include <iostream>

uintptr_t Serializer::serialize( Data* ptr ) {
	uintptr_t toptr = reinterpret_cast<uintptr_t>(ptr);
	return toptr;
}

Data* Serializer::deserialize(uintptr_t raw) {
	Data* todata = reinterpret_cast<Data*>(raw);
	return todata;
}
