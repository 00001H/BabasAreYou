#include"obj.hpp"
#include"lvl.hpp"
class load_failed : public cppp::u8_runtime_error{
    public:
        using cppp::u8_runtime_error::u8_runtime_error;
        load_failed(const sv msg,size_t lnno) : load_failed(msg+u8"(line "s+cppp::to_u8string(lnno)+u8')'){}
};
inline pos_t getnum(const sv s,size_t dgln){
    try{
        return cppp::sgetnum<pos_t>(s);
    }catch(std::invalid_argument&){
        throw load_failed(u8"Bad number "s+s,dgln);
    }catch(std::overflow_error&){
        throw load_failed(u8"Number too big/small "s+s,dgln);
    }
}
inline levelsz_t ugetnum(const sv s,size_t dgln){
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
GameState load_level(const sv code);
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
                return ::load_level(pygame::load_string_file(lpath));
            }else{
                throw cppp::u8_logic_error(u8"Level file not found: "sv+lpath);
            }
        }
};
