#ifndef DOG_HPP
# define DOG_HPP
# include "Animal.hpp"

class Dog : public Animal {
private:
	void p_dog( int num ) const;
protected:
public:
    Dog( void );
    Dog( const Dog& copy );
    Dog& operator=( const Dog& copy );
    virtual ~Dog( void );

    const std::string& getType( void ) const;
    void makeSound( void ) const;
};
#endif
