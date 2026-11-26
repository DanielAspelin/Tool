// Copyright (C) 2026 Daniel Alexander Aspelin
// SPDX-License-Identifier: GPL-3.0-only
#include <iostream>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>

class TOOL {
public:
    using Arguments = std::vector<std::string>;
    using Invocation = std::function<std::string(const Arguments&)>;
private:
    struct Entry { Invocation call; std::string origin; };
    std::map<std::string, Entry> registry;
    std::mutex guard;
    bool running = true;
public:
    TOOL() {
        inject("help", "core", [this](const Arguments&) {
            std::lock_guard<std::mutex> lock(guard);
            std::string result;
            for (const auto& entry : registry) result += entry.first + " [" + entry.second.origin + "]\n";
            return result;
        });
        inject("echo", "core", [](const Arguments& args) {
            std::string result;
            for (const auto& arg : args) result += (result.empty() ? "" : " ") + arg;
            return result;
        });
        inject("exit", "cli", [this](const Arguments&) { running = false; return std::string("Stopped"); });
    }
    bool inject(const std::string& name, const std::string& origin, Invocation call) {
        if (name.empty() || !call) return false;
        std::lock_guard<std::mutex> lock(guard);
        return registry.emplace(name, Entry{std::move(call), origin}).second;
    }
    std::string invoke(const std::string& name, const Arguments& args) {
        Invocation call;
        {
            std::lock_guard<std::mutex> lock(guard);
            auto it = registry.find(name);
            if (it == registry.end()) return "Unknown invocation";
            call = it->second.call;
        }
        return call(args);
    }
    bool active() const { return running; }
    void roundtrip() {
        std::string line, name, arg;
        if (!std::getline(std::cin, line)) { running = false; return; }
        std::istringstream input(line);
        if (!(input >> name)) return;
        Arguments args;
        for (; input >> arg;) args.push_back(arg);
        try { std::cout << invoke(name, args) << std::endl; }
        catch (const std::exception& e) { std::cerr << e.what() << std::endl; }
    }
    int run() {
        while (active()) roundtrip();
        return 0;
    }
};
class REGISTRATION {
    std::map<std::string, std::map<std::string, std::string>> records;
public:
    void add(const std::string& method, const std::string& key, const std::string& value) {
        records[method][key] = value;
    }
};

class REQUEST {
public:
    std::string method;
    TOOL::Arguments arguments;
};

class RESPONSE {
public:
    std::string value;
};

class DISPATCHER {
    TOOL& tool;
    REGISTRATION registry;
public:
    explicit DISPATCHER(TOOL& runtime): tool(runtime) { registry.add("dispatch","target","TOOL"); }
    RESPONSE dispatch(const REQUEST& request) {
        return {tool.invoke(request.method, request.arguments)};
    }
};

class SERVER {
    DISPATCHER& dispatcher;
    REGISTRATION registry;
public:
    explicit SERVER(DISPATCHER& d, const std::string& name): dispatcher(d) {
        registry.add("receive","interface",name);
    }
    RESPONSE receive(const REQUEST& request) { return dispatcher.dispatch(request); }
};

class CLIENT {
    SERVER& server;
    REGISTRATION registry;
public:
    explicit CLIENT(SERVER& s, const std::string& name): server(s) {
        registry.add("send","interface",name);
    }
    RESPONSE send(const REQUEST& request) { return server.receive(request); }
};

class APISERVER : public SERVER {
public:
    explicit APISERVER(DISPATCHER& d): SERVER(d,"API") {}
};
class APICLIENT : public CLIENT {
public:
    explicit APICLIENT(APISERVER& s): CLIENT(s,"API") {}
};
class API {
    REGISTRATION registry;
public:
    APISERVER server;
    APICLIENT client;
    explicit API(DISPATCHER& d): server(d), client(server) {
        registry.add("invoke","route","APICLIENT -> APISERVER");
    }
    RESPONSE invoke(const REQUEST& r) { return client.send(r); }
};

class CLISERVER : public SERVER {
public:
    explicit CLISERVER(DISPATCHER& d): SERVER(d,"CLI") {}
};
class CLICLIENT : public CLIENT {
public:
    explicit CLICLIENT(CLISERVER& s): CLIENT(s,"CLI") {}
};
class CLI {
    REGISTRATION registry;
public:
    CLISERVER server;
    CLICLIENT client;
    explicit CLI(DISPATCHER& d): server(d), client(server) {
        registry.add("invoke","route","CLICLIENT -> CLISERVER");
    }
    RESPONSE invoke(const REQUEST& r) { return client.send(r); }
};

class MCPSERVER : public SERVER {
public:
    explicit MCPSERVER(DISPATCHER& d): SERVER(d,"MCP") {}
};
class MCPCLIENT : public CLIENT {
public:
    explicit MCPCLIENT(MCPSERVER& s): CLIENT(s,"MCP") {}
};
class MCP {
    REGISTRATION registry;
public:
    MCPSERVER server;
    MCPCLIENT client;
    explicit MCP(DISPATCHER& d): server(d), client(server) {
        registry.add("invoke","route","MCPCLIENT -> MCPSERVER");
    }
    RESPONSE invoke(const REQUEST& r) { return client.send(r); }
};

