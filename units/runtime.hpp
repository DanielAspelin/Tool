class RUNTIME {
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
        return {{"class",{{"identity","RUNTIME"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"RUNTIME",{{"class","RUNTIME"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{{"status",{{"kind","metadata"},{"owner","RUNTIME"}}}};
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
    TOOL& tool;
    std::mutex guard;
    struct Environment {
        bool active=true;
        std::map<std::string,std::shared_ptr<INSTANCE>> instances;
    };
    std::map<std::string,Environment> environments;
public:
    explicit RUNTIME(TOOL& t):tool(t){
        for(const std::string op:{"create","inspect","attach","detach","instances","suspend","resume","remove","list","run"})registry.add(op,"runtime","local volatile environment");
    }
    std::string invoke(const TOOL::Arguments& a){
        std::unique_lock<std::mutex> lock(guard);
        if(a.size()==1&&a[0]=="list"){std::string out;for(const auto& e:environments)out+=e.first+"\n";return out;}
        if(a.size()<2)return "Expected operation and runtime identity";
        if(a[0]=="create"&&a.size()==2){
            if(a[1].empty()||environments.size()>=64||environments.count(a[1]))return "Invalid runtime or capacity reached";
            environments.emplace(a[1],Environment{});return "Runtime created";
        }
        auto it=environments.find(a[1]);if(it==environments.end())return "Unknown runtime";
        auto& e=it->second;
        if(a[0]=="inspect"&&a.size()==2)return std::string(e.active?"active":"suspended")+"; instances: "+std::to_string(e.instances.size());
        if(a[0]=="suspend"&&a.size()==2){e.active=false;return "Runtime suspended";}
        if(a[0]=="resume"&&a.size()==2){e.active=true;return "Runtime resumed";}
        if(a[0]=="remove"&&a.size()==2){environments.erase(it);return "Runtime removed";}
        if(a[0]=="instances"&&a.size()==2){std::string out;for(const auto& p:e.instances)out+=p.first+"\n";return out;}
        if(!e.active)return "Runtime suspended";
        if(a[0]=="attach"&&a.size()==4){
            if(a[2].empty()||a[3].empty()||e.instances.size()>=64||e.instances.count(a[2]))return "Invalid instance or capacity reached";
            e.instances.emplace(a[2],std::make_shared<INSTANCE>(a[2],std::make_shared<OBJECT>(std::make_shared<const CLASS>(a[3]))));return "Instance attached";
        }
        if(a[0]=="detach"&&a.size()==3)return e.instances.erase(a[2])?"Instance detached":"Unknown instance";
        if(a[0]=="run"&&a.size()>=4){
            auto p=e.instances.find(a[2]);if(p==e.instances.end())return "Unknown instance";
            auto instance=p->second;auto command=a[3];
            if(command=="exit"||command=="runtime"||command=="RUNTIME.invoke"||command.compare(0,8,"runtime.")==0)return "Unsupported runtime command";
            lock.unlock();return tool.invoke(command,TOOL::Arguments(a.begin()+4,a.end()));
        }
        return "Invalid runtime operation";
    }
};
