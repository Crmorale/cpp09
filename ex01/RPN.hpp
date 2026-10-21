#ifndef RPN_HPP
#define RPN_HPP

#include <stack>
#include <list>
#include <string>

class RPN
{
public:
	RPN();
	RPN(const RPN &other);
	RPN &operator=(const RPN &other);
	~RPN();

	// Evalua la expresion en notacion polaca inversa. Lanza excepcion si hay error.
	long	evaluate(const std::string &expression);

private:
	// std::stack usa std::deque por defecto; le pasamos std::list como
	// contenedor subyacente para dejar libres vector y deque para el ex02.
	std::stack<long, std::list<long> >	_stack;

	static bool	isOperator(const std::string &token);
	void		applyOperator(char op);
};

#endif