class SSHSERVER : public SERVER {
public:
    explicit SSHSERVER(DISPATCHER& d): SERVER(d,"SSH") {}
};
class SSHCLIENT : public CLIENT {
public:
    explicit SSHCLIENT(SSHSERVER& s): CLIENT(s,"SSH") {}
};
class SSH {
    REGISTRATION registry;
public:
    SSHSERVER server;
    SSHCLIENT client;
    explicit SSH(DISPATCHER& d): server(d), client(server) {
        registry.add("invoke","route","SSHCLIENT -> SSHSERVER");
    }
    RESPONSE invoke(const REQUEST& r) { return client.send(r); }
};

class URI {
    REGISTRATION registry;
public:
    URI() { registry.add("validate","kind","scheme"); }
    bool validate(const std::string& value) const {
        auto p = value.find(':');
        if (p == std::string::npos || p == 0) return false;
        const std::string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if (letters.find(value[0]) == std::string::npos) return false;
        return value.substr(0,p).find_first_not_of(letters+"0123456789+.-") == std::string::npos
            && value.find_first_of(" \t\r\n") == std::string::npos;
    }
};

class URL {
    URI& uri;
    REGISTRATION registry;
public:
    explicit URL(URI& u): uri(u) { registry.add("validate","kind","network endpoint"); }
    bool validate(const std::string& value) const {
        if (!uri.validate(value)) return false;
        auto p = value.find("://");
        if (p == std::string::npos) return false;
        auto start = p+3;
        auto end = value.find_first_of("/?#",start);
        auto authority = value.substr(start,end-start);
        return !authority.empty() && authority.find('@') == std::string::npos;
    }
};

class CONFIGURATION {
    URI& uri;
    URL& url;
    REGISTRATION registry;
    std::map<std::string,std::map<std::string,std::string>> settings;
    std::mutex guard;
public:
    CONFIGURATION(URI& u, URL& l): uri(u), url(l) {
        for (const auto* name : {"api","cli","mcp","ssh"}) {
            settings[name]["mode"]="hybrid";
            settings[name]["endpoint"]="";
        }
        registry.add("invoke","scope","session");
    }
    std::string invoke(const TOOL::Arguments& a) {
        std::lock_guard<std::mutex> lock(guard);
        if (a.empty() || a[0]=="list") {
            std::string out;
            for (const auto& group: settings)
                for (const auto& item: group.second)
                    out += group.first+"."+item.first+"="+item.second+"\n";
            return out;
        }
        if (a.size()<3) return "Usage: configure get|set interface key [value]";
        auto g=settings.find(a[1]);
        if (g==settings.end()) return "Unknown interface";
        auto k=g->second.find(a[2]);
        if (k==g->second.end()) return "Unknown key";
        if (a[0]=="get" && a.size()==3) return k->second;
        if (a[0]!="set" || a.size()!=4) return "Invalid configuration command";
        if (a[2]=="mode" && a[3]!="server" && a[3]!="client" && a[3]!="hybrid") return "Invalid mode";
        if (a[2]=="endpoint" && !url.validate(a[3])) return "Invalid endpoint";
        k->second=a[3];
        return "Configured for this session; transport activation not implemented";
    }
};

class HYBRID {
    TOOL tool;
    URI uri;
    URL url;
    CONFIGURATION configuration;
    DISPATCHER dispatcher;
    API api;
    CLI cli;
    MCP mcp;
    SSH ssh;
    REGISTRATION registry;
public:
    HYBRID(): url(uri), configuration(uri,url), dispatcher(tool), api(dispatcher), cli(dispatcher), mcp(dispatcher), ssh(dispatcher) {
        registry.add("route","API","local");
        registry.add("route","CLI","local");
        registry.add("route","MCP","local");
        registry.add("route","SSH","local");
        tool.inject("configure","configuration",[this](const TOOL::Arguments& a){return configuration.invoke(a);});
        tool.inject("uri","validation",[this](const TOOL::Arguments& a){return a.size()==1 && uri.validate(a[0]) ? "Valid scheme syntax" : "Invalid URI";});
        tool.inject("url","validation",[this](const TOOL::Arguments& a){return a.size()==1 && url.validate(a[0]) ? "Valid endpoint shape" : "Invalid URL";});
        tool.inject("api","hybrid",[this](const TOOL::Arguments& a){return route(api,a);});
        tool.inject("cli","hybrid",[this](const TOOL::Arguments& a){return route(cli,a);});
        tool.inject("mcp","hybrid",[this](const TOOL::Arguments& a){return route(mcp,a);});
        tool.inject("ssh","hybrid",[this](const TOOL::Arguments& a){return route(ssh,a);});
    }
    template<class INTERFACE>
    std::string route(INTERFACE& interface, const TOOL::Arguments& args) {
        if (args.empty()) return "Missing invocation";
        return interface.invoke({args.front(), TOOL::Arguments(args.begin()+1,args.end())}).value;
    }
    int run(int argc, char** argv) {
        if (argc<=1) return tool.run();
        TOOL::Arguments args;
        for (int i=2;i<argc;++i) args.emplace_back(argv[i]);
        std::cout << tool.invoke(argv[1],args) << std::endl;
        return 0;
    }
};

int main(int argc, char** argv) { return HYBRID{}.run(argc,argv); }
