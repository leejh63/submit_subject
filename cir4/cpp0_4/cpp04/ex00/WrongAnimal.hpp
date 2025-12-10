#ifndef WRONGANIMAL_HPP
# define WRONGANIMAL_HPP
# include <string>
# include <iostream>

class WrongAnimal {
private:
	void p_wanimal( int num ) const ;
protected:
    std::string type;
    void p_log( const std::string& log_msg, const std::string& _who) const;

public:
    WrongAnimal( void );
    WrongAnimal( const std::string& type );
    WrongAnimal( const WrongAnimal& copy );
    WrongAnimal& operator=( const WrongAnimal& copy );
    virtual ~WrongAnimal( void );

    virtual const std::string& getType( void ) const;
    void makeSound( void ) const;
};

#endif
