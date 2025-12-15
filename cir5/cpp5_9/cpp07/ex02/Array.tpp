/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Array.tpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/10 17:18:19 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/10 17:18:32 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

template <typename T>
Array<T>::Array( void ) : _data(NULL), _size(0) {}

template <typename T>
Array<T>::Array( size_t n ) : _data(new T[n]()), _size(n) {}

template <typename T>
Array<T>::Array( const Array& other )
    : _data(new T[other._size]), _size(other._size)
{
    for (size_t i = 0; i < _size; i++)
        _data[i] = other._data[i];
}


/* 문제 new에서 할당 실패시 예외 발생 터짐 그런데 이미 data는 없애버림 순서 중요함
	할당 받은 후 복사후 교체진행하는게 안전
    delete[] _data;
    _data = new T[other._size];
    _size = other._size;
    for (size_t i = 0; i < _size; i++)
        _data[i] = other._data[i];
*/

template <typename T>
Array<T>& Array<T>::operator=( const Array& other ) {
    if (this != &other) {
        T* newData = NULL;
        if (other._size > 0)
            newData = new T[other._size];

        for (size_t i = 0; i < other._size; ++i)
            newData[i] = other._data[i];

        delete[] _data;
        _data = newData;
        _size = other._size;
    }
    return *this;
}

template <typename T>
Array<T>::~Array( void ) {
    delete[] _data;
}

template <typename T>
T& Array<T>::operator[]( size_t idx ) {
    if (idx >= _size)
        throw std::out_of_range("non_const Index out of range");
    return _data[idx];
}

template <typename T>
const T& Array<T>::operator[]( size_t idx ) const {
    if (idx >= _size)
        throw std::out_of_range("const Index out of range");
    return _data[idx];
}

template <typename T>
size_t Array<T>::size( void ) const {
    return _size;
}
