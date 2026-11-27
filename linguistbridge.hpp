#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <functional>
#include <atomic>
#include <chrono>
#include <mutex>
#include <memory>
#include <utility>
namespace reciprocal {
using Value=std::variant<bool,long long,double,std::string>;
using Registry=std::map<std::string,std::vector<std::string>>;
struct Request {
 std::string protocol="tool-linguist/1", id, origin, target, operation;
 std::vector<Value> arguments;
 unsigned hops=1;
 std::chrono::steady_clock::time_point deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
};
struct Response { bool ok; std::string result; };
struct Provider {
 std::string identity, protocol;
 std::vector<std::string> capabilities;
 std::function<Response(const Request&)> invoke;
};
class Bridge {
 public:
 class STACK { public: Registry registry; std::vector<Value> operator()(const Request& r) const {return r.arguments;} };
 class HEAP { public: Registry registry; std::shared_ptr<const Provider> operator()(Provider p) const {return std::make_shared<const Provider>(std::move(p));} };
 class SERVICE { public: Registry registry{{"TYPE",{"local"}},{"MODE",{"opt-in"}},{"CONDITION",{"regulated"}},{"STATE",{"prototype"}}}; };
 class PROTOCOL : public SERVICE {};
 class SOCKET : public SERVICE {};
 class INTERFACE : public SERVICE {};
 Registry associations{{"ARCHITECTURE",{"reciprocal"}},{"DESIGN",{"optional"}},{"MODEL",{"Await","Preprocess","Postprocess","Await"}},{"VERSION",{"1"}},{"SPECIFICATION",{"local adapter prototype"}}};
 STACK stack; HEAP heap; SERVICE service; PROTOCOL protocol; SOCKET socket; INTERFACE interface;
 using Regulator=std::function<bool(const Request&)>;
 private:
 inline static std::atomic<unsigned long long> sequence{0};
 std::string local, remote;
 std::mutex mutex;
 std::shared_ptr<const Provider> peer;
 Regulator regulator;
 public:
 const unsigned long long subprocess=++sequence;
 Bridge(std::string l,std::string r,Regulator policy):local(std::move(l)),remote(std::move(r)),regulator(std::move(policy)){}
 bool discover(Provider p) {
  if(p.identity!=remote||p.protocol!="tool-linguist/1"||!p.invoke)return false;
  std::lock_guard<std::mutex> guard(mutex);peer=heap(std::move(p));return true;
 }
 void disconnect(){std::lock_guard<std::mutex> guard(mutex);peer.reset();}
 Response operator()(Request r) {
  std::shared_ptr<const Provider> p;
  {std::lock_guard<std::mutex> guard(mutex);p=peer;}
  if(!p)return {false,"counterpart unavailable; local fallback required"};
  if(r.protocol!=p->protocol||r.origin!=local||r.target!=remote||r.id.empty()||r.hops!=1||r.arguments.size()>256||r.deadline<=std::chrono::steady_clock::now())return {false,"invalid request"};
  std::size_t bytes=0;
  for(const auto& v:r.arguments)if(auto s=std::get_if<std::string>(&v)){if(s->size()>65536-bytes)return {false,"payload too large"};bytes+=s->size();}
  bool found=false;for(const auto& c:p->capabilities)if(c==r.operation)found=true;
  if(!found)return {false,"unsupported capability"};
  try {
   if(!regulator||!regulator(r))return {false,"authorization denied"};
   --r.hops;
   auto result=p->invoke(r);
   if(r.deadline<=std::chrono::steady_clock::now())return {false,"deadline exceeded after return"};
   return result;
  }catch(...){return {false,"provider or regulator failure"};}
 }
};
}
class LINGUISTBRIDGE : public reciprocal::Bridge {
 public:
 explicit LINGUISTBRIDGE(Regulator policy={}):Bridge("Tool","Linguist",std::move(policy)){}
};
