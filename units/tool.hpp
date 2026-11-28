class TOOL {
public:
    using ToolLocalRegistry = std::map<std::string,std::map<std::string,std::string>>;
    class SPECIFIER {
    public:
        static bool transversion(ToolLocalRegistry& specification,const std::string& category,const std::string& identity,const std::string& revision){
            if(category.empty()||identity.empty()||revision.empty())return false;
            const auto key=category+":"+identity;
            auto& record=specification[key];
            const auto previous=record.find("version");
            if(previous!=record.end()&&previous->second==revision)return false;
            if(previous!=record.end())record["previous_version"]=previous->second;
            record["version"]=revision;
            return true;
        }
        static void generate(ToolLocalRegistry& specification,const std::string& name,const ToolLocalRegistry& array){
            for(const auto& entry:array){
                auto& record=specification["array:"+name+":"+entry.first];
                record=entry.second;
            }
        }
        static std::uint64_t index(std::atomic<std::uint64_t>& counter){
            auto current=counter.load();
            bool complete=false;
            while(!complete){advance(counter,current,complete);}
            return current+1;
        }
        static void advance(std::atomic<std::uint64_t>& counter,std::uint64_t& current,bool& complete){
            if(current==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Instance index exhausted");
            complete=counter.compare_exchange_weak(current,current+1);
        }
    };
private:
    inline static std::atomic<std::uint64_t> specificationInstanceCounter{0};
    static ToolLocalRegistry initialSpecification(){
        return {{"class",{{"identity","TOOL"},{"version","0.1.0"},{"activation","invocation-bound"}}},
                {"array:ARCHITECTURE",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:DESIGN",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:VERSION",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:INFORMATION",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:DESCRIPTION",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:MODEL",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:COMMANDS",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"array:SPECIFICATION",{{"kind","nested associative array"},{"version","0.1.0"}}},
                {"function:COMMANDER.define",{{"version","0.1.0"}}},
                {"function:COMMANDER.inspect",{{"version","0.1.0"}}},
                {"function:SPECIFIER.transversion",{{"version","0.1.0"}}},
                {"function:SPECIFIER.generate",{{"version","0.1.0"}}},
                {"function:SPECIFIER.index",{{"version","0.1.0"}}}};
    }
    ToolLocalRegistry VERSION{{"class",{{"version","0.1.0"}}}};
    ToolLocalRegistry INFORMATION{{"lifecycle",{{"ownership","invoker"},{"runtime","optional"}}}};
    ToolLocalRegistry DESCRIPTION{{"structure",{{"architecture","reciprocal"},{"design","invocation-bound"}}}};
    ToolLocalRegistry SPECIFICATION=initialSpecification();
    std::uint64_t specificationInstanceIndex=SPECIFIER::index(specificationInstanceCounter);
public:

    private:
    ToolLocalRegistry ARCHITECTURE{{"structure",{{"ownership","invoker"},{"connectivity","reciprocal"}}}};
    ToolLocalRegistry DESIGN{{"behavior",{{"activation","invocation-bound"},{"runtime","optional"}}}};
    public:
    class COMMANDER {
    public:
        static void define(ToolLocalRegistry& commands,const std::string& id,const std::string& key,const std::string& value){commands[id][key]=value;}
        static const ToolLocalRegistry::mapped_type* inspect(const ToolLocalRegistry& commands,const std::string& id){auto it=commands.find(id);return it==commands.end()?nullptr:&it->second;}
    };
private:
    ToolLocalRegistry MODEL{{"TOOL",{{"class","TOOL"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{{"status",{{"kind","metadata"},{"owner","TOOL"}}}};
public:
    ToolLocalRegistry testifyIntegration() const {
        ToolLocalRegistry testimony;
        SPECIFIER::generate(testimony,"ARCHITECTURE",ARCHITECTURE);
        SPECIFIER::generate(testimony,"DESIGN",DESIGN);
        SPECIFIER::generate(testimony,"MODEL",MODEL);
        SPECIFIER::generate(testimony,"VERSION",VERSION);
        testimony["participant"]=SPECIFICATION.at("class");
        testimony["integration"]["status"]="declared; reciprocal verification required";
        return testimony;
    }
    ToolLocalRegistry specificationSnapshot() const {
        auto result=SPECIFICATION;
        SPECIFIER::generate(result,"ARCHITECTURE",ARCHITECTURE);
        SPECIFIER::generate(result,"DESIGN",DESIGN);
        SPECIFIER::generate(result,"VERSION",VERSION);
        SPECIFIER::generate(result,"INFORMATION",INFORMATION);
        SPECIFIER::generate(result,"DESCRIPTION",DESCRIPTION);
        SPECIFIER::generate(result,"MODEL",MODEL);
        SPECIFIER::generate(result,"COMMANDS",COMMANDS);
        result["instance"]["index"]=std::to_string(specificationInstanceIndex);
        result["instance"]["spid"]=result["class"]["identity"]+":"+std::to_string(specificationInstanceIndex);
        for(const auto* name: {"instantiate","initialize","load","run","execute","finalize","terminate","clean","flush","reconcile","automate","generate","copy","dump","delete","create","construct","reconstruct","deliver","serve","obtain","retain","resume","try","catch","throw","retry","request","respond","negotiate","pidkill","destroy","cleanup","commence","await","preprocess","postprocess","read","write","declare","scale","schedule","allocate","locate","resolve","returnResult","release"}){auto& entry=result[std::string("foundation:")+name];entry["version"]="0.1.0";entry["status"]="planned";}
        return result;
    }
private:


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
    enum class InvocationStatus { untracked, completed, rejected, timedOut };

    struct InvocationContext {
        using Clock = std::chrono::steady_clock;
        std::uint64_t id;
        Clock::time_point deadline;

        bool expired() const { return Clock::now() >= deadline; }
    };

    std::string invoke(const std::string& name, const Arguments& args,
                       const InvocationContext* context = nullptr, InvocationStatus* status = nullptr) {
        if (status) *status = InvocationStatus::rejected;
        const auto snapshot=timestamp.capture();
        static thread_local unsigned depth=0;
        if(!boundary.accept(name,args,depth)){dump.store(snapshot,name,"boundary");return "Invocation boundary exceeded";}
        const bool outer = depth == 0 && name != "script";
        if(outer&&!scheduler.enter()){dump.store(snapshot,name,"deferred");return "Capacity unavailable; retry later";}
        CLEANUP::Lease admission([this,outer]{if(outer)scheduler.leave();});
        ++depth;
        CLEANUP::Lease nesting([&]{--depth;});
        const bool exclusive=name=="configure"||name=="invoke"||name=="ipify"||name=="script";
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
        if (context && context->expired()) {
            if (status) *status = InvocationStatus::timedOut;
            trigger.complete(name, false);
            recordTiming(name, snapshot);
            return "Invocation deadline exceeded";
        }
        try {
            auto result = call(args);
            const bool expired = context && context->expired();
            const bool foregroundWorkspace = (name == "launch" || name == "workspace" || name == "server") && !args.empty() && args[0] == "start";
            if (expired || (!foregroundWorkspace && watchdog.expired(snapshot))) dump.store(snapshot, name, "deadline-exceeded");
            trigger.complete(name, !expired);
            recordTiming(name, snapshot);
            if (status) *status = expired ? InvocationStatus::timedOut : InvocationStatus::completed;
            if (expired) return "Invocation deadline exceeded";
            return result;
        }
        catch(...){if(!dump.store(snapshot,name,"exception"))std::cerr<<"Dump unavailable\n";trigger.complete(name,false);recordTiming(name,snapshot);throw;}
    }
    bool registered(const std::string& name){
        std::lock_guard<std::mutex> lock(guard);return registry.count(name)!=0;
    }
    std::string describeCommand(const std::string& name) {
        std::lock_guard<std::mutex> lock(guard);
        const auto entry = registry.find(name);
        if (entry == registry.end()) throw std::invalid_argument("Unknown command: " + name);
        return entry->first + " [" + entry->second.origin + "]";
    }
    std::string catalogue(const std::string& prefix){
        std::lock_guard<std::mutex> lock(guard);std::string out;
        for(const auto& e:registry)if(e.first.compare(0,prefix.size(),prefix)==0)out+=e.first+"\n";
        return out;
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
