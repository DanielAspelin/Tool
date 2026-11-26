// Copyright (C) 2026 Daniel Alexander Aspelin
// SPDX-License-Identifier: GPL-3.0-only
#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <future>
#include <utility>
#include <exception>
#include <stdexcept>
#include <memory>
#include <set>
#include <queue>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <cstdio>
#include <thread>
#include <variant>
#include <cstdint>
#include <condition_variable>
#include <limits>
#if defined(__unix__) && !defined(__wasi__)
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class TIMESTAMP {
    std::map<std::string,std::map<std::string,std::string>> registry;
public:
    struct Snapshot {
        std::chrono::system_clock::time_point utc;
        std::chrono::steady_clock::time_point monotonic;
    };
    TIMESTAMP(){registry["capture"]["source"]="system and steady clocks";}
    Snapshot capture() const {
        return {std::chrono::system_clock::now(),std::chrono::steady_clock::now()};
    }
    std::string format(const Snapshot& s) const {
        return std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(s.utc.time_since_epoch()).count());
    }
    long long elapsed(const Snapshot& s) const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-s.monotonic).count();
    }
};
class IPIFY {
    TIMESTAMP& clock;
    std::mutex guard;
    std::future<void> worker;
    std::map<std::string,std::map<std::string,std::string>> registry;
    std::string address, observed;
    bool enabled=false;
    void refresh(){
        std::string value;
#if defined(__unix__) && !defined(__wasi__)
        if(auto* pipe=popen("curl -q -fsS --proto '=https' --connect-timeout 2 --max-time 4 --max-filesize 128 https://api64.ipify.org 2>/dev/null","r")){
            char buffer[128]={};
            if(fgets(buffer,sizeof(buffer),pipe))value=buffer;
            if(pclose(pipe)!=0)value.clear();
        }
        unsigned char bytes[16];
        if(inet_pton(AF_INET,value.c_str(),bytes)!=1 && inet_pton(AF_INET6,value.c_str(),bytes)!=1)value.clear();
#endif
        std::lock_guard<std::mutex> lock(guard);
        if(enabled){address=value;observed=clock.format(clock.capture());}
    }
public:
    explicit IPIFY(TIMESTAMP& t):clock(t){registry["refresh"]["endpoint"]="https://api64.ipify.org";}
    ~IPIFY(){if(worker.valid())worker.wait();}
    std::string snapshot(){
        std::lock_guard<std::mutex> lock(guard);
        return enabled&&!address.empty()?address+" observed "+observed:"unavailable";
    }
    std::string invoke(const std::vector<std::string>& a){
        if(a.empty()||a[0]=="status")return snapshot();
        std::lock_guard<std::mutex> lock(guard);
        if(a[0]=="disable"){enabled=false;address.clear();return "IPIFY disabled";}
        if(a[0]!="enable"&&a[0]!="refresh")return "Usage: ipify enable|refresh|disable|status";
        if(worker.valid()&&worker.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return "Refresh pending";
        enabled=true;
        worker=std::async(std::launch::async,[this]{refresh();});
        return "Optional IP refresh scheduled";
    }
};
class DUMP {
    TIMESTAMP& clock;
    std::mutex guard;
    std::map<std::string,std::map<std::string,std::string>> registry;
    std::atomic<unsigned long long> sequence{0};
public:
    explicit DUMP(TIMESTAMP& t):clock(t){registry["store"]["location"]="execution working directory";}
    bool store(const TIMESTAMP::Snapshot& stamp,const std::string& method,const std::string& reason) noexcept {
        try {
            std::lock_guard<std::mutex> lock(guard);
            std::string safe;
            for(unsigned char c:method.substr(0,128))safe+=(c>=32&&c<127)?char(c):'?';
            std::string row=clock.format(stamp)+" "+std::to_string(++sequence)+" "+reason+" "+safe+"\n";
#if defined(__unix__) && !defined(__wasi__)
            int fd=open("tool.dump",O_WRONLY|O_CREAT|O_APPEND|O_NOFOLLOW|O_NONBLOCK,0600);
            if(fd<0)return false;
            struct stat st{};
            bool ok=fstat(fd,&st)==0&&S_ISREG(st.st_mode)&&st.st_uid==geteuid()&&(st.st_mode&077)==0;
            if(ok)ok=write(fd,row.data(),row.size())==static_cast<ssize_t>(row.size());
            close(fd);return ok;
#else
            return false;
#endif
        }catch(...){return false;}
    }
    std::string location() const {return (std::filesystem::current_path()/"tool.dump").string();}
};
class SIGNAL {
    std::mutex guard;
    std::map<std::string,std::map<std::string,unsigned long long>> registry;
public:
    void emit(const std::string& target,const std::string& event){
        std::lock_guard<std::mutex> lock(guard);
        ++registry[target][event];
    }
    std::string status(){
        std::lock_guard<std::mutex> lock(guard);
        std::string out;
        for(const auto& t:registry)for(const auto& e:t.second)out+=t.first+"/"+e.first+" "+std::to_string(e.second)+"\n";
        return out;
    }
};
class TRIGGER {
    SIGNAL& signal;
    std::mutex guard;
    std::map<std::string,std::map<std::string,std::string>> registry;
public:
    explicit TRIGGER(SIGNAL& s):signal(s){registry["fire"]["policy"]="observe invocation";}
    void fire(const std::string& target){signal.emit(target,"trigger");}
    void complete(const std::string& target,bool success){signal.emit(target,success?"returned":"exception");}
};
class CPU {
    std::map<std::string,std::map<std::string,std::string>> registry{{"capacity",{{"source","hardware concurrency"}}}};
public:
    unsigned capacity() const {return std::max(1u,std::thread::hardware_concurrency());}
};
class MEMORY {
    std::map<std::string,std::map<std::string,std::string>> registry{{"accept",{{"policy","bounded input"}}}};
public:
    bool accept(const std::string& name,const std::vector<std::string>& args) const {
        if(name.size()>256||args.size()>256)return false;
        size_t remaining=65536;
        for(const auto& a:args){if(a.size()>remaining)return false;remaining-=a.size();}
        return true;
    }
};
class THERMAL {
    std::map<std::string,std::map<std::string,std::string>> registry{{"observe",{{"source","Linux thermal zones"}}}};
public:
    double observe() const {
        double peak=-1;
#if defined(__linux__) && !defined(__wasi__)
        std::error_code ec;
        for(const auto& entry:std::filesystem::directory_iterator("/sys/class/thermal",ec)){
            if(entry.path().filename().string().rfind("thermal_zone",0)!=0)continue;
            std::ifstream in(entry.path()/"temp");double raw;
            if(in>>raw && raw>=0 && raw<=150000)peak=std::max(peak,raw/1000.0);
        }
#endif
        return peak;
    }
};
class CAPACITY {
    CPU& cpu;
    std::map<std::string,std::map<std::string,std::string>> registry{{"limit",{{"policy","bounded concurrency"}}}};
public:
    explicit CAPACITY(CPU& c):cpu(c){}
    unsigned limit() const {return std::min(8u,std::max(1u,cpu.capacity()/2));}
};
class PRESSURE {
    std::map<std::string,std::map<std::string,std::string>> registry{{"observe",{{"source","Linux PSI"}}}};
public:
    double observe() const {
#if defined(__linux__) && !defined(__wasi__)
        std::ifstream in("/proc/pressure/memory");std::string kind,field;
        if(in>>kind>>field && field.rfind("avg10=",0)==0){try{return std::stod(field.substr(6));}catch(...){}}
#endif
        return -1;
    }
};
class CONFLICT {
    std::map<std::string,std::map<std::string,std::string>> registry{{"enter",{{"policy","exclusive named operation"}}}};
    std::mutex guard;
    std::set<std::string> active;
public:
    bool enter(const std::string& name){std::lock_guard<std::mutex> lock(guard);return active.insert(name).second;}
    void leave(const std::string& name){std::lock_guard<std::mutex> lock(guard);active.erase(name);}
};
class THROTTLE {
    THERMAL& thermal; PRESSURE& pressure; CAPACITY& capacity;
    std::map<std::string,std::map<std::string,std::string>> registry{{"limit",{{"policy","conservative adaptive admission"}}}};
public:
    THROTTLE(THERMAL& t,PRESSURE& p,CAPACITY& c):thermal(t),pressure(p),capacity(c){}
    unsigned limit() const {
        double t=thermal.observe(),p=pressure.observe();
        if(t>=90)return 0;
        if(t<0||p<0||t>=80||p>=10)return 1;
        return capacity.limit();
    }
};
class GUARD {
    MEMORY& memory;
    std::map<std::string,std::map<std::string,std::string>> registry{{"accept",{{"policy","input and depth limits"}}}};
public:
    explicit GUARD(MEMORY& m):memory(m){}
    bool accept(const std::string& n,const std::vector<std::string>& a,unsigned depth) const {return depth<16&&memory.accept(n,a);}
};
class QUEUE {
    std::map<std::string,std::map<std::string,std::string>> registry{{"submit",{{"policy","bounded FIFO"}}}};
    std::mutex guard;std::queue<std::function<void()>> jobs;
public:
    bool submit(std::function<void()> job){std::lock_guard<std::mutex> lock(guard);if(jobs.size()>=64)return false;jobs.push(std::move(job));return true;}
    std::function<void()> take(){std::lock_guard<std::mutex> lock(guard);if(jobs.empty())return {};auto job=std::move(jobs.front());jobs.pop();return job;}
};
class WATCHDOG {
    std::map<std::string,std::map<std::string,std::string>> registry{{"expired",{{"policy","observe elapsed deadline"}}}};
public:
    bool expired(const TIMESTAMP::Snapshot& s) const {return std::chrono::steady_clock::now()-s.monotonic>std::chrono::seconds(30);}
};
class CLEANUP {
    std::map<std::string,std::map<std::string,std::string>> registry{{"release",{{"policy","scope exit"}}}};
public:
    struct Lease {std::function<void()> release;explicit Lease(std::function<void()> f):release(std::move(f)){} Lease(const Lease&)=delete;~Lease(){if(release)release();}};
};
class SCHEDULER {
    THROTTLE& throttle;
    std::map<std::string,std::map<std::string,std::string>> registry{{"enter",{{"policy","bounded admission"}}}};
    std::mutex guard;unsigned active=0;
public:
    explicit SCHEDULER(THROTTLE& t):throttle(t){}
    bool enter(){auto limit=throttle.limit();std::lock_guard<std::mutex> lock(guard);if(active>=limit)return false;++active;return true;}
    void leave(){std::lock_guard<std::mutex> lock(guard);if(active)--active;}
};
class TOOL {
public:
    using Arguments = std::vector<std::string>;
    using Invocation = std::function<std::string(const Arguments&)>;
private:
    struct Entry { Invocation call; std::string origin; };
    std::map<std::string, Entry> registry;
    std::mutex guard;
    std::atomic<bool> running{true};
    CPU cpu;
    MEMORY memory;
    THERMAL thermal;
    CAPACITY capacity{cpu};
    PRESSURE pressure;
    CONFLICT conflict;
    THROTTLE throttle{thermal,pressure,capacity};
    GUARD boundary{memory};
    QUEUE queue;
    WATCHDOG watchdog;
    CLEANUP cleanup;
    SCHEDULER scheduler{throttle};
    TIMESTAMP timestamp;
    IPIFY ipify{timestamp};
    DUMP dump{timestamp};
    SIGNAL signal;
    TRIGGER trigger{signal};
    std::mutex timingGuard;
    std::map<std::string,std::pair<std::string,long long>> timings;
    void recordTiming(const std::string& name,const TIMESTAMP::Snapshot& snapshot){
        std::lock_guard<std::mutex> lock(timingGuard);
        timings[name]={timestamp.format(snapshot),timestamp.elapsed(snapshot)};
    }
    std::mutex lifecycleGuard;
    std::map<std::string,std::pair<std::string,Arguments>> volatileCalls;
    const std::string invocationFile="tool.invocations";
    using Plans=std::map<std::string,std::pair<std::string,Arguments>>;
    Plans loadPlans(){
        Plans plans;
        std::ifstream in(invocationFile);
        std::string line,id,name,arg;
        for(;std::getline(in,line);){
            std::istringstream row(line);
            if(!(row>>std::quoted(id)>>std::quoted(name)))throw std::runtime_error("Invalid invocation record");
            Arguments args;
            for(;row>>std::quoted(arg);)args.push_back(arg);
            if(!row.eof())throw std::runtime_error("Invalid invocation arguments");
            plans[id]={name,args};
        }
        return plans;
    }
    void savePlans(const Plans& plans){
        std::ofstream out(invocationFile,std::ios::trunc);
        if(!out)throw std::runtime_error("Cannot open invocation store");
        for(const auto& p:plans){
            out<<std::quoted(p.first)<<" "<<std::quoted(p.second.first);
            for(const auto& arg:p.second.second)out<<" "<<std::quoted(arg);
            out<<"\n";
        }
        out.close();
        if(!out)throw std::runtime_error("Cannot save invocation store");
    }
    bool validPlan(const std::string& name,const Arguments& args){
        if(name=="invoke"||name=="exit"||name.find_first_of("\r\n")!=std::string::npos)return false;
        for(const auto& a:args)if(a.find_first_of("\r\n")!=std::string::npos)return false;
        std::lock_guard<std::mutex> lock(guard);
        return registry.count(name)!=0;
    }
    std::string lifecycle(const Arguments& a){
        if(a.size()>=2&&a[0]=="direct")return invoke(a[1],Arguments(a.begin()+2,a.end()));
        std::unique_lock<std::mutex> lock(lifecycleGuard);
        if(a.size()>=4&&a[0]=="define"){
            if(a[1]!="volatile"&&a[1]!="persistent")return "Invalid lifecycle";
            Arguments args(a.begin()+4,a.end());
            if(a[2].empty()||a[2].find_first_of("\r\n")!=std::string::npos||!validPlan(a[3],args))return "Invalid invocation plan";
            auto plans=loadPlans();
            if(plans.count(a[2])||volatileCalls.count(a[2]))return "Invocation ID exists";
            if(a[1]=="volatile")volatileCalls[a[2]]={a[3],args};
            else {plans[a[2]]={a[3],args};savePlans(plans);}
            return "Invocation defined";
        }
        if(a.size()==3&&a[0]=="transition"){
            auto plans=loadPlans();
            if(a[2]=="persistent"&&volatileCalls.count(a[1])&&!plans.count(a[1])){
                plans[a[1]]=volatileCalls.at(a[1]);savePlans(plans);volatileCalls.erase(a[1]);return "Persistent";
            }
            if(a[2]=="volatile"&&plans.count(a[1])&&!volatileCalls.count(a[1])){
                auto plan=plans.at(a[1]);plans.erase(a[1]);savePlans(plans);volatileCalls[a[1]]=plan;return "Volatile";
            }
            return "Invalid transition";
        }
        if(a.size()==2&&a[0]=="run"){
            auto plans=loadPlans();
            if(!volatileCalls.count(a[1])&&!plans.count(a[1]))return "Unknown invocation ID";
            auto plan=volatileCalls.count(a[1])?volatileCalls.at(a[1]):plans.at(a[1]);
            lock.unlock();
            if(!validPlan(plan.first,plan.second))return "Invocation unavailable";
            return invoke(plan.first,plan.second);
        }
        if(a.size()==1&&a[0]=="list"){
            std::string out;
            for(const auto& p:volatileCalls)out+=p.first+" volatile\n";
            for(const auto& p:loadPlans())out+=p.first+" persistent\n";
            return out;
        }
        return "Usage: invoke direct method ... | define volatile|persistent id method ... | transition id volatile|persistent | run id | list";
    }
public:
    TOOL() {
        inject("regulation","core",[this](const Arguments&){return std::string("CPU threads: ")+std::to_string(cpu.capacity())+"; thermal C (-1 unknown): "+std::to_string(thermal.observe())+"; memory PSI (-1 unknown): "+std::to_string(pressure.observe())+"; admission limit: "+std::to_string(throttle.limit());});
        inject("ipify","core",[this](const Arguments& a){return ipify.invoke(a);});
        inject("dump","core",[this](const Arguments& a){if(a.empty()||a[0]=="location")return dump.location();if(a[0]=="unacknowledged")return dump.store(timestamp.capture(),"manual","unacknowledged")?std::string("Stored"):std::string("Dump unavailable");return std::string("Usage: dump location|unacknowledged");});
        inject("invoke","core",[this](const Arguments& a){return lifecycle(a);});
        inject("signal","runtime",[this](const Arguments&){return signal.status();});
        inject("trigger","runtime",[this](const Arguments& a){if(a.size()!=1)return std::string("Usage: trigger name");trigger.fire(a[0]);return std::string("Signal emitted");});
        inject("timestamp","core",[this](const Arguments& a){if(a.empty()||a[0]=="now")return timestamp.format(timestamp.capture());if(a.size()==2&&a[0]=="last"){std::lock_guard<std::mutex> lock(timingGuard);auto it=timings.find(a[1]);if(it==timings.end())return std::string("No timing record");return it->second.first+" epoch ns; "+std::to_string(it->second.second)+" elapsed ns";}return std::string("Usage: timestamp now | timestamp last method");});
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
        const auto snapshot=timestamp.capture();
        static thread_local unsigned depth=0;
        if(!boundary.accept(name,args,depth)){dump.store(snapshot,name,"boundary");return "Invocation boundary exceeded";}
        const bool outer=depth==0;
        if(outer&&!scheduler.enter()){dump.store(snapshot,name,"deferred");return "Capacity unavailable; retry later";}
        CLEANUP::Lease admission([this,outer]{if(outer)scheduler.leave();});
        ++depth;
        CLEANUP::Lease nesting([&]{--depth;});
        const bool exclusive=name=="configure"||name=="invoke"||name=="ipify";
        if(exclusive&&!conflict.enter(name))return "Conflicting operation; retry later";
        CLEANUP::Lease ownership([this,&name,exclusive]{if(exclusive)conflict.leave(name);});
        const auto networkSnapshot=ipify.snapshot();
        (void)networkSnapshot;
        trigger.fire(name);
        Invocation call;
        {
            std::lock_guard<std::mutex> lock(guard);
            auto it = registry.find(name);
            if (it == registry.end()) {signal.emit(name,"unresolved");if(!dump.store(snapshot,name,"unresolved"))std::cerr<<"Dump unavailable\n";recordTiming(name,snapshot);return "Unknown invocation";}
            call = it->second.call;
        }
        try {auto result=call(args);if(watchdog.expired(snapshot))dump.store(snapshot,name,"deadline-exceeded");trigger.complete(name,true);recordTiming(name,snapshot);return result;}
        catch(...){if(!dump.store(snapshot,name,"exception"))std::cerr<<"Dump unavailable\n";trigger.complete(name,false);recordTiming(name,snapshot);throw;}
    }
    SIGNAL& signals(){return signal;}
    bool active() const { return running.load(); }
    void roundtrip() {
        std::string line, name, arg;
        if (!std::getline(std::cin, line)) { running = false; return; }
        std::istringstream input(line);
        if (!(input >> name)) return;
        Arguments args;
        for (; input >> std::quoted(arg);) args.push_back(arg);
        if(!input.eof() && input.fail()){std::cerr << "Invalid arguments" << std::endl;return;}
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
    mutable std::mutex guard;
public:
    void add(const std::string& method, const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(guard);
        records[method][key] = value;
    }
};

class VALUE {
    REGISTRATION registry;
public:
    using Data=std::variant<std::monostate,bool,std::int64_t,double,std::string>;
private:
    Data data;
public:
    explicit VALUE(Data v={}):data(std::move(v)){registry.add("get","policy","typed value");}
    const Data& get() const{return data;}
};
class PARAMETER {
    REGISTRATION registry;
    std::string name;
    bool required;
    std::size_t type;
public:
    PARAMETER(std::string n,std::size_t t,bool r=true):name(std::move(n)),required(r),type(t){
        if(name.empty()||type>4)throw std::invalid_argument("Invalid parameter");
        registry.add("accept","name",name);
    }
    const std::string& identity() const{return name;}
    bool accept(const VALUE& v) const{return (!required&&v.get().index()==0)||v.get().index()==type;}
};
class REFERENCE {
    REGISTRATION registry;
    std::string identity;
    std::weak_ptr<const VALUE> target;
public:
    REFERENCE(std::string id,const std::shared_ptr<const VALUE>& v):identity(std::move(id)),target(v){registry.add("resolve","identity",identity);}
    std::shared_ptr<const VALUE> resolve() const{return target.lock();}
};
class ARGUMENT {
    REGISTRATION registry;
    std::shared_ptr<const VALUE> value;
public:
    explicit ARGUMENT(std::shared_ptr<const VALUE> v):value(std::move(v)){registry.add("bind","policy","validated immutable value");}
    bool bind(const PARAMETER& p) const{return value&&p.accept(*value);}
    std::shared_ptr<const VALUE> get() const{return value;}
};
class VARIABLE {
    REGISTRATION registry;
    mutable std::mutex guard;
    std::shared_ptr<const VALUE> value;
public:
    explicit VARIABLE(std::shared_ptr<const VALUE> v):value(std::move(v)){registry.add("assign","policy","synchronized immutable replacement");}
    void assign(std::shared_ptr<const VALUE> v){std::lock_guard<std::mutex> lock(guard);value=std::move(v);}
    std::shared_ptr<const VALUE> read() const {std::lock_guard<std::mutex> lock(guard);return value;}
};
class RETURN {
    REGISTRATION registry;
public:
    const bool success;
    const std::string result;
    RETURN(bool ok,std::string r):success(ok),result(std::move(r)){registry.add("result","policy","explicit outcome");}
};
class FUNCTION {
    REGISTRATION registry;
    TOOL::Invocation callable;
public:
    explicit FUNCTION(TOOL::Invocation f):callable(std::move(f)){registry.add("execute","policy","registered callable");}
    RETURN execute(const TOOL::Arguments& args) const {
        if(!callable)return RETURN(false,"Unbound function");
        return RETURN(true,callable(args));
    }
};
class CALL {
    REGISTRATION registry;
    TOOL& tool;
public:
    explicit CALL(TOOL& t):tool(t){registry.add("invoke","target","TOOL");}
    RETURN invoke(const std::string& name,const TOOL::Arguments& args){
        return RETURN(true,tool.invoke(name,args));
    }
};
class CLASS {
    REGISTRATION registry;
    const std::string name;
public:
    explicit CLASS(std::string n):name(std::move(n)){registry.add("identity","name",name);}
    const std::string& identity() const{return name;}
};
class OBJECT {
    REGISTRATION registry;
    std::shared_ptr<const CLASS> definition;
    std::mutex guard;
    std::map<std::string,std::shared_ptr<const VALUE>> fields;
public:
    explicit OBJECT(std::shared_ptr<const CLASS> c):definition(std::move(c)){registry.add("set","policy","bounded object fields");}
    bool set(const std::string& key,std::shared_ptr<const VALUE> v){
        std::lock_guard<std::mutex> lock(guard);
        if(key.empty()||key.size()>256||(!fields.count(key)&&fields.size()>=64))return false;
        fields[key]=std::move(v);return true;
    }
    std::shared_ptr<const VALUE> get(const std::string& key){
        std::lock_guard<std::mutex> lock(guard);auto it=fields.find(key);return it==fields.end()?nullptr:it->second;
    }
};
class INSTANCE {
    REGISTRATION registry;
    const std::string identity;
    std::shared_ptr<OBJECT> object;
public:
    INSTANCE(std::string id,std::shared_ptr<OBJECT> o):identity(std::move(id)),object(std::move(o)){registry.add("resolve","identity",identity);}
    std::shared_ptr<OBJECT> resolve() const{return object;}
    const std::string& id() const{return identity;}
};
class ARRAY {
    REGISTRATION registry;
    mutable std::mutex guard;
    std::vector<std::shared_ptr<const VALUE>> values;
public:
    ARRAY(){registry.add("append","policy","bounded indexed values");registry.add("get","policy","checked index");}
    bool append(std::shared_ptr<const VALUE> value){
        std::lock_guard<std::mutex> lock(guard);
        if(!value||values.size()>=256)return false;
        values.push_back(std::move(value));return true;
    }
    std::shared_ptr<const VALUE> get(std::size_t index) const{
        std::lock_guard<std::mutex> lock(guard);return index<values.size()?values[index]:nullptr;
    }
    std::size_t size() const{std::lock_guard<std::mutex> lock(guard);return values.size();}
};
class BOOLEAN {
    REGISTRATION registry;
    const bool value;
public:
    explicit BOOLEAN(bool v):value(v){registry.add("evaluate","policy","immutable truth value");}
    bool evaluate() const{return value;}
    bool negate() const{return !value;}
    bool conjunction(const BOOLEAN& other) const{return value&&other.value;}
    bool disjunction(const BOOLEAN& other) const{return value||other.value;}
    bool exclusive(const BOOLEAN& other) const{return value!=other.value;}
    std::shared_ptr<const VALUE> asValue() const{return std::make_shared<const VALUE>(VALUE::Data{value});}
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
        for (const auto* name : {"api","cli","mcp","ssh","rdp","vnc","rpc"}) {
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

class SESSION {
    REGISTRATION registry;
    std::string identity;
    std::string parent;
    unsigned depth;
    std::map<std::string,std::unique_ptr<SESSION>> children;
    std::mutex guard;
public:
    explicit SESSION(std::string id="root", std::string origin="", unsigned level=0)
        : identity(std::move(id)), parent(std::move(origin)), depth(level) {
        registry.add("trigger","identity",identity);
        registry.add("trigger","parent",parent);
    }
    std::string trigger() {
        std::lock_guard<std::mutex> lock(guard);
        if (depth>=3 || children.size()>=4) return "Session limit reached";
        auto id=identity+"."+std::to_string(children.size()+1);
        children.emplace(id,std::make_unique<SESSION>(id,identity,depth+1));
        return id;
    }
    std::string trigger(const std::string& target) {
        if (target==identity) return trigger();
        std::lock_guard<std::mutex> lock(guard);
        for (auto& child: children) {
            if (target==child.first || target.rfind(child.first+".",0)==0)
                return child.second->trigger(target);
        }
        return "Unknown session";
    }
    std::string list() {
        std::lock_guard<std::mutex> lock(guard);
        std::string out=identity+" parent="+parent+"\n";
        for (auto& child: children) out+=child.second->list();
        return out;
    }
    std::string invoke(const TOOL::Arguments& args) {
        if (args.empty() || args[0]=="list") return list();
        if (args[0]=="trigger" && args.size()==1) return trigger();
        if (args[0]=="trigger" && args.size()==2) return trigger(args[1]);
        return "Usage: session list | session trigger [id]";
    }
};

class MESH {
    REGISTRATION registry;
    std::map<std::string,std::set<std::string>> edges;
    std::mutex guard;
public:
    MESH(){registry.add("connect","kind","directed");}
    void connect(const std::string& a,const std::string& b){
        std::lock_guard<std::mutex> lock(guard);
        edges[a].insert(b); edges.try_emplace(b);
    }
    auto snapshot(){std::lock_guard<std::mutex> lock(guard);return edges;}
};
class CHAIN {
    MESH& mesh;
    REGISTRATION registry;
public:
    explicit CHAIN(MESH& m):mesh(m){registry.add("connect","kind","sequential");}
    void connect(const TOOL::Arguments& nodes){
        for(std::size_t i=1;i<nodes.size();++i) mesh.connect(nodes[i-1],nodes[i]);
    }
};
class RING {
    CHAIN& chain;
    MESH& mesh;
    REGISTRATION registry;
public:
    RING(CHAIN& c,MESH& m):chain(c),mesh(m){registry.add("connect","kind","circular");}
    void connect(const TOOL::Arguments& nodes){
        chain.connect(nodes);
        if(nodes.size()>1) mesh.connect(nodes.back(),nodes.front());
    }
};
class MATRIX {
    REGISTRATION registry;
    std::map<std::vector<std::string>,std::string> cells;
    std::mutex guard;
public:
    MATRIX(){registry.add("set","kind","multidimensional");}
    void set(const std::vector<std::string>& coordinates,const std::string& value){
        std::lock_guard<std::mutex> lock(guard);cells[coordinates]=value;
    }
    std::string get(const std::vector<std::string>& coordinates){
        std::lock_guard<std::mutex> lock(guard);
        auto it=cells.find(coordinates);return it==cells.end()?"Unassigned":it->second;
    }
};
class GRID {
    MATRIX& matrix;
    REGISTRATION registry;
public:
    explicit GRID(MATRIX& m):matrix(m){registry.add("set","kind","two-dimensional");}
    void set(const std::string& x,const std::string& y,const std::string& value){matrix.set({x,y},value);}
    std::string get(const std::string& x,const std::string& y){return matrix.get({x,y});}
};
class PATH {
    MESH& mesh;
    REGISTRATION registry;
public:
    explicit PATH(MESH& m):mesh(m){registry.add("find","kind","breadth-first");}
    TOOL::Arguments find(const std::string& a,const std::string& b, const std::function<bool(const std::string&,const std::string&)>& allowed = {}){
        auto graph=mesh.snapshot();
        if(!graph.count(a)||!graph.count(b))return {};
        std::queue<TOOL::Arguments> pending;
        std::set<std::string> seen{a};
        pending.push({a});
        for(;!pending.empty();){
            auto path=pending.front();pending.pop();
            if(path.back()==b)return path;
            for(const auto& n:graph[path.back()])if((!allowed || allowed(path.back(),n)) && seen.insert(n).second){
                auto next=path;next.push_back(n);pending.push(std::move(next));
            }
        }
        return {};
    }
};
class ROUTE {
    PATH& path;
    REGISTRATION registry;
public:
    explicit ROUTE(PATH& p):path(p){registry.add("select","kind","shortest-hop");}
    std::string select(const std::string& a,const std::string& b){
        auto nodes=path.find(a,b);std::string out;
        for(const auto& n:nodes)out+=(out.empty()?"":" -> ")+n;
        return out.empty()?"No route":out;
    }
};
class LANE {
    ROUTE& route;
    REGISTRATION registry;
    std::map<std::string,std::pair<std::string,std::string>> lanes;
    std::mutex guard;
public:
    explicit LANE(ROUTE& r):route(r){registry.add("assign","kind","named channel");}
    void assign(const std::string& name,const std::string& a,const std::string& b){
        std::lock_guard<std::mutex> lock(guard);lanes[name]={a,b};
    }
    std::string resolve(const std::string& name){
        std::lock_guard<std::mutex> lock(guard);
        auto it=lanes.find(name);
        return it==lanes.end()?"Unknown lane":route.select(it->second.first,it->second.second);
    }
};

class BRIDGE {
    MESH& mesh;
    ROUTE& route;
    REGISTRATION registry;
    std::mutex guard;
    std::map<std::string,std::pair<std::string,std::string>> bridges;
public:
    BRIDGE(MESH& m, ROUTE& r):mesh(m),route(r){
        registry.add("connect","kind","bidirectional");
        registry.add("resolve","kind","route");
    }
    std::string connect(const std::string& name,const std::string& a,const std::string& b){
        std::lock_guard<std::mutex> lock(guard);
        if(a==b || bridges.count(name)) return "Invalid or duplicate bridge";
        mesh.connect(a,b);
        mesh.connect(b,a);
        bridges.emplace(name,std::make_pair(a,b));
        return "Bridge registered";
    }
    std::string resolve(const std::string& name){
        std::lock_guard<std::mutex> lock(guard);
        auto it=bridges.find(name);
        if(it==bridges.end()) return "Unknown bridge";
        return route.select(it->second.first,it->second.second);
    }
};

class RPC {
    DISPATCHER& dispatcher;
    REGISTRATION registry;
public:
    explicit RPC(DISPATCHER& d):dispatcher(d){
        registry.add("call","transport","local");
        registry.add("call","target","DISPATCHER");
    }
    RESPONSE call(const REQUEST& request){
        return dispatcher.dispatch(request);
    }
    std::string invoke(const TOOL::Arguments& args){
        if(args.empty()) return "Usage: rpc method [arguments...]";
        return call({args.front(),TOOL::Arguments(args.begin()+1,args.end())}).value;
    }
};

class RDPSERVER : public SERVER {
public:
    explicit RDPSERVER(DISPATCHER& d): SERVER(d,"RDP") {}
};
class RDPCLIENT : public CLIENT {
public:
    explicit RDPCLIENT(RDPSERVER& s): CLIENT(s,"RDP") {}
};
class RDP {
    REGISTRATION registry;
public:
    RDPSERVER server;
    RDPCLIENT client;
    explicit RDP(DISPATCHER& d): server(d), client(server) {
        registry.add("invoke","transport","local");
        registry.add("invoke","route","RDPCLIENT -> RDPSERVER");
    }
    RESPONSE invoke(const REQUEST& r) { return client.send(r); }
};

class VNCSERVER : public SERVER {
public:
    explicit VNCSERVER(DISPATCHER& d): SERVER(d,"VNC") {}
};
class VNCCLIENT : public CLIENT {
public:
    explicit VNCCLIENT(VNCSERVER& s): CLIENT(s,"VNC") {}
};
class VNC {
    REGISTRATION registry;
public:
    VNCSERVER server;
    VNCCLIENT client;
    explicit VNC(DISPATCHER& d): server(d), client(server) {
        registry.add("invoke","transport","local");
        registry.add("invoke","route","VNCCLIENT -> VNCSERVER");
    }
    RESPONSE invoke(const REQUEST& r) { return client.send(r); }
};

class AST {
    REGISTRATION registry;
public:
    struct Node {
        std::string kind;
        std::string value;
        std::vector<Node> children;
    };
    AST(){registry.add("parse","grammar","invocation");}
    Node parse(const REQUEST& r) const {
        Node root{"invocation",r.method,{}};
        for(const auto& arg:r.arguments)root.children.push_back({"argument",arg,{}});
        return root;
    }
};
class IR {
    AST& ast;
    REGISTRATION registry;
public:
    explicit IR(AST& a):ast(a){registry.add("lower","representation","procedure");}
    REQUEST lower(const AST::Node& node) const {
        if(node.kind!="invocation" || node.value.empty())throw std::invalid_argument("Invalid invocation AST");
        REQUEST result{node.value,{}};
        for(const auto& child:node.children){
            if(child.kind!="argument" || !child.children.empty())throw std::invalid_argument("Invalid argument AST");
            result.arguments.push_back(child.value);
        }
        return result;
    }
    REQUEST normalize(const REQUEST& r) const {return lower(ast.parse(r));}
};
class CPP {
    IR& ir;
    REGISTRATION registry;
public:
    explicit CPP(IR& i):ir(i){registry.add("prepare","target","C++");}
    REQUEST prepare(const REQUEST& r) const {return ir.normalize(r);}
    std::string invoke(const TOOL::Arguments& args) const {
        if(args.empty() || args[0]=="status")return "CPP: native source preparation; compiler pending";
        if(args[0]!="prepare" || args.size()<2)return "Usage: cpp status | cpp prepare method [arguments...]";
        auto r=prepare({args[1],TOOL::Arguments(args.begin()+2,args.end())});
        return "Prepared C++ invocation: "+r.method+" ("+std::to_string(r.arguments.size())+" arguments)";
    }
};
class WASM {
    CPP& cpp;
    IR& ir;
    REGISTRATION registry;
public:
    WASM(CPP& c,IR& i):cpp(c),ir(i){registry.add("prepare","target","WebAssembly");}
    REQUEST prepare(const REQUEST& r) const {return ir.normalize(cpp.prepare(r));}
    std::string invoke(const TOOL::Arguments& args) const {
        if(args.empty() || args[0]=="status")return "WASM: portable invocation preparation; emitter and runtime pending";
        if(args[0]!="prepare" || args.size()<2)return "Usage: wasm status | wasm prepare method [arguments...]";
        auto r=prepare({args[1],TOOL::Arguments(args.begin()+2,args.end())});
        return "Prepared WASM invocation: "+r.method+" ("+std::to_string(r.arguments.size())+" arguments)";
    }
};
class CODE {
    REGISTRATION registry;
public:
    CODE(){registry.add("validate","scope","source record");}
    bool validate(const std::string& language,const std::string& source) const {
        return (language=="cpp" || language=="wasm") && !source.empty() && source.size()<=1048576;
    }
};
class CODEBASE {
    CODE& code;
    REGISTRATION registry;
    std::map<std::string,std::pair<std::string,std::string>> records;
    std::mutex guard;
public:
    explicit CODEBASE(CODE& c):code(c){registry.add("store","scope","in-memory source records");}
    std::string invoke(const TOOL::Arguments& a){
        std::lock_guard<std::mutex> lock(guard);
        if(a.empty()||a[0]=="list"){
            std::string out;
            for(const auto& item:records)out+=item.first+" "+item.second.first+"\n";
            return out;
        }
        if(a[0]=="get"&&a.size()==2){
            auto it=records.find(a[1]);return it==records.end()?"Unknown record":it->second.second;
        }
        if(a[0]=="put"&&a.size()==4){
            if(a[1].empty()||!code.validate(a[2],a[3]))return "Invalid source record";
            if(records.size()>=64&&!records.count(a[1]))return "Record limit reached";
            records[a[1]]={a[2],a[3]};return "Stored in session";
        }
        return "Usage: codebase list | get name | put name cpp|wasm source";
    }
};
class SANDBOX {
    IR& ir;
    REGISTRATION registry;
public:
    explicit SANDBOX(IR& i):ir(i){registry.add("prepare","policy","no execution");}
    std::string invoke(const TOOL::Arguments& a) const {
        if(a.empty()||a[0]=="status")return "SANDBOX: preparation only; isolation backend unavailable";
        if(a[0]!="prepare"||a.size()<2)return "Usage: sandbox status | prepare method [arguments...]";
        auto r=ir.normalize({a[1],TOOL::Arguments(a.begin()+2,a.end())});
        return "Prepared without execution: "+r.method;
    }
};
class SDK {
    CPP& cpp;
    WASM& wasm;
    SANDBOX& sandbox;
    CODEBASE& codebase;
    REGISTRATION registry;
public:
    SDK(CPP& c,WASM& w,SANDBOX& s,CODEBASE& b):cpp(c),wasm(w),sandbox(s),codebase(b){
        registry.add("invoke","targets","cpp,wasm,sandbox,codebase");
    }
    std::string invoke(const TOOL::Arguments& a){
        if(a.empty()||a[0]=="status")return "SDK: cpp wasm sandbox codebase";
        TOOL::Arguments rest(a.begin()+1,a.end());
        if(a[0]=="cpp")return cpp.invoke(rest);
        if(a[0]=="wasm")return wasm.invoke(rest);
        if(a[0]=="sandbox")return sandbox.invoke(rest);
        if(a[0]=="codebase")return codebase.invoke(rest);
        return "Unknown SDK target";
    }
};
class AES {
    REGISTRATION registry;
public:
    AES(){registry.add("encrypt","standard","FIPS 197");registry.add("decrypt","backend","unavailable");}
    bool validKeyBits(unsigned bits) const {return bits==128||bits==192||bits==256;}
    std::string invoke(const TOOL::Arguments& a) const {
        if(a.empty()||a[0]=="status")return "AES: 128/192/256-bit key profiles; cryptographic backend unavailable";
        return "AES operation unavailable: no cryptographic backend";
    }
};
class GCM {
    AES& aes;
    REGISTRATION registry;
public:
    explicit GCM(AES& a):aes(a){registry.add("seal","standard","NIST SP 800-38D");registry.add("open","policy","authenticate before release");}
    bool validProfile(unsigned keyBits,unsigned nonceBytes,unsigned tagBytes) const {
        return aes.validKeyBits(keyBits)&&nonceBytes==12&&tagBytes==16;
    }
    std::string invoke(const TOOL::Arguments& a) const {
        if(a.empty()||a[0]=="status")return "GCM: AES authenticated encryption profile; 96-bit nonce, 128-bit tag; backend unavailable";
        return "GCM operation unavailable: no cryptographic backend";
    }
};
class RSA {
    REGISTRATION registry;
public:
    RSA(){registry.add("encrypt","scheme","RSA-OAEP");registry.add("sign","scheme","RSA-PSS");}
    bool validKeyBits(unsigned bits) const {return bits>=2048;}
    std::string invoke(const TOOL::Arguments& a) const {
        if(a.empty()||a[0]=="status")return "RSA: OAEP encryption and PSS signature profiles; backend unavailable";
        return "RSA operation unavailable: no cryptographic backend";
    }
};
class SHA {
    REGISTRATION registry;
    const std::map<std::string,unsigned> profiles{
        {"sha224",224},{"sha256",256},{"sha384",384},{"sha512",512},
        {"sha512/224",224},{"sha512/256",256},{"sha3-224",224},
        {"sha3-256",256},{"sha3-384",384},{"sha3-512",512}
    };
public:
    SHA(){registry.add("digest","standard","FIPS 180-4 and FIPS 202");registry.add("digest","backend","unavailable");}
    bool supports(const std::string& name) const {return profiles.count(name)!=0;}
    std::string invoke(const TOOL::Arguments& a) const {
        if(a.empty()||a[0]=="status")return "SHA: SHA-2 and SHA-3 profiles; hashing backend unavailable";
        if(a[0]=="list"){std::string out;for(const auto& p:profiles)out+=p.first+" "+std::to_string(p.second)+" bits\n";return out;}
        return "SHA operation unavailable: no cryptographic backend";
    }
};
class OAEP {
    RSA& rsa;
    SHA& sha;
    REGISTRATION registry;
public:
    OAEP(RSA& r,SHA& h):rsa(r),sha(h){
        registry.add("encrypt","standard","RFC 8017 RSAES-OAEP");
        registry.add("encrypt","hash","SHA-256");
        registry.add("encrypt","mask","MGF1-SHA-256");
    }
    bool validProfile(unsigned bits) const {return rsa.validKeyBits(bits)&&sha.supports("sha256");}
    std::string invoke(const TOOL::Arguments& a) const {
        if(a.empty()||a[0]=="status")return "OAEP: RSA encryption profile, SHA-256, MGF1-SHA-256; backend unavailable";
        return "OAEP operation unavailable: no cryptographic backend";
    }
};
class PSS {
    RSA& rsa;
    SHA& sha;
    REGISTRATION registry;
public:
    PSS(RSA& r,SHA& h):rsa(r),sha(h){
        registry.add("sign","standard","RFC 8017 RSASSA-PSS");
        registry.add("sign","hash","SHA-256");
        registry.add("sign","mask","MGF1-SHA-256");
        registry.add("sign","saltBytes","32");
    }
    bool validProfile(unsigned bits) const {return rsa.validKeyBits(bits)&&sha.supports("sha256");}
    std::string invoke(const TOOL::Arguments& a) const {
        if(a.empty()||a[0]=="status")return "PSS: RSA signature profile, SHA-256, MGF1-SHA-256, 32-byte salt; backend unavailable";
        return "PSS operation unavailable: no cryptographic backend";
    }
};
class BASE64 {
    REGISTRATION registry;
    const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
public:
    BASE64(){registry.add("encode","standard","RFC 4648");registry.add("decode","policy","strict canonical padding");}
    std::string encode(const std::string& input) const {
        std::string out;
        for(std::size_t i=0;i<input.size();i+=3){
            unsigned a=static_cast<unsigned char>(input[i]);
            unsigned b=i+1<input.size()?static_cast<unsigned char>(input[i+1]):0;
            unsigned c=i+2<input.size()?static_cast<unsigned char>(input[i+2]):0;
            unsigned v=(a<<16)|(b<<8)|c;
            out+=alphabet[(v>>18)&63];out+=alphabet[(v>>12)&63];
            out+=i+1<input.size()?alphabet[(v>>6)&63]:'=';
            out+=i+2<input.size()?alphabet[v&63]:'=';
        }
        return out;
    }
    std::string decode(const std::string& input) const {
        if(input.size()%4)throw std::invalid_argument("Invalid Base64 length");
        std::string out;
        for(std::size_t i=0;i<input.size();i+=4){
            unsigned v=0,pad=0;
            for(unsigned j=0;j<4;++j){
                char c=input[i+j];
                if(c=='='){
                    if(j<2||i+4!=input.size())throw std::invalid_argument("Invalid Base64 padding");
                    ++pad;v<<=6;
                }else{
                    auto p=alphabet.find(c);
                    if(pad||p==std::string::npos)throw std::invalid_argument("Invalid Base64 character");
                    v=(v<<6)|static_cast<unsigned>(p);
                }
            }
            if((pad==2&&(v&65535))||(pad==1&&(v&255)))throw std::invalid_argument("Noncanonical Base64");
            out+=static_cast<char>((v>>16)&255);
            if(pad<2)out+=static_cast<char>((v>>8)&255);
            if(pad<1)out+=static_cast<char>(v&255);
        }
        return out;
    }
    std::string invoke(const TOOL::Arguments& a) const {
        if(a.empty()||a[0]=="status")return "BASE64: RFC 4648 standard alphabet, strict padding";
        if(a.size()!=2)return "Usage: base64 encode|decode value";
        try{
            if(a[0]=="encode")return encode(a[1]);
            if(a[0]=="decode")return decode(a[1]);
        }catch(const std::invalid_argument& e){return e.what();}
        return "Unknown Base64 operation";
    }
};
class AUTHORIZATION {
    REGISTRATION registry;
public:
    AUTHORIZATION(){registry.add("permit","scope","local read-only fusion");}
    bool permit(const REQUEST& r) const {return r.method=="echo" || r.method=="help";}
};
class COMPATIBILITY {
    MATRIX& matrix;
    REGISTRATION registry;
public:
    explicit COMPATIBILITY(MATRIX& m):matrix(m){registry.add("accept","scope","local procedure");}
    bool accept(const std::string& a,const std::string& b){
        return matrix.get({"fusion",a,b})=="local-procedure";
    }
    void registerPair(const std::string& a,const std::string& b){
        matrix.set({"fusion",a,b},"local-procedure");
    }
};
class ADAPTER {
    REGISTRATION registry;
    std::function<RESPONSE(const REQUEST&)> endpoint;
public:
    explicit ADAPTER(std::function<RESPONSE(const REQUEST&)> e):endpoint(std::move(e)){
        registry.add("invoke","transport","local");
    }
    REQUEST transit(const REQUEST& r, const IR& ir) const {return ir.normalize(r);}
    RESPONSE invoke(const REQUEST& r){return endpoint(r);}
};
class FUSION {
    PATH& path;
    COMPATIBILITY& compatibility;
    AUTHORIZATION& authorization;
    IR& ir;
    REGISTRATION registry;
    std::map<std::string,std::unique_ptr<ADAPTER>> endpoints;
public:
    FUSION(PATH& p,COMPATIBILITY& c,AUTHORIZATION& a,IR& i):path(p),compatibility(c),authorization(a),ir(i){
        registry.add("invoke","policy","deny unsupported");
    }
    void attach(const std::string& name,std::function<RESPONSE(const REQUEST&)> e){
        endpoints.emplace(name,std::make_unique<ADAPTER>(std::move(e)));
    }
    std::string invoke(const TOOL::Arguments& args){
        if(args.size()<3)return "Usage: fuse origin destination method [arguments...]";
        REQUEST r{args[2],TOOL::Arguments(args.begin()+3,args.end())};
        r=ir.normalize(r);
        if(!authorization.permit(r))return "Denied: method not authorized for fusion";
        if(!endpoints.count(args[0])||!endpoints.count(args[1]))return "Unknown interface";
        auto nodes=path.find(args[0],args[1],[this](const std::string& a,const std::string& b){return endpoints.count(a)&&endpoints.count(b)&&compatibility.accept(a,b);});
        if(nodes.empty())return "No passage";
        for(std::size_t i=0;i<nodes.size();++i){
            if(!endpoints.count(nodes[i]))return "Unregistered passage";
            if(i && !compatibility.accept(nodes[i-1],nodes[i]))return "Incompatible passage";
            r=endpoints.at(nodes[i])->transit(r,ir);
            if(!authorization.permit(r))return "Denied after transit";
        }
        return endpoints.at(nodes.back())->invoke(r).value;
    }
};
class ROUNDROBIN {
    REGISTRATION registry;
    std::mutex guard;
    std::size_t cursor=0;
    SIGNAL* signal=nullptr;
public:
    ROUNDROBIN(){registry.add("select","policy","cyclic");}
    void attach(SIGNAL& s){signal=&s;}
    std::string select(const TOOL::Arguments& candidates){
        std::lock_guard<std::mutex> lock(guard);
        if(candidates.empty())return {};
        cursor%=candidates.size();
        auto result=candidates[cursor];
        cursor=(cursor+1)%candidates.size();
        if(signal)signal->emit(result,"roundrobin");
        return result;
    }
};
class HEARTBEAT {
    REGISTRATION registry;
    std::mutex guard;
    std::map<std::string,std::chrono::steady_clock::time_point> pulses;
public:
    HEARTBEAT(){registry.add("pulse","clock","monotonic");}
    void pulse(const std::string& name){
        std::lock_guard<std::mutex> lock(guard);
        pulses[name]=std::chrono::steady_clock::now();
    }
    std::string status(const std::string& name){
        std::lock_guard<std::mutex> lock(guard);
        auto it=pulses.find(name);
        if(it==pulses.end())return "No heartbeat";
        auto age=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-it->second).count();
        return name+" last pulse "+std::to_string(age)+" ms ago";
    }
};
class FLOW {
    FUSION& fusion;
    ROUNDROBIN& robin;
    HEARTBEAT& heartbeat;
    REGISTRATION registry;
public:
    FLOW(FUSION& f,ROUNDROBIN& r,HEARTBEAT& h):fusion(f),robin(r),heartbeat(h){
        registry.add("invoke","policy","cyclic destination selection");
    }
    std::string invoke(const TOOL::Arguments& a){
        if(a.size()<4)return "Usage: flow origin method destination1,destination2 argument...";
        TOOL::Arguments destinations;
        std::istringstream input(a[2]);
        std::string name;
        for(;std::getline(input,name,',');)if(!name.empty())destinations.push_back(name);
        auto destination=robin.select(destinations);
        if(destination.empty())return "No destination";
        TOOL::Arguments request{a[0],destination,a[1]};
        request.insert(request.end(),a.begin()+3,a.end());
        heartbeat.pulse("flow");
        return fusion.invoke(request);
    }
};
class LAMBDA {
    REGISTRATION registry;
    std::mutex guard;
    std::map<std::string,TOOL::Invocation> functions;
public:
    LAMBDA(){
        registry.add("bind","kind","callable");
        registry.add("invoke","policy","registered callable only");
        bind("identity",[](const TOOL::Arguments& a){
            std::string out;
            for(std::size_t i=0;i<a.size();++i){if(i)out+=" ";out+=a[i];}
            return out;
        });
    }
    bool bind(const std::string& name,TOOL::Invocation fn){
        if(name.empty()||!fn)return false;
        std::lock_guard<std::mutex> lock(guard);
        return functions.emplace(name,std::move(fn)).second;
    }
    std::string invoke(const TOOL::Arguments& a){
        TOOL::Invocation fn;
        {
            std::lock_guard<std::mutex> lock(guard);
            if(a.empty()||a[0]=="list"){
                std::string out;
                for(const auto& item:functions)out+=item.first+"\n";
                return out;
            }
            auto it=functions.find(a[0]);
            if(it==functions.end())return "Unknown lambda";
            fn=it->second;
        }
        return fn(TOOL::Arguments(a.begin()+1,a.end()));
    }
};
class INDEX {
    REGISTRATION registry;
    std::mutex guard;
    std::map<std::string,std::map<std::string,std::string>> entries;
public:
    INDEX(){registry.add("set","structure","namespace/key/value");registry.add("get","policy","exact lookup");}
    bool set(const std::string& space,const std::string& key,const std::string& value){
        if(space.empty()||key.empty())return false;
        std::lock_guard<std::mutex> lock(guard);
        entries[space][key]=value;
        return true;
    }
    std::string invoke(const TOOL::Arguments& a){
        if(a.size()==4&&a[0]=="set")return set(a[1],a[2],a[3])?"Indexed":"Invalid index key";
        std::lock_guard<std::mutex> lock(guard);
        if(a.size()==3&&a[0]=="get"){
            auto ns=entries.find(a[1]);
            if(ns==entries.end())return "Index namespace not found";
            auto it=ns->second.find(a[2]);
            return it==ns->second.end()?"Index key not found":it->second;
        }
        if(a.empty()||a[0]=="list"){
            std::string out;
            for(const auto& ns:entries)if(a.size()<2||a[1]==ns.first)
                for(const auto& item:ns.second)out+=ns.first+"/"+item.first+" = "+item.second+"\n";
            return out;
        }
        return "Usage: index list [namespace] | index get namespace key | index set namespace key value";
    }
};
class POOL {
    REGISTRATION registry;
    std::mutex guard;
    std::map<std::string,std::map<std::string,bool>> resources;
public:
    POOL(){registry.add("acquire","policy","exclusive logical lease");}
    bool add(const std::string& pool,const std::string& resource){
        if(pool.empty()||resource.empty())return false;
        std::lock_guard<std::mutex> lock(guard);
        return resources[pool].emplace(resource,false).second;
    }
    std::string acquire(const std::string& pool){
        std::lock_guard<std::mutex> lock(guard);
        auto it=resources.find(pool);
        if(it==resources.end())return {};
        for(auto& r:it->second)if(!r.second){r.second=true;return r.first;}
        return {};
    }
    bool release(const std::string& pool,const std::string& resource){
        std::lock_guard<std::mutex> lock(guard);
        auto p=resources.find(pool);
        if(p==resources.end())return false;
        auto r=p->second.find(resource);
        if(r==p->second.end()||!r->second)return false;
        r->second=false;return true;
    }
    std::string invoke(const TOOL::Arguments& a){
        if(a.size()==3&&a[0]=="add")return add(a[1],a[2])?"Registered":"Rejected";
        if(a.size()==2&&a[0]=="acquire"){auto r=acquire(a[1]);return r.empty()?"No available resource":r;}
        if(a.size()==3&&a[0]=="release")return release(a[1],a[2])?"Released":"Rejected";
        if(a.empty()||a[0]=="list"){
            std::lock_guard<std::mutex> lock(guard);
            std::string out;
            for(const auto& p:resources)for(const auto& r:p.second)out+=p.first+"/"+r.first+(r.second?" leased\n":" available\n");
            return out;
        }
        return "Usage: pool add|release pool resource | pool acquire pool | pool list";
    }
};
class FARM {
    POOL& pool;
    ROUNDROBIN& robin;
    HEARTBEAT& heartbeat;
    REGISTRATION registry;
    std::mutex guard;
    std::map<std::string,TOOL::Arguments> farms;
public:
    FARM(POOL& p,ROUNDROBIN& r,HEARTBEAT& h):pool(p),robin(r),heartbeat(h){registry.add("acquire","policy","cyclic pool selection");}
    std::string invoke(const TOOL::Arguments& a){
        if(a.size()>=3&&a[0]=="define"){
            TOOL::Arguments names;
            for(auto i=a.begin()+2;i!=a.end();++i)if(!i->empty()&&std::find(names.begin(),names.end(),*i)==names.end())names.push_back(*i);
            if(names.empty()||a[1].empty())return "Invalid farm";
            std::lock_guard<std::mutex> lock(guard);
            farms[a[1]]=names;return "Farm defined";
        }
        if(a.size()==2&&a[0]=="acquire"){
            TOOL::Arguments names;
            {std::lock_guard<std::mutex> lock(guard);auto it=farms.find(a[1]);if(it==farms.end())return "Unknown farm";names=it->second;}
            auto first=robin.select(names);
            auto start=std::find(names.begin(),names.end(),first)-names.begin();
            for(std::size_t i=0;i<names.size();++i){
                const auto& p=names[(start+i)%names.size()];
                auto r=pool.acquire(p);
                if(!r.empty()){heartbeat.pulse("farm/"+a[1]);return p+"/"+r;}
            }
            return "No available resource";
        }
        if(a.empty()||a[0]=="list"){
            std::lock_guard<std::mutex> lock(guard);
            std::string out;
            for(const auto& f:farms)for(const auto& p:f.second)out+=f.first+"/"+p+"\n";
            return out;
        }
        return "Usage: farm define name pool... | farm acquire name | farm list";
    }
};
class REFLEX {
    SIGNAL& signal;
    HEARTBEAT& heartbeat;
    REGISTRATION registry;
public:
    REFLEX(SIGNAL& s,HEARTBEAT& h):signal(s),heartbeat(h){
        registry.add("respond","policy","immediate acknowledgement");
    }
    void respond(const std::string& target){
        heartbeat.pulse(target);
        signal.emit(target,"reflex");
    }
};
class REACTION {
    SIGNAL& signal;
    REFLEX& reflex;
    ROUNDROBIN& robin;
    REGISTRATION registry;
public:
    REACTION(SIGNAL& s,REFLEX& r,ROUNDROBIN& rr):signal(s),reflex(r),robin(rr){
        registry.add("react","policy","bounded cyclic notification");
    }
    std::string invoke(const TOOL::Arguments& a){
        if(a.empty())return "Usage: reaction target [destination...]";
        signal.emit(a[0],"reaction");
        reflex.respond(a[0]);
        TOOL::Arguments candidates(a.begin()+1,a.end());
        auto destination=robin.select(candidates);
        if(!destination.empty()){
            signal.emit(destination,"reaction-notice");
            return "Reaction acknowledged; notified "+destination;
        }
        return "Reaction acknowledged";
    }
};
class HYBRID {
    TOOL tool;
    SESSION session;
    MESH mesh;
    CHAIN chain{mesh};
    RING ring{chain,mesh};
    MATRIX matrix;
    GRID grid{matrix};
    PATH path{mesh};
    ROUTE routeSelector{path};
    LANE lane{routeSelector};
    BRIDGE bridge{mesh,routeSelector};
    URI uri;
    URL url;
    CONFIGURATION configuration;
    DISPATCHER dispatcher;
    RPC rpc{dispatcher};
    API api;
    CLI cli;
    MCP mcp;
    SSH ssh;
    RDP rdp{dispatcher};
    VNC vnc{dispatcher};
    BASE64 base64;
    AES aes;
    GCM gcm{aes};
    RSA rsa;
    SHA sha;
    OAEP oaep{rsa,sha};
    PSS pss{rsa,sha};
    AUTHORIZATION authorization;
    COMPATIBILITY compatibility{matrix};
    AST ast;
    IR ir{ast};
    CPP cpp{ir};
    WASM wasm{cpp,ir};
    CODE code;
    CODEBASE codebase{code};
    SANDBOX sandbox{ir};
    SDK sdk{cpp,wasm,sandbox,codebase};
    FUSION fusion{path,compatibility,authorization,ir};
    ROUNDROBIN roundrobin;
    HEARTBEAT heartbeat;
    FLOW flow{fusion,roundrobin,heartbeat};
    LAMBDA lambda;
    INDEX index;
    REFLEX reflex{tool.signals(),heartbeat};
    REACTION reaction{tool.signals(),reflex,roundrobin};
    POOL pool;
    FARM farm{pool,roundrobin,heartbeat};
    REGISTRATION registry;
public:
    HYBRID(): url(uri), configuration(uri,url), dispatcher(tool), api(dispatcher), cli(dispatcher), mcp(dispatcher), ssh(dispatcher) {
        fusion.attach("api",[this](const REQUEST& r){return api.invoke(r);});
        fusion.attach("cli",[this](const REQUEST& r){return cli.invoke(r);});
        fusion.attach("mcp",[this](const REQUEST& r){return mcp.invoke(r);});
        fusion.attach("ssh",[this](const REQUEST& r){return ssh.invoke(r);});
        fusion.attach("rdp",[this](const REQUEST& r){return rdp.invoke(r);});
        fusion.attach("vnc",[this](const REQUEST& r){return vnc.invoke(r);});
        fusion.attach("rpc",[this](const REQUEST& r){return rpc.call(r);});
        TOOL::Arguments names{"api","cli","mcp","ssh","rdp","vnc","rpc"};
        roundrobin.attach(tool.signals());
        ring.connect(names);
        for(const auto& name:names)index.set("interfaces",name,"local");
        for(std::size_t i=0;i<names.size();++i)grid.set("interface",std::to_string(i),names[i]);
        lane.assign("default","api","rpc");
        for(const auto& a:names)for(const auto& b:names)compatibility.registerPair(a,b);
        tool.inject("cpp","development",[this](const TOOL::Arguments& a){return cpp.invoke(a);});
        tool.inject("wasm","development",[this](const TOOL::Arguments& a){return wasm.invoke(a);});
        tool.inject("code","development",[this](const TOOL::Arguments& a){return a.size()==2&&code.validate(a[0],a[1])?"Valid source record":"Invalid source record";});
        tool.inject("codebase","development",[this](const TOOL::Arguments& a){return codebase.invoke(a);});
        tool.inject("sandbox","development",[this](const TOOL::Arguments& a){return sandbox.invoke(a);});
        tool.inject("sdk","development",[this](const TOOL::Arguments& a){return sdk.invoke(a);});
        tool.inject("aes","cryptography",[this](const TOOL::Arguments& a){return aes.invoke(a);});
        tool.inject("gcm","cryptography",[this](const TOOL::Arguments& a){return gcm.invoke(a);});
        tool.inject("rsa","cryptography",[this](const TOOL::Arguments& a){return rsa.invoke(a);});
        tool.inject("oaep","cryptography",[this](const TOOL::Arguments& a){return oaep.invoke(a);});
        tool.inject("pss","cryptography",[this](const TOOL::Arguments& a){return pss.invoke(a);});
        tool.inject("sha","cryptography",[this](const TOOL::Arguments& a){return sha.invoke(a);});
        tool.inject("base64","encoding",[this](const TOOL::Arguments& a){return base64.invoke(a);});
        tool.inject("roundrobin","runtime",[this](const TOOL::Arguments& a){return roundrobin.select(a);});
        tool.inject("heartbeat","runtime",[this](const TOOL::Arguments& a){if(a.size()==2&&a[0]=="pulse"){heartbeat.pulse(a[1]);return std::string("Pulse recorded");}if(a.size()==2&&a[0]=="status")return heartbeat.status(a[1]);return std::string("Usage: heartbeat pulse|status name");});
        tool.inject("flow","fusion",[this](const TOOL::Arguments& a){return flow.invoke(a);});
        lambda.bind("flow",[this](const TOOL::Arguments& a){return flow.invoke(a);});
        lambda.bind("fusion",[this](const TOOL::Arguments& a){return fusion.invoke(a);});
        tool.inject("lambda","runtime",[this](const TOOL::Arguments& a){return lambda.invoke(a);});
        index.set("runtime","lambda","callable registry");
        index.set("runtime","flow","operational circulation");
        lambda.bind("index",[this](const TOOL::Arguments& a){return index.invoke(a);});
        tool.inject("index","runtime",[this](const TOOL::Arguments& a){return index.invoke(a);});
        index.set("runtime","pool","logical resource allocation");
        index.set("runtime","farm","grouped pool allocation");
        lambda.bind("pool",[this](const TOOL::Arguments& a){return pool.invoke(a);});
        lambda.bind("farm",[this](const TOOL::Arguments& a){return farm.invoke(a);});
        tool.inject("pool","runtime",[this](const TOOL::Arguments& a){return pool.invoke(a);});
        tool.inject("farm","runtime",[this](const TOOL::Arguments& a){return farm.invoke(a);});
        index.set("runtime","reaction","controlled operational response");
        index.set("runtime","reflex","immediate heartbeat acknowledgement");
        tool.inject("reaction","runtime",[this](const TOOL::Arguments& a){return reaction.invoke(a);});
        tool.inject("reflex","runtime",[this](const TOOL::Arguments& a){if(a.size()!=1)return std::string("Usage: reflex target");reflex.respond(a[0]);return std::string("Reflex acknowledged");});
        tool.inject("fuse","fusion",[this](const TOOL::Arguments& a){return fusion.invoke(a);});
        registry.add("route","API","local");
        registry.add("route","CLI","local");
        registry.add("route","MCP","local");
        registry.add("route","SSH","local");
        tool.inject("rpc","rpc",[this](const TOOL::Arguments& a){return rpc.invoke(a);});
        tool.inject("bridge","topology",[this](const TOOL::Arguments& a){if(a.size()==3)return bridge.connect(a[0],a[1],a[2]);if(a.size()==1)return bridge.resolve(a[0]);return std::string("Usage: bridge name [from to]");});
        tool.inject("call","runtime",[this](const TOOL::Arguments& a){if(a.empty())return std::string("Usage: call method [arguments]");CALL c(tool);return c.invoke(a[0],TOOL::Arguments(a.begin()+1,a.end())).result;});
        tool.inject("return","runtime",[](const TOOL::Arguments& a){if(a.size()!=1)return std::string("Usage: return text");RETURN r(true,a[0]);return r.result;});
        tool.inject("variable","runtime",[](const TOOL::Arguments& a){if(a.size()!=1)return std::string("Usage: variable text");VARIABLE v(std::make_shared<const VALUE>(VALUE::Data{a[0]}));return std::get<std::string>(v.read()->get());});
        tool.inject("function","runtime",[](const TOOL::Arguments& a){FUNCTION f([](const TOOL::Arguments& x){return x.empty()?std::string{}:x.front();});return f.execute(a).result;});
        tool.inject("class","runtime",[](const TOOL::Arguments& a){if(a.size()!=1)return std::string("Usage: class name");CLASS c(a[0]);return c.identity();});
        tool.inject("object","runtime",[](const TOOL::Arguments& a){if(a.size()!=2)return std::string("Usage: object class text");OBJECT o(std::make_shared<const CLASS>(a[0]));o.set("value",std::make_shared<const VALUE>(VALUE::Data{a[1]}));return std::get<std::string>(o.get("value")->get());});
        tool.inject("instance","runtime",[](const TOOL::Arguments& a){if(a.size()!=2)return std::string("Usage: instance id class");INSTANCE i(a[0],std::make_shared<OBJECT>(std::make_shared<const CLASS>(a[1])));return i.id();});
        tool.inject("array","runtime",[](const TOOL::Arguments& a){ARRAY array;for(const auto& item:a){if(!array.append(std::make_shared<const VALUE>(VALUE::Data{item})))return std::string("Array capacity exceeded");}return std::to_string(array.size());});
        tool.inject("boolean","runtime",[](const TOOL::Arguments& a){if(a.empty()||a.size()>3)return std::string("Usage: boolean true|false [not|and|or|xor [true|false]]");auto valid=[](const std::string& x){return x=="true"||x=="false";};if(!valid(a[0]))return std::string("Invalid boolean");BOOLEAN b(a[0]=="true");bool result=b.evaluate();if(a.size()==2&&a[1]=="not")result=b.negate();else if(a.size()==3&&valid(a[2])){BOOLEAN other(a[2]=="true");if(a[1]=="and")result=b.conjunction(other);else if(a[1]=="or")result=b.disjunction(other);else if(a[1]=="xor")result=b.exclusive(other);else return std::string("Invalid operation");}else if(a.size()!=1)return std::string("Invalid operation");return std::string(result?"true":"false");});
        tool.inject("value","runtime",[](const TOOL::Arguments& a){if(a.size()!=1)return std::string("Usage: value text");VALUE v{VALUE::Data{a[0]}};return std::get<std::string>(v.get());});
        tool.inject("parameter","runtime",[](const TOOL::Arguments& a){if(a.size()!=2)return std::string("Usage: parameter name text");PARAMETER p(a[0],4);VALUE v{VALUE::Data{a[1]}};return p.accept(v)?std::string("Accepted"):std::string("Rejected");});
        tool.inject("argument","runtime",[](const TOOL::Arguments& a){if(a.size()!=2)return std::string("Usage: argument parameter text");PARAMETER p(a[0],4);ARGUMENT arg(std::make_shared<const VALUE>(VALUE::Data{a[1]}));return arg.bind(p)?std::string("Bound"):std::string("Rejected");});
        tool.inject("reference","runtime",[](const TOOL::Arguments& a){if(a.size()!=2)return std::string("Usage: reference id text");auto v=std::make_shared<const VALUE>(VALUE::Data{a[1]});REFERENCE ref(a[0],v);auto resolved=ref.resolve();return resolved?std::get<std::string>(resolved->get()):std::string("Expired");});
        tool.inject("mesh","topology",[this](const TOOL::Arguments& a){if(a.size()!=2)return std::string("Usage: mesh from to");mesh.connect(a[0],a[1]);return std::string("Connected");});
        tool.inject("chain","topology",[this](const TOOL::Arguments& a){chain.connect(a);return std::string("Chain registered");});
        tool.inject("ring","topology",[this](const TOOL::Arguments& a){ring.connect(a);return std::string("Ring registered");});
        tool.inject("route","topology",[this](const TOOL::Arguments& a){return a.size()==2?routeSelector.select(a[0],a[1]):"Usage: route from to";});
        tool.inject("path","topology",[this](const TOOL::Arguments& a){if(a.size()!=2)return std::string("Usage: path from to");std::string out;for(const auto& n:path.find(a[0],a[1]))out+=n+" ";return out;});
        tool.inject("grid","topology",[this](const TOOL::Arguments& a){if(a.size()==3){grid.set(a[0],a[1],a[2]);return std::string("Assigned");}return a.size()==2?grid.get(a[0],a[1]):"Usage: grid x y [value]";});
        tool.inject("matrix","topology",[this](const TOOL::Arguments& a){if(a.size()<2)return std::string("Usage: matrix get coordinates... | matrix set coordinates... value");if(a[0]=="get")return matrix.get(TOOL::Arguments(a.begin()+1,a.end()));if(a[0]=="set"&&a.size()>=3){matrix.set(TOOL::Arguments(a.begin()+1,a.end()-1),a.back());return std::string("Assigned");}return std::string("Invalid matrix command");});
        tool.inject("lane","topology",[this](const TOOL::Arguments& a){if(a.size()==3){lane.assign(a[0],a[1],a[2]);return std::string("Assigned");}return a.size()==1?lane.resolve(a[0]):"Usage: lane name [from to]";});
        tool.inject("session","runtime",[this](const TOOL::Arguments& a){return session.invoke(a);});
        tool.inject("configure","configuration",[this](const TOOL::Arguments& a){return configuration.invoke(a);});
        tool.inject("uri","validation",[this](const TOOL::Arguments& a){return a.size()==1 && uri.validate(a[0]) ? "Valid scheme syntax" : "Invalid URI";});
        tool.inject("url","validation",[this](const TOOL::Arguments& a){return a.size()==1 && url.validate(a[0]) ? "Valid endpoint shape" : "Invalid URL";});
        tool.inject("api","hybrid",[this](const TOOL::Arguments& a){return route(api,a);});
        tool.inject("cli","hybrid",[this](const TOOL::Arguments& a){return route(cli,a);});
        tool.inject("mcp","hybrid",[this](const TOOL::Arguments& a){return route(mcp,a);});
        registry.add("route","RDP","local");
        registry.add("route","VNC","local");
        tool.inject("rdp","hybrid",[this](const TOOL::Arguments& a){return route(rdp,a);});
        tool.inject("vnc","hybrid",[this](const TOOL::Arguments& a){return route(vnc,a);});
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
        try {std::cout << tool.invoke(argv[1],args) << std::endl;return 0;}
        catch(const std::exception& e){std::cerr << e.what() << std::endl;return 1;}
    }
};

int main(int argc, char** argv) { return HYBRID{}.run(argc,argv); }
