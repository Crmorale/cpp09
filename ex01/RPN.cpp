#include "RPN.hpp"
#include <sstream>
#include <stdexcept>
#include <cctype>

RPN::RPN() {}

RPN::RPN(const RPN &other) : _stack(other._stack) {}

RPN &RPN::operator=(const RPN &other)
{
	if (this != &other)
		_stack = other._stack;
	return *this;
}

RPN::~RPN() {}

bool RPN::isOperator(const std::string &token)
{
	return token.size() == 1
		&& (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/');
}

void RPN::applyOperator(char op)
{
	if (_stack.size() < 2)
		throw std::runtime_error("not enough operands");

	// El orden importa: el primero que sale es el operando derecho
	long right = _stack.top();
	_stack.pop();
	long left = _stack.top();
	_stack.pop();

	long result = 0;
	switch (op)
	{
		case '+': result = left + right; break;
		case '-': result = left - right; break;
		case '*': result = left * right; break;
		case '/':
			if (right == 0)
				throw std::runtime_error("division by zero");
			result = left / right;
			break;
	}
	_stack.push(result);
}

long RPN::evaluate(const std::string &expression)
{
	// Vaciamos la pila por si el objeto se reutiliza
	while (!_stack.empty())
		_stack.pop();

	std::istringstream iss(expression);
	std::string token;
	while (iss >> token)
	{
		if (token.size() == 1 && std::isdigit(static_cast<unsigned char>(token[0])))
			_stack.push(token[0] - '0');
		else if (isOperator(token))
			applyOperator(token[0]);
		else
			throw std::runtime_error("invalid token");
	}
	if (_stack.size() != 1)
		throw std::runtime_error("invalid expression");
	return _stack.top();
}
