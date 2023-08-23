#pragma once
#include"idecl.hpp"
namespace{
    cppp::BiMap<str,const ObjectType*> object_types;
    cppp::BiMap<const ObjectType*,const Word*> words;
    cppp::BiMap<str,const Property*> props;
}
const Property* getprop(sv st){
    return props.lookup(st);
}
inline const ObjectType* objtype(sv st){
    ITRY
    return object_types.lookup(st);
    ICATCH(std::out_of_range&)
    load_logger.log(u8"No such object: "s+st,ERR);
    IPROPAGATE
}
inline str objname(const ObjectType* t){
    return object_types.reversed_lookup(t);
}
inline bool hasword(const ObjectType* ky){
    return words.hasKey(ky);
}
inline const Word* getword(const ObjectType* ky){
    ITRY
    return words.lookup(ky);
    ICATCH(std::out_of_range&)
    load_logger.log(u8"No such word: "s+objname(ky),ERR);
    IPROPAGATE
}
inline const Word* getword(sv st){
    return getword(objtype(st));
}
inline const ObjectType* wordobj(const Word* wd){
    return words.reversed_lookup(wd);
}
inline str wordname(const Word* wd){
    return objname(words.reversed_lookup(wd));
}
template<typename T>
inline const T* getwordsp(sv ky){
    return &dynamic_cast<const T&>(*getword(ky));//throw error on failure.
}
inline str propname(const Property* pr){
    return props.reversed_lookup(pr);
}


void inline register_object(sv name,ObjectRender&& rd){
    object_types.emplace(str(name),new const ObjectType(std::move(rd)));
}
template<std::derived_from<Word> Wt,typename ...Args>
void inline registerWord(sv name,Args&& ...a){
    words.emplace(objtype(u8'#'+str(name)),new const Wt(std::forward<Args>(a)...));
}
//register text object and word packed
template<std::derived_from<Word> Wt,typename ...Args>
void inline rTWPacked(sv name,ObjectRender&& rd,Args&& ...a){
    register_object(u8'#'+str(name),std::move(rd));
    registerWord<Wt,Args...>(name,std::forward<Args>(a)...);
}
template<std::derived_from<Property> Pr,typename ...Args>
void inline registerProperty(sv name,Args&& ...a){
    props.emplace(str(name),new const Pr(std::forward<Args>(a)...));
}
//register text object and property packed
template<std::derived_from<Property> Pr,typename ...Args>
void inline rTPPacked(sv name,ObjectRender&& rd,Args&& ...a){
    registerProperty<Pr,Args...>(name,std::forward<Args>(a)...);
    rTWPacked<PropertyWord,const Property*>(name,std::move(rd),getprop(name));
}
