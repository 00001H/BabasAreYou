#include"obj.hpp"
#include"lvl.hpp"
class load_failed : public cppp::u8_runtime_error{
    public:
        using cppp::u8_runtime_error::u8_runtime_error;
        load_failed(const sv msg,size_t lnno) : load_failed(msg+u8"(line "s+cppp::to_u8string(lnno)+u8')'){}
};
namespace{
    pos_t getnum(const sv s,size_t dgln){
        try{
            return cppp::sgetnum<pos_t>(s);
        }catch(std::invalid_argument&){
            throw load_failed(u8"Bad number "s+s,dgln);
        }catch(std::overflow_error&){
            throw load_failed(u8"Number too big/small "s+s,dgln);
        }
    }
    levelsz_t ugetnum(const sv s,size_t dgln){
        if(s.starts_with('-')){
            throw load_failed(u8"Number must be unsigned "s+s,dgln);
        }
        try{
            return cppp::sgetnum<levelsz_t>(s);
        }catch(std::invalid_argument&){
            throw load_failed(u8"Bad number "s+s,dgln);
        }catch(std::overflow_error&){
            throw load_failed(u8"Number too big/small "s+s,dgln);
        }
    }
}
GameState load_level(const sv code){
    std::vector<str> lines = cppp::split(code,u8'\n');
    if(lines.size()<2uz){
        throw load_failed(u8"Level data missing"sv);
    }
    str prnt = lines.front();
    std::vector<str> lx{cppp::split(lines[1uz],u8' ')};
    if(lx.size()!=2uz){
        throw load_failed(u8"First line must be two numbers, found "sv+lines.front());
    }
    GameState gs{ugetnum(lx[0uz],1uz),ugetnum(lx[1uz],1uz),GameState::MetaLD{prnt}};
    std::vector<str> instr;
    coords_t posbuf;
    const static std::unordered_map<char,Direction> dimap{
        {'^',Direction::UP},
        {'<',Direction::LEFT},
        {'>',Direction::RIGHT},
        {'v',Direction::DOWN}
    };
    constexpr static auto getdir = [](const sv s,size_t dgln){
        if(s.length()>1uz){
            throw load_failed(u8"Bad direction"s,dgln);
        }
        if(!dimap.contains(s.front())){
            throw load_failed(u8"Bad direction"s,dgln);
        }
        return dimap.at(s.front());
    };
    std::vector<str>::const_iterator iit;
    Direction dbuf{Direction::UP};
    size_t lnno=1uz;
    str sbuf;
    pBo last_added = nullptr;
    for(auto li=lines.cbegin()+2uz;li!=lines.cend();++li){
        ++lnno;
        instr = cppp::split(*li,u8' ');
        iit = instr.cbegin();
        if((*iit)==u8"-"sv){
            if(instr.size()!=6uz){
                throw load_failed(u8"Line: exactly 5 arguments required, found "s+cppp::to_u8string(instr.size()-1uz),lnno);
            }
            posbuf.x = getnum(*++iit,lnno);
            posbuf.y = getnum(*++iit,lnno);
            dbuf = getdir(*++iit,lnno);
            levelsz_t lth = ugetnum(*++iit,lnno);
            ++iit;
            for(levelsz_t i=0u;i<lth;++i){
                gs.add(posbuf,npBo(*iit,dbuf));
                posbuf += dbuf.off();
            }
        }else{
            if(instr.size()<3uz){
                throw load_failed(u8"Place-object: too few arguments"s,lnno);
            }
            try{
                posbuf.x = getnum(*iit,lnno);
            }catch(load_failed& l){
                throw load_failed(l.u8what()+u8"(Did you mistype the command?)"s);
            }
            posbuf.y = getnum(*++iit,lnno);
            ++iit;
            try{
                dbuf = getdir(*iit,lnno);
            }catch(load_failed& l){
                dbuf = Direction::RIGHT;
                --iit;
            }
            while((++iit)!=instr.cend()){
                sbuf = *iit;
                if(sbuf.starts_with(u8'"')){
                    if(!sbuf.ends_with(u8'"')){
                        throw load_failed(u8"Special: need closing quotes(specials cannot contain spaces)"sv);
                    }
                    if(!last_added){
                        throw load_failed(u8"Specials must go after an object in the specifier list"sv);
                    }
                    last_added->setspecial(sv(sbuf).substr(1uz,sbuf.size()-2uz));
                    last_added = nullptr;
                }else{
                    gs.add(posbuf,last_added = npBo(sbuf,dbuf));
                    posbuf += dbuf.off();
                }
            }
        }
    }
    return gs;
}
class LevelSet{
    str path;
    str levelpath(const sv levelname){
        return path+levelname+u8".ld"sv;
    }
    public:
        LevelSet(const sv pat) : path(pat){
            if(!path.ends_with(u8'/')){
                path.push_back(u8'/');
            }
        }
        GameState load_level(const sv levelname){
            str lpath = levelpath(levelname);
            if(pygame::fileexists(lpath)){
                return ::load_level(pygame::loadStringFile(lpath));
            }else{
                throw cppp::u8_logic_error(u8"Level file not found: "sv+lpath);
            }
        }
};
