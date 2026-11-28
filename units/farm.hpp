class FARM {
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
        return {{"class",{{"identity","FARM"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"FARM",{{"class","FARM"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{{"status",{{"kind","metadata"},{"owner","FARM"}}}};
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
