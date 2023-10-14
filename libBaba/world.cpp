#include"world.hpp"
GameState load_level(const sv code){
    std::vector<str> lines = cppp::split(code,u8'\n');
    if(lines.size()<2uz){
        throw load_failed(u8"Level data missing"sv);
    }
    str prnt = lines.front();
    std::vector<str> lx{cppp::split(lines[1uz],u8' ')};
    if(lx.size()!=2uz){
        throw load_failed(u8"Second line must be two numbers, found "sv+lines[1uz]);
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
    Object* last_added = nullptr;
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
                gs.add(posbuf,new Object(objtype(*iit),dbuf));
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
                    last_added = new Object(objtype(sbuf),dbuf);
                    using pO = Object*;
                    gs.add(posbuf,pO(last_added));
                    posbuf += dbuf.off();
                }
            }
        }
    }
    return gs;
}
