#ifndef ANIMAL_HPP
# define ANIMAL_HPP
# include <string>
# include <iostream>

class Animal {
private:
	void p_animal( int num ) const ;
protected:
    std::string type;
    void p_log( const std::string& log_msg, const std::string& _who) const;

public:
    Animal( void );
    Animal( const std::string& type );
    Animal( const Animal& copy );
    Animal& operator=( const Animal& copy );
    virtual ~Animal( void );

    virtual const std::string& getType( void ) const;
    virtual void makeSound( void ) const;
};

#endif
