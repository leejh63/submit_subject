/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Account.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:11:23 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/09/14 15:11:24 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Account.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

int Account::_nbAccounts         = 0;
int Account::_totalAmount        = 0;
int Account::_totalNbDeposits    = 0;
int Account::_totalNbWithdrawals = 0;

int	Account::getNbAccounts( void ) { return Account::_nbAccounts; }
int	Account::getTotalAmount( void ) { return Account::_totalAmount; }
int	Account::getNbDeposits( void ) { return Account::_totalNbDeposits; }
int	Account::getNbWithdrawals( void ) { return Account::_totalNbWithdrawals; }

int	Account::checkAmount( void ) const { return this->_amount; }

Account::Account( int initial_deposit ) :
	_accountIndex(getNbAccounts()), 
	_amount(initial_deposit),
	_nbDeposits(0),
	_nbWithdrawals(0)
{
	_displayTimestamp();

	std::cout << "index:" << getNbAccounts()
		  << ";amount:" << checkAmount()
		  << ";created\n";
	_nbAccounts += 1;
	_totalAmount += initial_deposit;
}

Account::~Account( void ) {
	_displayTimestamp();

	std::cout << "index:" << _accountIndex 
		  << ";amount:" << checkAmount() 
		  << ";closed\n";
};

void	Account::_displayTimestamp( void ) {
    std::time_t t = std::time(0);
    char mbstr[100];
    if (std::strftime(mbstr, sizeof(mbstr), "[%Y%m%d_%H%M%S]", std::localtime(&t)))
        std::cout << mbstr << ' ';
}

void	Account::displayAccountsInfos( void ) {
	_displayTimestamp();
	
	std::cout << "accounts:" << getNbAccounts()
		  << ";total:" << getTotalAmount()
		  << ";deposits:" << _totalNbDeposits
		  << ";withdrawals:" << _totalNbWithdrawals
		  << '\n';
}


void	Account::displayStatus( void ) const {
	_displayTimestamp();
	std::cout << "index:" << _accountIndex
		  << ";amount:" << checkAmount()
		  << ";deposits:" << _nbDeposits
		  << ";withdrawals:" << _nbWithdrawals
		  << '\n';
}

void	Account::makeDeposit( int deposit ) {
	int pamt = checkAmount();
	
	_amount += deposit;
	_totalAmount += deposit;
	_nbDeposits += 1;
	_totalNbDeposits += 1;

	_displayTimestamp();
	std::cout << "index:" << _accountIndex
		  << ";p_amount:" << pamt
		  << ";deposit:" << deposit
		  << ";amount:" << checkAmount()
		  << ";nb_deposits:" << _nbDeposits
		  << '\n';
}

bool	Account::makeWithdrawal( int withdrawal ) {
	int pamt = checkAmount();
	
	if (_amount < withdrawal) {
		_displayTimestamp();
		std::cout << "index:" << _accountIndex
			  << ";p_amount:" << pamt
			  << ";withdrawal:" << "refused"
			  << '\n';
		return false;
	}

	_amount -= withdrawal;
	_totalAmount -= withdrawal;
	_nbWithdrawals += 1;
	_totalNbWithdrawals += 1;

	_displayTimestamp();
	std::cout << "index:" << _accountIndex
		  << ";p_amount:" << pamt
		  << ";withdrawal:" << withdrawal
		  << ";amount:" << checkAmount()
		  << ";nb_withdrawals:" << _nbWithdrawals
		  << '\n';
	return true;
}






























