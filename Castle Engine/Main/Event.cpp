#include "Event.hpp"

void EventFloatPtr::Add(const CallbackFloatPtr& callback) { actions.push_back(callback); };
void Event::Add(const Action& act) { actions.push_back(act); }
void Eventi::Add(const Actioni& func) { actions.push_back(func); }
void Eventf::Add(const Actionf& func) { actions.push_back(func); }
void Action2i::Add(const Changed2i& act) { actions.push_back(act); }
void Action2f::Add(const Changed2f& act) { actions.push_back(act); }
void Action3f::Add(const Changed3f& act) { actions.push_back(act); }

void Event::Invoke()  const { for (Action act : actions) act(); }

void Action2i::Invoke(const int& x, const int& y) const {
	for (Changed2i act : actions) act(x, y);
}

void Action2f::Invoke(const float& x, const float& y) const {
	for (Changed2f act : actions) act(x, y);
}

void Action3f::Invoke(const float& x, const float& y, const float& z) const {
	for (Changed3f act : actions) act(x, y, z);
}

void Action2i::Invoke(const std::tuple<int, int>& pos) const {
	for (Changed2i act : actions) act(std::get<0>(pos), std::get<1>(pos));
}

void Action2f::Invoke(const glm::vec2& pos) const {
	for (Changed2f act : actions) act(pos.x, pos.y);
}

void Action3f::Invoke(const glm::vec3& pos) const {
	for (Changed3f act : actions) act(pos.x, pos.y, pos.z);
}

void Eventi::Invoke(const int& x) const {
	for (Actioni act : actions) act(x);
}

void Eventf::Invoke(const float& x) const {
	for (Actionf act : actions) act(x);
}

void EventFloatPtr::Invoke(const float* mat)const
{
	for (CallbackFloatPtr act : actions) act(mat);
}
