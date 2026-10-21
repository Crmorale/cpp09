#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <map>
#include <string>

class BitcoinExchange
{
public:
	BitcoinExchange();
	BitcoinExchange(const BitcoinExchange &other);
	BitcoinExchange &operator=(const BitcoinExchange &other);
	~BitcoinExchange();

	// Carga la base de datos de precios (data.csv). Lanza excepcion si falla.
	void	loadDatabase(const std::string &filename);
	// Procesa el fichero de entrada linea a linea e imprime los resultados.
	void	processInput(const std::string &filename) const;

private:
	std::map<std::string, double>	_rates;

	void		processLine(const std::string &line) const;
	static bool	isValidDate(const std::string &date);
	static bool	parseValue(const std::string &str, double &value);
	static std::string	trim(const std::string &str);
};

#endif
