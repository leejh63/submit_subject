#ifndef CHARACTER_HPP
# define CHARACTER_HPP
# include "ICharacter.hpp"
# include <string>

class Character : public ICharacter {
    private:
        std::string _name;
        AMateria *slot[4];
    
        void plog( int num ) const;
    
        Character( void );
	public:
        Character( const std::string& name );
        Character( const Character& copy );
        Character& operator=( const Character& copy );
        virtual ~Character( void );
        std::string const & getName( void ) const;
        void equip( AMateria* m );
        void unequip( int idx );
        void use( int idx, ICharacter& target );
};

#endif
