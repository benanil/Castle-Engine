#pragma once
#ifndef _EVENT_H
#define _EVENT_H

#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include <tuple>

typedef void(*Actionf)(int);
typedef void(*Actioni)(int);

typedef void(*Changed2i)(const int&, const int&);
typedef void(*Changed2f)(const float&, const float&);
typedef void(*Changed3f)(const float&, const float&, const float&);

typedef void(*CallbackFloatPtr)(const float*);

typedef void(*Action)();

class Event
{
private:
	std::vector<Action> actions;
public:
	void Add(const Action& func);
	void Invoke() const;
	void operator()() { Invoke(); }
};

// matrix4 event
class EventFloatPtr
{
private:
	std::vector<CallbackFloatPtr> actions;
public:
	void Add(const CallbackFloatPtr& func);
	void Invoke(const float* mat) const;
	void operator()(const float* mat) { Invoke(mat); }
};

class Eventi
{
private:
	std::vector<Actioni> actions;
public:
	void Add(const Actioni& func);
	void Invoke(const int& x) const;
	void operator()(const int& x) { Invoke(x); }
};

class Eventf
{
private:
	std::vector<Actionf> actions;
public:
	void Add(const Actionf& func);
	void Invoke(const float& x) const;
	void operator()(const float& x) { Invoke(x); }
};

class Action2i
{
private:
	std::vector<Changed2i> actions;
public:
	void Add(const Changed2i& func);
	void Invoke(const int& x, const int& y) const;
	void Invoke(const std::tuple<int, int>& pos) const;
	void operator()(const std::tuple<int, int>& pos) { Invoke(pos); }
	void operator()(const int& x, const int& y) const { Invoke(x, y); };
};

class Action2f
{
private:
	std::vector<Changed2f> actions;
public:
	void Add(const Changed2f& func);
	void Invoke(const float& x, const float& y) const;
	void Invoke(const glm::vec2& pos) const;
	void operator()(const glm::vec2& pos) { Invoke(pos); }
	void operator()(const float& x, const float& y) const { Invoke(x, y); };
};

class Action3f
{
private:
	std::vector<Changed3f> actions;
public:
	void Add(const Changed3f& func);
	void Invoke(const float& x, const float& y, const float& z) const;
	void Invoke(const glm::vec3& pos) const;
	void operator()(const glm::vec3& pos) { Invoke(pos); }
	void operator()(const float& x, const float& y, const float& z) const { Invoke(x, y, z); };
};

#endif
