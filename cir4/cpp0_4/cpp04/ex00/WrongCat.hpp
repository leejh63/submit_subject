#ifndef WRONGCAT_HPP
# define WRONGCAT_HPP
# include "WrongAnimal.hpp"

class WrongCat : virtual public WrongAnimal {
private:
	void p_wcat( int num ) const;
protected:
public:
    WrongCat( void );
    WrongCat( const WrongCat& copy );
    WrongCat& operator=( const WrongCat& copy );
    virtual ~WrongCat( void );

    virtual const std::string& getType( void ) const;
    void makeSound( void ) const;
};
#endif
