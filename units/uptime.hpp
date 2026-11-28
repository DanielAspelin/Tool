class UPTIME {
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
        return {{"class",{{"identity","UPTIME"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"UPTIME",{{"class","UPTIME"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{
        {"status", {{"kind","metadata"},{"owner","UPTIME"}}},
        {"start", {{"kind","availability tracking"},{"owner","UPTIME"}}},
        {"list", {{"kind","tracked entities"},{"owner","UPTIME"}}},
        {"lock", {{"kind","immediate counter lock"},{"owner","UPTIME"}}},
        {"lock-status", {{"kind","counter lock timing"},{"owner","UPTIME"}}},
        {"unlock", {{"kind","release or cancel counter lock"},{"owner","UPTIME"}}}
    };
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


    REGISTRATION registry;
    std::mutex guard;
    using Clock=std::chrono::steady_clock;
    struct State {
        bool up = true;
        Clock::time_point since = Clock::now();
        Clock::duration available{}, unavailable{};
        std::optional<Clock::time_point> deadline;

        bool locked(Clock::time_point now) const { return deadline && now >= *deadline; }

        Clock::duration measure(bool availability, Clock::time_point now) const {
            const auto total = availability ? available : unavailable;
            return up == availability ? total + ((locked(now) ? *deadline : now) - since) : total;
        }

        void accrueUntil(Clock::time_point end) {
            (up ? available : unavailable) += end - since;
            since = end;
        }
    };
    std::map<std::string,State> states;

    static auto nanoseconds(Clock::duration duration) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    }

    std::string lockStatus(const std::string& id) {
        const std::lock_guard<std::mutex> lock(guard);
        const auto it = states.find(id);
        if (it == states.end()) return "Unknown entity";
        const auto& state = it->second;
        if (!state.deadline) return "phase=open";
        const auto now = Clock::now();
        return std::string(state.locked(now) ? "phase=locked" : "phase=armed") +
            " deadline_ns=" + std::to_string(nanoseconds(state.deadline->time_since_epoch())) +
            " remaining_ns=" + std::to_string(nanoseconds(std::max(Clock::duration::zero(), *state.deadline - now)));
    }

    std::string unlock(const std::string& id) {
        const std::lock_guard<std::mutex> lock(guard);
        const auto it = states.find(id);
        if (it == states.end()) return "Unknown entity";
        auto& state = it->second;
        if (!state.deadline) return "Time counters already open";
        const auto now = Clock::now();
        const bool locked = state.locked(now);
        if (locked) {
            state.accrueUntil(*state.deadline);
            state.since = now;
        }
        state.deadline.reset();
        return locked ? "Time counters unlocked" : "Time lock canceled";
    }
public:
    UPTIME() {
        for (const auto& operation : COMMANDS) {
            registry.add(operation.first, "uptime", operation.second.at("kind"));
        }
    }

    std::string prepareLock(const std::string& id, std::chrono::milliseconds delay) {
        const std::lock_guard<std::mutex> lock(guard);
        const auto it = states.find(id);
        if (it == states.end()) return "Unknown entity";
        auto& state = it->second;
        const auto now = Clock::now();
        if (state.deadline) return state.locked(now) ? "Time counters locked" : "Time lock already armed";
        state.deadline = now + delay;
        return delay.count() == 0 ? "Time counters locked" : "Time lock armed";
    }

    std::string transition(const std::string& id,bool up){
        std::lock_guard<std::mutex> lock(guard);
        auto it=states.find(id);if(it==states.end())return "Unknown entity";
        auto& state = it->second;
        const auto now = Clock::now();
        if (state.locked(now)) return "Time counters locked";
        if (state.up == up) return "State unchanged";
        state.accrueUntil(now);
        state.up = up;
        return up ? "Available" : "Unavailable";
    }
    std::string measure(const std::string& id,bool up){
        std::lock_guard<std::mutex> lock(guard);
        auto it=states.find(id);if(it==states.end())return "Unknown entity";
        const auto& state = it->second;
        const auto elapsed = state.measure(up, Clock::now());
        return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()) +
            " ms; state=" + (state.up ? "up" : "down");
    }
    std::string invoke(const TOOL::Arguments& a){
        if(a.size()==2&&a[0]=="status")return measure(a[1],true);
        if (a.size() == 2 && a[0] == "lock") return prepareLock(a[1], std::chrono::milliseconds::zero());
        if (a.size() == 2 && a[0] == "lock-status") return lockStatus(a[1]);
        if (a.size() == 2 && a[0] == "unlock") return unlock(a[1]);
        std::lock_guard<std::mutex> lock(guard);
        if(a.size()==2&&a[0]=="start"){if(a[1].empty()||a[1].size()>128||states.size()>=256||states.count(a[1]))return "Invalid entity or capacity reached";states.emplace(a[1],State{});return "Availability tracking started";}
        if(a.size()==1&&a[0]=="list"){std::string out;for(const auto& p:states)out+=p.first+"\n";return out;}
        return "Invalid uptime operation";
    }
};
