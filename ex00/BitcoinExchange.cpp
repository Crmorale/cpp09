#include "BitcoinExchange.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cctype>

BitcoinExchange::BitcoinExchange() {}

BitcoinExchange::BitcoinExchange(const BitcoinExchange &other) : _rates(other._rates) {}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange &other)
{
	if (this != &other)
		_rates = other._rates;
	return *this;
}

BitcoinExchange::~BitcoinExchange() {}

/* ------------------------------------------------------------------ */
/*  Utilities                                                         */
/* ------------------------------------------------------------------ */

std::string BitcoinExchange::trim(const std::string &str)
{
	std::string::size_type start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	std::string::size_type end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

// Comprueba formato YYYY-MM-DD y que la fecha exista en el calendario
bool BitcoinExchange::isValidDate(const std::string &date)
{
	if (date.size() != 10 || date[4] != '-' || date[7] != '-')
		return false;
	for (std::string::size_type i = 0; i < date.size(); ++i)
	{
		if (i == 4 || i == 7)
			continue;
		if (!std::isdigit(static_cast<unsigned char>(date[i])))
			return false;
	}
	int year = std::atoi(date.substr(0, 4).c_str());
	int month = std::atoi(date.substr(5, 2).c_str());
	int day = std::atoi(date.substr(8, 2).c_str());

	if (month < 1 || month > 12 || day < 1)
		return false;

	int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	if (leap)
		daysInMonth[1] = 29;
	return day <= daysInMonth[month - 1];
}

// Convierte el string a double. Devuelve false si no es un numero valido.
bool BitcoinExchange::parseValue(const std::string &str, double &value)
{
	if (str.empty())
		return false;
	std::istringstream iss(str);
	iss >> value;
	if (iss.fail())
		return false;
	// No debe quedar nada despues del numero
	std::string rest;
	iss >> rest;
	return rest.empty();
}

/* ------------------------------------------------------------------ */
/*  Data base                                                     */
/* ------------------------------------------------------------------ */

void BitcoinExchange::loadDatabase(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Error: could not open database.");

	std::string line;
	if (!std::getline(file, line))
		throw std::runtime_error("Error: empty database.");

	while (std::getline(file, line))
	{
		line = trim(line);
		if (line.empty())
			continue;
		std::string::size_type comma = line.find(',');
		if (comma == std::string::npos)
			throw std::runtime_error("Error: bad database line => " + line);

		std::string date = trim(line.substr(0, comma));
		std::string rateStr = trim(line.substr(comma + 1));
		double rate;
		if (!isValidDate(date) || !parseValue(rateStr, rate) || rate < 0)
			throw std::runtime_error("Error: bad database line => " + line);
		_rates[date] = rate;
	}
	if (_rates.empty())
		throw std::runtime_error("Error: empty database.");
}

/* ------------------------------------------------------------------ */
/*  Fichero de entrada                                                 */
/* ------------------------------------------------------------------ */

void BitcoinExchange::processInput(const std::string &filename) const
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Error: could not open file.");

	std::string line;
	bool first = true;
	while (std::getline(file, line))
	{
		// Saltamos la cabecera "date | value" si esta en la primera linea
		if (first)
		{
			first = false;
			if (trim(line) == "date | value")
				continue;
		}
		if (trim(line).empty())
			continue;
		processLine(line);
	}
}

void BitcoinExchange::processLine(const std::string &line) const
{
	std::string::size_type sep = line.find(" | ");
	if (sep == std::string::npos)
	{
		std::cout << "Error: bad input => " << line << std::endl;
		return;
	}

	std::string date = trim(line.substr(0, sep));
	std::string valueStr = trim(line.substr(sep + 3));

	if (!isValidDate(date))
	{
		std::cout << "Error: bad input => " << date << std::endl;
		return;
	}

	double value;
	if (!parseValue(valueStr, value))
	{
		std::cout << "Error: bad input => " << line << std::endl;
		return;
	}
	if (value < 0)
	{
		std::cout << "Error: not a positive number." << std::endl;
		return;
	}
	if (value > 1000)
	{
		std::cout << "Error: too large a number." << std::endl;
		return;
	}

	// upper_bound devuelve la primera fecha > date; la anterior es
	// la fecha exacta o la inmediatamente inferior.
	std::map<std::string, double>::const_iterator it = _rates.upper_bound(date);
	if (it == _rates.begin())
	{
		std::cout << "Error: no data available before => " << date << std::endl;
		return;
	}
	--it;
	std::cout << date << " => " << valueStr << " = " << value * it->second << std::endl;
}
