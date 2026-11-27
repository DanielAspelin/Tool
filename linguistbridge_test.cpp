#include "linguistbridge.hpp"
#include <cassert>
int main(){
 LINGUISTBRIDGE bridge([](const reciprocal::Request& r){return r.operation=="probe";});
 reciprocal::Request r; r.id="test-1";r.origin="Tool";r.target="Linguist";r.operation="probe";r.arguments={std::string("hello"),42LL};
 assert(!bridge(r).ok);
 assert(!bridge.discover({"Linguist","wrong",{"probe"},[](const auto&){return reciprocal::Response{true,"bad"};}}));
 assert(bridge.discover({"Linguist","tool-linguist/1",{"probe"},[](const auto& q){return reciprocal::Response{q.hops==0&&q.arguments.size()==2,"ack"};}}));
 assert(bridge(r).ok);r.hops=0;assert(!bridge(r).ok);r.hops=1;
 r.operation="unknown";assert(!bridge(r).ok);r.operation="probe";
 bridge.disconnect();assert(!bridge(r).ok);
}
