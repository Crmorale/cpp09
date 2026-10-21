#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <vector>
#include <deque>
#include <cstddef>

class PmergeMe
{
public:
	PmergeMe();
	PmergeMe(const PmergeMe &other);
	PmergeMe &operator=(const PmergeMe &other);
	~PmergeMe();

	void	parse(int argc, char **argv);
	void	run();

private:
	std::vector<int>	_vec;
	std::deque<int>		_deq;

	// Ordenan indices (posiciones en _vec / _deq), no los numeros en si
	std::vector<size_t>	sortVector(const std::vector<size_t> &idx);
	std::deque<size_t>	sortDeque(const std::deque<size_t> &idx);

	// Busqueda binaria en [0, end) e insercion del elemento
	void	insertVector(std::vector<size_t> &chain, size_t elem, size_t end);
	void	insertDeque(std::deque<size_t> &chain, size_t elem, size_t end);
};

#endif
