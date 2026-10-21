#include "PmergeMe.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <climits>
#include <cctype>
#include <ctime>

PmergeMe::PmergeMe() {}

PmergeMe::PmergeMe(const PmergeMe &other) : _vec(other._vec), _deq(other._deq) {}

PmergeMe &PmergeMe::operator=(const PmergeMe &other)
{
	if (this != &other)
	{
		_vec = other._vec;
		_deq = other._deq;
	}
	return *this;
}

PmergeMe::~PmergeMe() {}

/* ------------------------------------------------------------------ */
/*  Parseo: solo enteros positivos que quepan en un int               */
/* ------------------------------------------------------------------ */

void PmergeMe::parse(int argc, char **argv)
{
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg.empty() || arg.size() > 10)
			throw std::runtime_error("bad argument");
		for (size_t j = 0; j < arg.size(); j++)
			if (!std::isdigit(static_cast<unsigned char>(arg[j])))
				throw std::runtime_error("bad argument");

		std::istringstream iss(arg);
		long n;
		iss >> n;
		if (n <= 0 || n > INT_MAX)
			throw std::runtime_error("bad argument");
		_vec.push_back(n);
		_deq.push_back(n);
	}
}

/*
** IDEA GENERAL
** No ordenamos los numeros directamente, sino sus indices (posiciones en
** _vec o _deq). Asi cada elemento es unico aunque haya numeros repetidos,
** y podemos apuntar la pareja de cada grande en una tabla:
**     partner[grande] = pequeno
** Cuando la recursion nos devuelve los grandes ordenados, sacamos la pareja
** de cada uno al instante, sin tener que buscarla.
** Las comparaciones siempre se hacen con los valores: _vec[a] < _vec[b].
**
** ORDEN DE JACOBSTHAL
** Numeros de Jacobsthal: 1, 3, 5, 11, 21, 43...  J(n+1) = J(n) + 2 * J(n-1)
** Los pequenos (pend) se insertan por grupos. Cada grupo acaba en un
** Jacobsthal y se recorre de mayor a menor (indices desde 0):
**     2 1 | 4 3 | 10 9 8 7 6 5 | 20 19 ... 11 | ...
** pend[0] ya esta colocado al principio, asi que no aparece.
**
** HASTA DONDE BUSCAR
** Cada pequeno pend[k] es <= que su grande bigs[k], asi que solo hace falta
** buscarle sitio a la izquierda de bigs[k]. Delante de bigs[k] solo pueden
** estar bigs[0..k-1] y los pequenos ya insertados, por lo que bigs[k] esta
** como mucho en la posicion k + inserted. Buscamos en [0, k + inserted).
*/

/* ------------------------------------------------------------------ */
/*  Ford-Johnson con std::vector                                      */
/* ------------------------------------------------------------------ */

void PmergeMe::insertVector(std::vector<size_t> &chain, size_t elem, size_t end)
{
	if (end > chain.size())
		end = chain.size();
	// Busqueda binaria: primera posicion cuyo valor no es menor que elem
	size_t low = 0;
	size_t high = end;
	while (low < high)
	{
		size_t mid = (low + high) / 2;
		if (_vec[chain[mid]] < _vec[elem])
			low = mid + 1;
		else
			high = mid;
	}
	chain.insert(chain.begin() + low, elem);
}

std::vector<size_t> PmergeMe::sortVector(const std::vector<size_t> &idx)
{
	if (idx.size() <= 1)
		return idx;

	// 1. Hacer parejas: el grande va a bigs y apuntamos cual es su pequeno.
	//    Si sobra uno (tamano impar), se guarda aparte.
	std::vector<size_t> bigs;
	std::vector<size_t> partner(_vec.size());
	for (size_t i = 0; i + 1 < idx.size(); i += 2)
	{
		size_t big = idx[i];
		size_t small = idx[i + 1];
		if (_vec[big] < _vec[small])
			std::swap(big, small);
		bigs.push_back(big);
		partner[big] = small;
	}
	bool hasStraggler = (idx.size() % 2 == 1);

	// 2. Ordenar los grandes con el mismo algoritmo (recursion)
	bigs = sortVector(bigs);

	// 3. pend[i] es la pareja de bigs[i]. El suelto va al final.
	std::vector<size_t> pend;
	for (size_t i = 0; i < bigs.size(); i++)
		pend.push_back(partner[bigs[i]]);
	if (hasStraggler)
		pend.push_back(idx[idx.size() - 1]);

	// 4. Cadena principal = grandes ordenados.
	//    pend[0] <= bigs[0], que es el menor: va directo al principio.
	std::vector<size_t> chain = bigs;
	chain.insert(chain.begin(), pend[0]);
	size_t inserted = 1;

	// 5. Insertar el resto en orden de Jacobsthal con busqueda binaria
	size_t prev = 1;	// J(n-1)
	size_t curr = 3;	// J(n)
	size_t done = 1;	// cuantos pequenos estan ya colocados
	while (done < pend.size())
	{
		size_t top = std::min(curr, pend.size());
		for (size_t k = top - 1; k >= done; k--)
		{
			insertVector(chain, pend[k], k + inserted);
			inserted++;
		}
		done = top;
		size_t next = curr + 2 * prev;
		prev = curr;
		curr = next;
	}
	return chain;
}

