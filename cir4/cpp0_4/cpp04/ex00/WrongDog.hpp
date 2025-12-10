#ifndef WRONGDOG_HPP
# define WRONGDOG_HPP
# include "WrongAnimal.hpp"

class WrongDog : virtual public WrongAnimal {
private:
	void p_wdog( int num ) const;
protected:
public:
    WrongDog( void );
    WrongDog( const WrongDog& copy );
    WrongDog& operator=( const WrongDog& copy );
    virtual ~WrongDog( void );

    virtual const std::string& getType( void ) const;
    void makeSound( void ) const;
};
#endif
