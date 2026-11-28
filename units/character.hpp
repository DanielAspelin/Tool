class CHARACTER {
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
        return {{"class",{{"identity","CHARACTER"},{"version","0.1.0"},{"activation","invocation-bound"}}},
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
    ToolLocalRegistry MODEL{{"CHARACTER",{{"class","CHARACTER"},{"activation","invocation-bound"}}}};
    ToolLocalRegistry COMMANDS{{"status",{{"kind","metadata"},{"owner","CHARACTER"}}}};
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
    const char32_t value;
public:
    explicit CHARACTER(char32_t v):value(v){
        if(v>0x10ffff||(v>=0xd800&&v<=0xdfff))throw std::invalid_argument("Invalid Unicode scalar");
        registry.add("get","type","Unicode scalar");
    }
    char32_t get() const{return value;}
    static std::pair<char32_t,std::size_t> decode(const std::string& text,std::size_t offset){
        if(offset>=text.size())throw std::invalid_argument("Missing character");
        auto b=static_cast<unsigned char>(text[offset]);char32_t v;std::size_t n;
        if(b<0x80){v=b;n=1;}else if(b>=0xc2&&b<=0xdf){v=b&31;n=2;}else if(b>=0xe0&&b<=0xef){v=b&15;n=3;}else if(b>=0xf0&&b<=0xf4){v=b&7;n=4;}else throw std::invalid_argument("Invalid UTF-8");
        if(n>text.size()-offset)throw std::invalid_argument("Truncated UTF-8");
        for(std::size_t i=1;i<n;++i){auto c=static_cast<unsigned char>(text[offset+i]);if((c&0xc0)!=0x80)throw std::invalid_argument("Invalid UTF-8");v=(v<<6)|(c&63);}
        if((n==2&&v<0x80)||(n==3&&v<0x800)||(n==4&&v<0x10000)||v>0x10ffff||(v>=0xd800&&v<=0xdfff))throw std::invalid_argument("Invalid Unicode scalar");
        return {v,n};
    }
    static char32_t parse(const std::string& text){auto d=decode(text,0);if(d.second!=text.size())throw std::invalid_argument("Expected one Unicode scalar");return d.first;}
    std::string utf8() const{
        std::string out;auto v=value;
        if(v<0x80)out.push_back(static_cast<char>(v));
        else if(v<0x800){out.push_back(static_cast<char>(0xc0|(v>>6)));out.push_back(static_cast<char>(0x80|(v&63)));}
        else if(v<0x10000){out.push_back(static_cast<char>(0xe0|(v>>12)));out.push_back(static_cast<char>(0x80|((v>>6)&63)));out.push_back(static_cast<char>(0x80|(v&63)));}
        else{out.push_back(static_cast<char>(0xf0|(v>>18)));out.push_back(static_cast<char>(0x80|((v>>12)&63)));out.push_back(static_cast<char>(0x80|((v>>6)&63)));out.push_back(static_cast<char>(0x80|(v&63)));}
        return out;
    }
    std::shared_ptr<const VALUE> asValue() const{return std::make_shared<const VALUE>(VALUE::Data{utf8()});}
};
