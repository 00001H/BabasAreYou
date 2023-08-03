#pragma once
#include"idecl.hpp"
namespace{
    cppp::BiMap<std::string,const ObjectType*> object_types;
    cppp::BiMap<const ObjectType*,const Word*> words;
    cppp::BiMap<std::string,const Property*> props;
}
const Property* getprop(const std::string& st){
    return props.lookup(st);
}
inline const ObjectType* objtype(const std::string& st){
    ITRY
    return object_types.lookup(st);
    ICATCH(std::out_of_range&)
    load_logger.log("No such object: "+st,ERR);
    IPROPAGATE
}
inline const std::string& objname(const ObjectType* t){
    return object_types.reversed_lookup(t);
}
inline bool hasword(const ObjectType* ky){
    return words.hasKey(ky);
}
inline const Word* getword(const ObjectType* ky){
    ITRY
    return words.lookup(ky);
    ICATCH(std::out_of_range&)
    load_logger.log("No such word: "+objname(ky),ERR);
    IPROPAGATE
}
inline const Word* getword(const std::string& st){
    return getword(objtype(st));
}
inline const ObjectType* wordobj(const Word* wd){
    return words.reversed_lookup(wd);
}
inline const std::string& wordname(const Word* wd){
    return objname(words.reversed_lookup(wd));
}
template<typename T>
inline const T* getwordsp(const std::string& ky){
    return &dynamic_cast<const T&>(*getword(ky));//throw error on failure.
}
inline const std::string& propname(const Property* pr){
    return props.reversed_lookup(pr);
}


void inline register_object(const std::string& name,ObjectRender&& rd){
    object_types.emplace(name,new const ObjectType(std::move(rd)));
}
template<std::derived_from<Word> Wt,typename ...Args>
void inline registerWord(const std::string& name,Args&& ...a){
    words.emplace(objtype("#"+name),new const Wt(std::forward<Args>(a)...));
}
//register text object and word packed
template<std::derived_from<Word> Wt,typename ...Args>
void inline rTWPacked(const std::string& name,ObjectRender&& rd,Args&& ...a){
    register_object("#"+name,std::move(rd));
    registerWord<Wt,Args...>(name,std::forward<Args>(a)...);
}
template<std::derived_from<Property> Pr,typename ...Args>
void inline registerProperty(const std::string& name,Args&& ...a){
    props.emplace(name,new const Pr(std::forward<Args>(a)...));
}
//register text object and property packed
template<std::derived_from<Property> Pr,typename ...Args>
void inline rTPPacked(const std::string& name,ObjectRender&& rd,Args&& ...a){
    registerProperty<Pr,Args...>(name,std::forward<Args>(a)...);
    rTWPacked<PropertyWord,const Property*>(name,std::move(rd),getprop(name));
}
