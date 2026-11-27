class CAPSULE {
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
        return {{"class",{{"identity","CAPSULE"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"CAPSULE",{{"class","CAPSULE"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{{"status",{{"kind","metadata"},{"owner","CAPSULE"}}}};
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
    struct Record {std::string payload;bool sealed=false;bool quarantined=false;std::string reason;};
    std::map<std::string,Record> records;
public:
    CAPSULE(){for(const std::string op:{"create","inspect","read","seal","remove","list"})registry.add(op,"capsule","volatile data");}
    std::string invoke(const TOOL::Arguments& a){
        std::lock_guard<std::mutex> lock(guard);
        if(a.size()==1&&a[0]=="list"){std::string out;for(const auto& p:records)out+=p.first+"\n";return out;}
        if(a.size()<2)return "Expected operation and capsule identity";
        if(a[0]=="create"&&a.size()==3){
            if(a[1].empty()||a[1].size()>128||a[2].size()>4096||records.size()>=256||records.count(a[1]))return "Invalid capsule or capacity reached";
            records.emplace(a[1],Record{a[2],false,false,{}});return "Capsule created";
        }
        auto it=records.find(a[1]);if(it==records.end())return "Unknown capsule";
        auto& r=it->second;
        if(a[0]=="inspect"&&a.size()==2)return std::string(r.quarantined?"quarantined":r.sealed?"sealed":"open")+"; bytes: "+std::to_string(r.payload.size());
        if(r.quarantined)return "Capsule quarantined";
        if(a[0]=="read"&&a.size()==2)return r.payload;
        if(a[0]=="seal"&&a.size()==2){r.sealed=true;return "Capsule sealed";}
        if(a[0]=="remove"&&a.size()==2){records.erase(it);return "Capsule removed";}
        return "Invalid capsule operation";
    }
    std::string quarantine(const TOOL::Arguments& a){
        std::lock_guard<std::mutex> lock(guard);
        if(a.size()==1&&a[0]=="list"){std::string out;for(const auto& p:records)if(p.second.quarantined)out+=p.first+"\n";return out;}
        if(a.size()<2)return "Expected operation and capsule identity";
        auto it=records.find(a[1]);if(it==records.end())return "Unknown capsule";
        auto& r=it->second;
        if(a[0]=="hold"&&a.size()==3&&!a[2].empty()&&a[2].size()<=512){if(r.quarantined)return "Already quarantined";r.reason=a[2];r.quarantined=true;return "Capsule quarantined";}
        if(a[0]=="inspect"&&a.size()==2)return r.quarantined?"Quarantined: "+r.reason:"Not quarantined";
        if(a[0]=="release"&&a.size()==2&&r.quarantined){r.quarantined=false;r.reason.clear();return "Capsule released by local operator (not verified)";}
        return "Invalid quarantine operation";
    }
};
