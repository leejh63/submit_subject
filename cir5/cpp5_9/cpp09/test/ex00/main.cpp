/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/15 10:36:04 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/12/15 10:36:05 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//#include "BitcoinExchange.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <limits>
#include <sstream>
#include <iomanip>

enum line_status {
	L_OK,
	TRIM_EMPTY,
	ONLY_SPACE,
	INVA_D_FORMAT,
	INVA_D_HEADER,
	INVA_D_DATE,
	INVA_D_RATE,
	
};



bool p_status( line_status status_code, std::size_t line_num ) {
	const static char* status_info[] = {
		"Data file is good!\n",
		"Empty line!",
		"Only Space!",
		"Invalid data format!",
		"Invalid data header!",
		"Invalid data date!",
		"Invalid data rate!",
	};

	if (status_code != L_OK) {
		std::cout << "line: " << line_num << ", " << status_info[status_code]; return true;
	}
	else {
		std::cout << status_info[status_code]; return false;
	}
}



bool line_trim( std::string& line, std::size_t line_num ) {
	if (line.empty()) { return p_status(TRIM_EMPTY, line_num); }

	const char* space_word = " \t\n\v\f\r";
	std::size_t start, last;
	
	start = line.find_first_not_of(space_word);
	if (start == std::string::npos)  { return p_status(ONLY_SPACE, line_num); }

	last = line.find_last_not_of(space_word);
	line = line.substr(start, last - start + 1);
	return false;
}

void word_trim( std::string& word ) {

	const char* space_word = " \t\n\v\f\r";

	std::size_t start, last;

	start = word.find_first_not_of(space_word);

	last = word.find_last_not_of(space_word);

	word = word.substr(start, last - start + 1);
}




bool is_all_digit( std::string& word ) {
	for (std::size_t i = 0; i < word.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(word[i])))
			return true;
	}
	return false;
}



bool is_leap_year(int yy) {
	return (yy % 4 == 0) && (yy % 100 != 0 || yy % 400 == 0);
}

int get_day( int yy, int mm ) {
	const static int day[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
	if (mm == 2) { 
		return (day[mm] + (is_leap_year(yy) ? 1 : 0));
	}
	return day[mm];
}

bool is_date_vaild( std::string& date ) {
	if (date.size() != 10 || date[4] != '-' || date[7] != '-') { return true; }
	
	std::string yy = date.substr(0, 4);
	std::string mm = date.substr(5, 2);
	std::string dd = date.substr(8, 2);
	if ( is_all_digit(yy) || is_all_digit(mm) || is_all_digit(dd) ) { return true; }
    
	int year, month, day;
	{ std::istringstream ys(yy); ys >> year; }
	{ std::istringstream ms(mm); ms >> month; }
	{ std::istringstream ds(dd); ds >> day; }
    
	if (month < 1 || month > 12) { return true; }
	
	if (day < 1 || day > get_day(year, month)) { return true; }
    
	return false;
}

bool is_only_double( std::string& rate ) {
	std::size_t i = 0;
	if (rate[i] == '+') { ++i; }
	int dot = 0;
	for (; i < rate.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(rate[i]))) {
			if (!dot && rate[i] == '.') {
				dot = 1; continue;
			}
			return true;
		}
	}
	return false;
}

bool is_rate_vaild( std::string& rate, double& new_rate ) {
	word_trim(rate);
	if (is_only_double(rate)) { return true; }

	double d_rate;
	std::istringstream srate(rate);
	srate >> d_rate;
	if (!srate || !srate.eof()) { return true; }
	if (d_rate < 0) { return true; }
	new_rate = d_rate;
	return false;
}

bool data_parser( std::string& line, size_t line_num ) {

	std::size_t f_comma = line.find(',');
	if (f_comma == std::string::npos || f_comma == 0 || f_comma == (line.size() - 1)) {
		return p_status(INVA_D_FORMAT, line_num);
	}
	
	if (line.find(',', f_comma + 1) != std::string::npos) { return p_status(INVA_D_FORMAT, line_num); }
	
	std::string date = line.substr(0, f_comma);
	std::string rate = line.substr(f_comma + 1);
	std::cout << line_num << ": =date=" << date << "=  |  =rate=" << rate << "=\n";

	if (is_date_vaild(date)) { return p_status(INVA_D_DATE, line_num); }
	std::cout << line_num << ": V_date=" << date << "=\n";

	double d_rate;
	if (is_rate_vaild(rate, d_rate)) { return p_status(INVA_D_RATE, line_num); }
	std::cout << line_num << ": V_rate=" << std::setprecision(17) << d_rate << "=\n";
	return false;
}


bool is_datafile_valid( std::ifstream& file ) {
	std::cout << "======data======\n";
	std::string file_line;
	std::size_t file_line_num = 1;
	std::getline(file, file_line);
	std::cout << "line 0" << ": ==" << file_line << "==\n";
	if (file_line != "date,exchange_rate") { return p_status(INVA_D_HEADER, file_line_num); }


	while (std::getline(file, file_line)) {
		std::cout << file_line_num << ": ==" << file_line << "==\n";
		if (line_trim(file_line, file_line_num)) { return true; }
		if (data_parser(file_line, file_line_num)) { return true; }
		++file_line_num;
	}
	return false;
}



int main(int argc, char* argv[]) {
	// check the para
	if ( argc != 2) { std::cout << "Arg: ./bit <filepath>\n"; return 1; }
	
	// tmp argv[1] use
	std::cout << "Need to open and verify the required rules: "<< argv[1] << "\n";
	
	// open & check the file open status
	std::ifstream data_file;
	data_file.open("test.csv");
	if (!data_file.is_open()) { std::cout << "Can't open data file!\n"; return 1; }
	
	// check the datafile
	if (is_datafile_valid(data_file))  { std::cout << ", Data file wrong!\n"; return 1; }
	
	return 0;
}













































