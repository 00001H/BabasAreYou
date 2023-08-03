#include"obj.hpp"
#include"lvl.hpp"
class load_failed : public std::runtime_error{
    public:    
        using std::runtime_error::runtime_error;
        load_failed(const std::string& msg,size_t lnno) : load_failed(msg+"(line "+std::to_string(lnno)+')'){}
};
namespace{
    pos_t getnum(const std::string& s,size_t dgln){
        static pos_t va;
        static size_t c;
        try{
            va = std::stoi(s,&c);
            if(c<s.length()){
                throw load_failed("Bad number "+s,dgln);
            }
        }catch(std::invalid_argument&){
            throw load_failed("Bad number "+s,dgln);
        }
        return va;
    }
    levelsz_t ugetnum(const std::string& s,size_t dgln){
        if(s.starts_with('-')){
            throw load_failed("Number must be unsigned "+s,dgln);
        }
        static levelsz_t va;
        static size_t c;
        try{
            if constexpr(sizeof(unsigned long)>=sizeof(levelsz_t)){
                va = std::stoul(s,&c);
                if(c<s.length()){
                    throw load_failed("Bad number "+s,dgln);
                }
            }else{
                unsigned long long ullr = std::stoull(s,&c);
                if(ullr>static_cast<unsigned long long>(std::numeric_limits<levelsz_t>::max())){
                    throw load_failed("Number too big "+s,dgln);
                }
                return static_cast<levelsz_t>(ullr);
            }
        }catch(std::invalid_argument&){
            throw load_failed("Bad number "+s,dgln);
        }
        return va;
    }
}
GameState load_level(const std::string& code){
    std::vector<std::string> lines = cppp::splitlines(code);
    if(lines.empty()){
        throw load_failed("Level data is empty");
    }
    std::vector<levelsz_t> lx;
    cppp::tokenize<char>(lines.front(),' ',[&lx](const std::string& s,size_t){
        lx.push_back(ugetnum(s,1uz));
        return false;
    });
    if(lx.size()!=2uz){
        throw load_failed("First line must be two numbers");
    }
    GameState gs{lx[0uz],lx[1uz]};
    std::vector<std::string> instr;
    coords_t posbuf;
    const static std::unordered_map<char,Direction> dimap{
        {'^',Direction::UP},
        {'<',Direction::LEFT},
        {'>',Direction::RIGHT},
        {'v',Direction::DOWN}
    };
    constexpr static auto getdir = [](const std::string& s,size_t dgln){
        if(s.length()>1uz){
            throw load_failed("Bad direction",dgln);
        }
        if(!dimap.contains(s.front())){
            throw load_failed("Bad direction",dgln);
        }
        return dimap.at(s.front());
    };
    std::vector<std::string>::const_iterator iit;
    Direction _dbuf{Direction::UP};
    size_t lnno=1uz;
    for(auto li=lines.cbegin()+1uz;li!=lines.cend();++li){
        ++lnno;
        instr.clear();
        cppp::tokenize<char>(*li,' ',[&instr](const std::string& s,size_t){
            instr.emplace_back(s);
            return false;
        });
        iit = instr.cbegin();
        if((*iit)=="-"){
            if(instr.size()!=6uz){
                throw load_failed("Line: exactly 5 arguments required, found "+std::to_string(instr.size()-1uz),lnno);
            }
            posbuf.x = getnum(*++iit,lnno);
            posbuf.y = getnum(*++iit,lnno);
            _dbuf = getdir(*++iit,lnno);
            levelsz_t lth = ugetnum(*++iit,lnno);
            ++iit;
            for(levelsz_t i=0u;i<lth;++i){
                gs.add(posbuf,npBo(*iit,_dbuf));
                posbuf += _dbuf.off();
            }
        }else{
            if(instr.size()<3uz){
                throw load_failed("Place-object: too few arguments",lnno);
            }
            try{
                posbuf.x = getnum(*iit,lnno);
            }catch(load_failed& l){
                throw load_failed(l.what()+"(Did you mistype the command?)"s);
            }
            posbuf.y = getnum(*++iit,lnno);
            ++iit;
            try{
                _dbuf = getdir(*iit,lnno);
            }catch(load_failed& l){
                _dbuf = Direction::RIGHT;
                --iit;
            }
            while((++iit)!=instr.cend()){
                gs.add(posbuf,npBo(*iit,_dbuf));
                posbuf += _dbuf.off();
            }
        }
    }
    return gs;
}
class World{
    std::string path;
    std::string levelpath(const std::string& levelname){
        return path+levelname+".ld";
    }
    public:
        World(const std::string& pat) : path(pat){
            if(!path.ends_with('/')){
                path.push_back('/');
            }
        }
        GameState load_level(const std::string& levelname){
            std::string lpath = levelpath(levelname);
            if(pygame::fileexists(lpath)){
            }else{
                throw std::logic_error("Level file not found: "+levelname);
            }
        }
};
