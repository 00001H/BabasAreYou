#pragma once
#include"../libBaba/baba.hpp"
class Stop : public Property{
    public:
        ActionResult on_pushed(const pBo&,RWIState&,const Direction&,const pBo&) const override{
            dbgPrint("STOP");
            return ActionResult::FAIL;
        }
};
class Still : public Property{
    public:
        ActionResult on_moved(const pBo&,RWIState&,const Direction&) const override{
            return ActionResult::FAIL;
        }
};
class ObjTargetAction : public Action{
    pBo obj;
    protected:
        const pBo& target() const{
            return obj;
        }
        ObjTargetAction(const pBo& t) : obj(t){}
    public:
        void fixObjects(const std::unordered_map<pBo,pBo>& fixmap) override{
            obj = fixmap.at(obj);
        }
};
class NudgeAction : public ObjTargetAction{
    coords_t src;
    Direction di;
    public:
        NudgeAction(const RWIState& s,const pBo& obj,const Direction& di) : ObjTargetAction(obj), src(s.where(obj)), di(di){}
        ActionResult exec(RWIState& s) const override{
            if(s.where(target())!=src){
                dbgPrint("Nudge invalid");
                return ActionResult::NOTHING_CHECKED;//Dismiss quietly: do not fail then retry
            }
            ActionResult rslt = s.test_nudge(di,target());
            if(rslt.success()){
                dbgPrint("Nudge success");
                s.move(target(),src+di.off());
            }else{
                dbgPrint("Nudge failed");
            }
            return rslt;
        }
};
class Push : public Property{
    public:
        ActionResult on_pushed(const pBo& item,RWIState& state,const Direction& direction,const pBo&) const override{
            dbgPrint("Push evaluated");
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
        void on_turn_start(const pBo& obj,RWIState& state,const PlayerAction& ac) const override{
            if(ac.idle()){
                return;
            }
            state.add_action(pAc(new NudgeAction(state,obj,ac.direction())));
        }
};
class Win : public Property{
    public:
        void on_turn_end(const pBo& obj,const RWIState& state,const PlayerAction&) const override{
            for(const auto& oobj : state.overlapping(obj)){
                if(oobj->has_prop(getprop("you"))){
                    throw level_win();
                }
            }
        }
        void draw_vfx(const pygame::Rect&,const BabaObject&) const override{
            //TODO: Add particles
        }
};
