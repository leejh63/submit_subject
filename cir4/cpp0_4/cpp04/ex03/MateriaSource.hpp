#ifndef MATERIASOURCE_HPP
# define MATERIASOURCE_HPP
# include "IMateriaSource.hpp"
# include <string>

class MateriaSource : public IMateriaSource {
private:
    AMateria* magics[4];
    void plog( int num ) const;
    MateriaSource( const MateriaSource& copy );
    MateriaSource& operator=( const MateriaSource& copy );

public:
    MateriaSource( void );

    virtual ~MateriaSource();
    
    void learnMateria(AMateria*);
    AMateria* createMateria(std::string const & type);
};

#endif
