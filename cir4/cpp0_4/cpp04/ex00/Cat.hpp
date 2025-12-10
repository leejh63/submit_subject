#ifndef CAT_HPP
# define CAT_HPP
# include "Animal.hpp"

class Cat : public Animal {
private:
	void p_cat( int num ) const;
protected:
public:
    Cat( void );
    Cat( const Cat& copy );
    Cat& operator=( const Cat& copy );
    virtual ~Cat( void );

    const std::string& getType( void ) const;
    void makeSound( void ) const;
};
#endif