/* ------------------------------------------------------------------ */
/*  Ford-Johnson con std::deque (mismo algoritmo)                     */
/* ------------------------------------------------------------------ */

void PmergeMe::insertDeque(std::deque<size_t> &chain, size_t elem, size_t end)
{
	if (end > chain.size())
		end = chain.size();
	size_t low = 0;
	size_t high = end;
	while (low < high)
	{
		size_t mid = (low + high) / 2;
		if (_deq[chain[mid]] < _deq[elem])
			low = mid + 1;
		else
			high = mid;
	}
	chain.insert(chain.begin() + low, elem);
}

std::deque<size_t> PmergeMe::sortDeque(const std::deque<size_t> &idx)
{
	if (idx.size() <= 1)
		return idx;

	// 1. Parejas
	std::deque<size_t> bigs;
	std::deque<size_t> partner(_deq.size());
	for (size_t i = 0; i + 1 < idx.size(); i += 2)
	{
		size_t big = idx[i];
		size_t small = idx[i + 1];
		if (_deq[big] < _deq[small])
			std::swap(big, small);
		bigs.push_back(big);
		partner[big] = small;
	}
	bool hasStraggler = (idx.size() % 2 == 1);

	// 2. Recursion sobre los grandes
	bigs = sortDeque(bigs);

	// 3. Pequenos en el mismo orden que sus grandes
	std::deque<size_t> pend;
	for (size_t i = 0; i < bigs.size(); i++)
		pend.push_back(partner[bigs[i]]);
	if (hasStraggler)
		pend.push_back(idx[idx.size() - 1]);

	// 4. Cadena principal
	std::deque<size_t> chain = bigs;
	chain.push_front(pend[0]);
	size_t inserted = 1;

	// 5. Insercion en orden de Jacobsthal
	size_t prev = 1;
	size_t curr = 3;
	size_t done = 1;
	while (done < pend.size())
	{
		size_t top = std::min(curr, pend.size());
		for (size_t k = top - 1; k >= done; k--)
		{
			insertDeque(chain, pend[k], k + inserted);
			inserted++;
		}
		done = top;
		size_t next = curr + 2 * prev;
		prev = curr;
		curr = next;
	}
	return chain;
}

/* ------------------------------------------------------------------ */
/*  Ejecucion                                                         */
/* ------------------------------------------------------------------ */

void PmergeMe::run()
{
	std::cout << "Before:";
	for (size_t i = 0; i < _vec.size(); i++)
		std::cout << " " << _vec[i];
	std::cout << std::endl;

	// vector: indices 0..n-1 -> ordenar -> pasar de indices a valores
	std::clock_t start = std::clock();
	std::vector<size_t> idxVec;
	for (size_t i = 0; i < _vec.size(); i++)
		idxVec.push_back(i);
	idxVec = sortVector(idxVec);
	std::vector<int> sortedVec;
	for (size_t i = 0; i < idxVec.size(); i++)
		sortedVec.push_back(_vec[idxVec[i]]);
	double timeVec = (double)(std::clock() - start) / CLOCKS_PER_SEC * 1000000;

	// deque: lo mismo
	start = std::clock();
	std::deque<size_t> idxDeq;
	for (size_t i = 0; i < _deq.size(); i++)
		idxDeq.push_back(i);
	idxDeq = sortDeque(idxDeq);
	std::deque<int> sortedDeq;
	for (size_t i = 0; i < idxDeq.size(); i++)
		sortedDeq.push_back(_deq[idxDeq[i]]);
	double timeDeq = (double)(std::clock() - start) / CLOCKS_PER_SEC * 1000000;

	std::cout << "After:";
	for (size_t i = 0; i < sortedVec.size(); i++)
		std::cout << " " << sortedVec[i];
	std::cout << std::endl;

	std::cout << "Time to process a range of " << _vec.size()
			  << " elements with std::vector : " << timeVec << " us" << std::endl;
	std::cout << "Time to process a range of " << _deq.size()
			  << " elements with std::deque  : " << timeDeq << " us" << std::endl;
}
