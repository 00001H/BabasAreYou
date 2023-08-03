#pragma once
#include"render.hpp"
class ObjectType{
    ObjectRender render;
    float z;
    friend class Word;
    public:
        ObjectType(ObjectRender&& rndr,float z=3.0f) : render(std::move(rndr)), z(z){}
        float zl() const{
            return z;
        }
        void draw(pygame::Rect bbox, Direction di) const{
            render.render(bbox,di);
        }
};
class BabaObject{
    const ObjectType* ot;
    Direction di;
    public:
    std::unordered_set<const Property*> props;
        void reset_props(){
            props.clear();
        }
        const Direction& getDir() const{
            return di;
        }
        bool has_prop(const Property* prop){
            return props.contains(prop);
        }
        //Note: more expensive the has_prop. Use only when matching multiple types.
        template<std::derived_from<Property> T>
        bool has_prop_of_type(){
            for(const auto& pr : props){
                if(cppp::isinstanceof<T>(pr)){
                    return true;
                }
            }
            return false;
        }
        void add_prop(const Property* prop){
            props.insert(prop);
        }
        ActionResult moved(const pBo& self,RWIState& ls, const Direction& dir) const{
            ActionResult rslt = ActionResult::NOTHING_CHECKED;
            for(const auto& prop : props){
                rslt &= prop->on_moved(self,ls,dir);
            }
            return rslt;
        }
        ActionResult pushed(const pBo& self,RWIState& ls, const Direction& dir,const pBo& src) const{
            ActionResult rslt = ActionResult::NOTHING_CHECKED;
            for(const auto& prop : props){
                rslt &= prop->on_pushed(self,ls,dir,src);
            }
            return rslt;
        }
        void on_turn_start(const pBo& self,RWIState& ls,const PlayerAction& pa) const{
            for(const auto& prop : props){
                prop->on_turn_start(self,ls,pa);
            }
        }
        void on_turn_end(const pBo& self,const RWIState& ls,const PlayerAction& pa) const{
            for(const auto& prop : props){
                prop->on_turn_end(self,ls,pa);
            }
        }
        const ObjectType* get_type() const{
            return ot;
        }
        BabaObject(const ObjectType* t,Direction d=Direction::UP) : ot(t), di(d){}
        void draw(pygame::Rect bbox) const{
            ot->draw(bbox,di);
            for(const auto& prop : props){
                prop->draw_vfx(bbox,*this);
            }
        }
};
pBo npBo(const ObjectType* ot,Direction d=Direction::UP){
    return pBo(new BabaObject(ot,d));
}
inline const ObjectType* objtype(const std::string&);
pBo npBo(const std::string& objn,Direction d=Direction::UP){
    return pBo(new BabaObject(objtype(objn),d));
}
template<cppp::aret_fun<const pBo&> fn>
void NounLike::allAccepts(const objmap_t& mp,const fn& f) const{
    if(countability()==SINGULAR){
        pBo obj = nullptr;
        for(const auto& [k,v] : mp){
            if(!accepts(k->get_type()))continue;
            if(obj)return;//multiple objects: none match
            obj = k;
        }
        if(obj){
            f(obj);
        }
    }else{
        for(const auto& [k,v] : mp){
            if(accepts(k->get_type())){
                f(k);
            }
        }
    }
}
