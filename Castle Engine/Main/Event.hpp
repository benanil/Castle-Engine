#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <tuple>
#include <functional>

typedef void(*Actionf)(int);
typedef void(*Actioni)(int);

typedef void(*Changed2i)(const int&, const int&);
typedef void(*Changed2f)(const float&, const float&);
typedef void(*Changed3f)(const float&, const float&, const float&);

typedef void(*CallbackFloatPtr)(const float*);

typedef void(*Action)();

#define CREATE_FUNC(_type, name) typedef Func<std::function<void(const _type&)>, _type> name;
#define CREATE_FUNC2(_type, _typeTwo, name) typedef Func<std::function<void(const _type&, const _typeTwo&)>, _type, _typeTwo> name;

template<typename act, class... args>
class Func
{
public:
	std::vector<act> actions;
	Func() { }
	Func(const act& _act) { actions.push_back(_act); }
	void Add(const act& func) { actions.push_back(func); };

	void Invoke(const args&... _args) const
	{
		for (auto& action : actions) action(_args...);
	}
	void operator()(const args&... _args) { Invoke(_args...); }
	void Clear() { actions.clear(); }
};

class Event
{
private:
	std::vector<Action> actions;
public:
	void Add(const Action& act) { actions.push_back(act); }
	void Invoke()  const { for (Action act : actions) act(); }
	void operator()() { Invoke(); }
	void Clear() { actions.clear(); }
};

