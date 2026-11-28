class FLOW {
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
        return {{"class",{{"identity","FLOW"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"FLOW",{{"class","FLOW"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{{"status",{{"kind","metadata"},{"owner","FLOW"}}}};
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


    CLI& cli;
    API& api;
    AUTHORIZATION& authorization;
    ROUNDROBIN& robin;
    HEARTBEAT& heartbeat;
    REGISTRATION registry;
public:
    FLOW(CLI& cli, API& api, AUTHORIZATION& authorization, ROUNDROBIN& robin, HEARTBEAT& heartbeat)
        : cli(cli), api(api), authorization(authorization), robin(robin), heartbeat(heartbeat) {
        registry.add("invoke","policy","cyclic selection among reachable destinations");
    }
    std::string preview(const TOOL::Arguments& args) {
        constexpr std::string_view suffix = ".sessions.mainframes.digital";
        constexpr std::size_t labelLength = 32;
        using namespace mainframes;
        if (args.size() < 3 || args[0] != "preview") {
            throw std::invalid_argument("Usage: platform preview session-host echo [arguments...]");
        }

        const std::string_view host = args[1];
        const auto label = host.substr(0, labelLength);
        if (host.size() != labelLength + suffix.size() || host.substr(labelLength) != suffix ||
            !std::all_of(label.begin(), label.end(), [](char c) {
                return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
            })) {
            throw std::invalid_argument("Invalid mainframes.digital session locator");
        }

        unsigned bytes = 0;
        for (auto arg = args.begin() + 3; arg != args.end(); ++arg) {
            // TOOL's echo inserts a space once its result is nonempty.
            const unsigned separator = bytes == 0 ? 0 : 1;
            if (separator > maxPayloadBytes - bytes ||
                arg->size() > maxPayloadBytes - bytes - separator) {
                throw std::invalid_argument("Platform input exceeds 4096 bytes");
            }
            bytes += separator + static_cast<unsigned>(arg->size());
        }

        Passage passage;
        constexpr unsigned handle = 1; // One isolated preview per invocation.
        passage.step(Action::enter, handle, Operation::none, 0);
        const auto operation = args[2] == "echo" ? Operation::echo : Operation::none;
        if (passage.step(Action::input, handle, operation, bytes) != Result::accepted) {
            throw std::invalid_argument("Platform operation is not allowed");
        }
        passage.step(Action::passThrough, handle, Operation::none, 0);
        TOOL::Arguments route{"cli", args[2], "api"};
        route.insert(route.end(), args.begin() + 3, args.end());
        auto result = invoke(route);
        passage.step(Action::output, handle, Operation::none, 0);
        passage.step(Action::exit, handle, Operation::none, 0);
        passage.step(Action::close, handle, Operation::none, 0);
        return result;
    }

    std::string invoke(const TOOL::Arguments& a){
        if (a.size() < 3) return "Usage: flow origin method destination1,destination2 [arguments...]";
        const auto deadline = TOOL::InvocationContext::Clock::now() + std::chrono::seconds(1);
        TOOL::Arguments destinations;
        std::istringstream input(a[2]);
        std::string name;
        for(;std::getline(input,name,',');)if(!name.empty())destinations.push_back(name);
        if (destinations.empty()) return "No destination";
        REQUEST instruction{a[1],TOOL::Arguments(a.begin()+3,a.end())};
        if (!authorization.permit(instruction)) return "Denied: method not authorized";

        bool dispatch = false;
        if (a[0] == "cli" && cli.canReach(api, instruction.method)) {
            const auto end = std::remove_if(destinations.begin(), destinations.end(),
                                           [](const auto& candidate) { return candidate != "api"; });
            if (end != destinations.begin()) {
                destinations.erase(end, destinations.end());
                dispatch = true;
            }
        }
        const auto destination = robin.select(destinations);
        heartbeat.pulse("flow");
        if (dispatch) {
            const auto response = cli.invokePeer(api, std::move(instruction), deadline);
            if (response.status != TOOL::InvocationStatus::completed) {
                throw std::runtime_error(response.value);
            }
            return response.value;
        }
        return "Destination selected: "+destination+"; reciprocal participant testimony required";
    }
};
