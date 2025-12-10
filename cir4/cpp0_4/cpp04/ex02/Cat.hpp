#ifndef CAT_HPP
# define CAT_HPP
# include "Animal.hpp"
# include "Brain.hpp"

class Cat : public Animal {
private:
	Brain* brain;

	void p_cat( int num ) const;
protected:
public:
    Cat( void );
    Cat( const Cat& copy );
    Cat& operator=( const Cat& copy );
    virtual ~Cat( void );

    const std::string& getType( void ) const;
    const std::string& getOldid( const int mem ) const;
    void setNewid( const int mem, const std::string& novel) const ;
    void makeSound( void ) const;
};
#endif
