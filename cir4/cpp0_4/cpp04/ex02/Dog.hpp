#ifndef DOG_HPP
# define DOG_HPP
# include "Animal.hpp"
# include "Brain.hpp"

class Dog : public Animal {
private:
	Brain* brain;
	
	void p_dog( int num ) const;
protected:
public:
    Dog( void );
    Dog( const Dog& copy );
    Dog& operator=( const Dog& copy );
    virtual ~Dog( void );

    const std::string& getType( void ) const;
    const std::string& getOldid( const int mem ) const;
    void setNewid( const int mem, const std::string& novel) const ;
    void makeSound( void ) const;
};
#endif
