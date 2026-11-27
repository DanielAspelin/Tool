#pragma once
#include "linguistbridge.hpp"
class TOOLCORE {
 public:
 using Request=reciprocal::Request;
 using Response=reciprocal::Response;
 using Registry=reciprocal::Registry;
 using STACK=reciprocal::Bridge::STACK;
 using HEAP=reciprocal::Bridge::HEAP;
 Registry associations{{"ARCHITECTURE",{"independent-core"}},{"DESIGN",{"optional-reciprocity"}},{"MODEL",{"Await","Preprocess","Postprocess","Await"}},{"VERSION",{"0.1"}},{"SPECIFICATION",{"local adapter"}}};
 STACK stack; HEAP heap;
 private:
 LINGUISTBRIDGE bridge;
 public:
 explicit TOOLCORE(reciprocal::Bridge::Regulator policy={}):bridge(std::move(policy)){}
 bool discover(reciprocal::Provider provider){return bridge.discover(std::move(provider));}
 void disconnect(){bridge.disconnect();}
 Response operator()(Request request){return bridge(std::move(request));}
 unsigned long long subprocess() const{return bridge.subprocess;}
};
