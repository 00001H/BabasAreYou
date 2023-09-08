#pragma once
#include"../libBaba/baba.hpp"
class Stop : public Property{
    public:
        ActionResult on_pushed(Object*,RWIState&,Direction,Object*) const override{
            return ActionResult::FAIL;
        }
};
class Still : public Property{
    public:
        ActionResult on_moved(Object*,RWIState&,Direction) const override{
            return ActionResult::FAIL;
        }
};
class ObjTargetAction : public Action{
    Object* obj;
    protected:
        Object* target() const{
            return obj;
        }
        ObjTargetAction(Object* t) : obj(t){}
    public:
        void fixObjects(const std::unordered_map<const Object*,Object*>& fixmap) override{
            obj = fixmap.at(obj);
        }
};
class NudgeAction : public ObjTargetAction{
    coords_t src;
    Direction di;
    public:
        NudgeAction(const RWIState& s,Object* obj,const Direction di) : ObjTargetAction(obj), src(s.where(obj)), di(di){}
        ActionResult exec(RWIState& s) const override{
            if(s.where(target())!=src){
                return ActionResult::NOTHING_CHECKED;//Dismiss quietly: do not fail then retry
            }
            ActionResult rslt = s.test_nudge(di,target());
            if(rslt.success()){
                rslt &= s.move(target(),src+di.off());
            }
            if(rslt.success()){
                s.writestate(target())->next_movement_frame();
                s.writestate(target())->rotate_to_face(di);
            }
            return rslt;
        }
};
class Push : public Property{
    public:
        ActionResult on_pushed(Object* item,RWIState& state,const Direction direction,Object*) const override{
            pAc na{new NudgeAction(state,item,direction)};
            ActionResult rslt = na->exec(state);
            if(!rslt.success()){
                state.add_action(na);
            }
            return rslt;
        }
};
class You : public Property{
    public:
        void on_turn_start(Object* obj,RWIState& state,const WorldAction& ac) const override{
            if(ac.idle()){
                return;
            }
            state.add_action(pAc(new NudgeAction(state,obj,ac.direction())));
        }
};
class Select : public You{};
class Win : public Property{
    public:
        ActionResult overlapped(Object*,RWIState&,Object* src) const override{
            if(src->has_prop(getprop(u8"you"sv))){
                throw level_win();
            }
            return ActionResult::NOTHING_CHECKED;
        }
        void draw_vfx(const pygame::Rect&,const Object&) const override{
            //TODO: Add particles
        }
};
class Sink : public Property{
    public:
        ActionResult overlapped(Object* dst,RWIState& state,Object* src) const override{
            state.remove(dst);
            state.remove(src);
            return ActionResult::SUCCESS;
        }
};
class Enterable : public Property{
    public:
        void on_turn_start(Object* dst,RWIState& wo,const WorldAction& wa) const override{
            if(wa.idle()){
                for(const auto& e : wo.overlapping(dst)){
                    if(e->has_prop(getprop(u8"select"sv))){
                        throw level_transition(dst->special());
                    }
                }
            }
        }
        void on_turn_end(const Object* dst,const RWIState& wo,const WorldAction&) const override{
            for(const auto& e : wo.overlapping(dst)){
                if(e->has_prop(getprop(u8"select"sv))){
                    wo.add_notice(dst->special());
                }
            }
        }
};
