/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   easyfind.tpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/12 17:24:45 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/12 17:24:47 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

class NotFoundException : public std::exception {
public:
    const char* what( void ) const throw() {
        return "value not found";
    }
};

// non-const container
template <typename T>
typename T::iterator easyfind( T& container, int value ) {
    typename T::iterator it =
        std::find(container.begin(), container.end(), value);

    if (it == container.end())
        throw NotFoundException();

    return it;
}

// const container
template <typename T>
typename T::const_iterator easyfind( const T& container, int value ) {
    typename T::const_iterator it =
        std::find(container.begin(), container.end(), value);

    if (it == container.end())
        throw NotFoundException();

    return it;
}
