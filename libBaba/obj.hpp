#pragma once
#include"render.hpp"
class ObjectType{
    pRender render;
    float z;
    friend class Word;
    public:
        ObjectType(pRender&& rndr,const float z=3.0f) : render(std::move(rndr)), z(z){}
        float zl() const{
            return z;
        }
        size_t mfc() const{
            return render->mfcount();
        }
        void draw(pygame::Rect bbox,ASInfo ai,float transparency) const{
            render->render(bbox,ai,transparency);
        }
};
class Object{
    const ObjectType* ot;
    Direction di;
    str _special;
    std::unordered_set<const Property*> props;
    size_t mvfrm;
    public:
        void next_movement_frame(){
            ++mvfrm;
            mvfrm %= ot->mfc();
        }
        sv special() const{
            return _special;
        }
        void setspecial(sv sv){
            _special = sv;
        }
        void reset_props(){
            props.clear();
        }
        const Direction& direction() const{
            return di;
        }
        void rotate_to_face(Direction nd){
            di = std::move(nd);
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
        ActionResult moved(RWIState& ls, Direction dir){
            ActionResult rslt = ActionResult::NOTHING_CHECKED;
            for(const auto& prop : props){
                rslt &= prop->on_moved(this,ls,dir);
            }
            return rslt;
        }
        ActionResult pushed(RWIState& ls, Direction dir,Object* src){
            ActionResult rslt = ActionResult::NOTHING_CHECKED;
            for(const auto& prop : props){
                rslt &= prop->on_pushed(this,ls,dir,src);
            }
            return rslt;
        }
        ActionResult overlapped(RWIState& ls,Object* src){
            ActionResult rslt = ActionResult::NOTHING_CHECKED;
            for(const auto& prop : props){
                rslt &= prop->overlapped(this,ls,src);
            }
            return rslt;
        }
        void on_turn_start(RWIState& ls,const WorldAction& pa){
            for(const auto& prop : props){
                prop->on_turn_start(this,ls,pa);
            }
        }
        void on_turn_end(const RWIState& ls,const WorldAction& pa) const{
            for(const auto& prop : props){
                prop->on_turn_end(this,ls,pa);
            }
        }
        const ObjectType* get_type() const{
            return ot;
        }
        Object(const ObjectType* t,const Direction d=Direction::UP,const sv spcl=u8""sv) : 
        ot(t), di(d), _special(spcl), mvfrm(0uz){}
        void draw(pygame::Rect bbox,float tra) const{
            ot->draw(bbox,ASInfo{.mfr=mvfrm,.d=di},tra);
            for(const auto& prop : props){
                prop->draw_vfx(bbox,*this);
            }
        }
};
inline const ObjectType* objtype(sv);
template<cppp::aret_fun<Object*> fn>
void NounLike::allAccepts(const objmap_t& mp,const fn& f) const{
    if(countability()==SINGULAR){
        Object* obj = nullptr;
        for(const auto& [k,v] : mp){
            if(!accepts(k->get_type()))continue;
            if(obj)return;//multiple objects: none match
            obj = k.get();
        }
        if(obj){
            f(obj);
        }
    }else{
        for(const auto& [k,v] : mp){
            if(accepts(k->get_type())){
                f(k.get());
            }
        }
    }
}
class ObjectSpriteUIC : public uila::UIComponent{
    Object o;
    constexpr static float SCAL{2.4f};
    public:
        template<typename ...Ea>
        ObjectSpriteUIC(Ea&& ...ea) : UIComponent(), o(std::forward<Ea>(ea)...){}
        void draw() const override{
            o.draw(state.r,1.0f);
        }
        glm::vec2 suggest_dimensions() const override{
            return {SPRITE_DIM*SCAL,SPRITE_DIM*SCAL};
        }
};
