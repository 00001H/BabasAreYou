#include"lvl.hpp"
void find_rules_at(rlqu_t& rulz, const coords_t& c,const LevelState& ls){
    coords_t scan = c;
    pRule rul = nullptr;
    std::vector<const Word*> wds;
    bool hasw;
    //Horiz:
    while(true){
        if(!ls.cboard().contains(scan))break;
        hasw = false;
        for(const auto& obj : ls.cboard().at(scan)){
            if(hasword(obj->get_type())){
                wds.push_back(getword(obj->get_type()));
                hasw = true;
                break;//TODO: Stacked text support
            }
        }
        if(!hasw)break;
        ++scan.x;
    }
    if((rul = parse_sentence(wds))){
        rulz.emplace_back(std::move(rul));
    }
    //Vert:
    scan = c;
    wds.clear();
    while(true){
        if(!ls.cboard().contains(scan))break;
        hasw = false;
        for(const auto& obj : ls.cboard().at(scan)){
            if(words.hasKey(obj->get_type())){
                wds.emplace_back(words.lookup(obj->get_type()));
                hasw = true;
                break;//TODO: Stacked text support
            }
        }
        if(!hasw)break;
        ++scan.y;
    }
    if((rul = parse_sentence(wds))){
        rulz.emplace_back(std::move(rul));
    }
}
void find_rules(rlqu_t& rulz,const LevelState& ls,const rlqu_t* baserules){
    for(const auto& elem : ls.cobjmap()){
        if(words.hasKey(elem.first->get_type())){
            if(words.lookup(elem.first->get_type())->isSentenceStart()){
                find_rules_at(rulz,elem.second,ls);
            }
        }
    }
    if(baserules){
        for(const auto& e : *baserules){
            rulz.emplace_back(e);
        }
    }
}
void RWIState::flip(LevelState& buf){
    mvInto(buf);
    read = &buf;
    fix.clear();
    write = read->deep_copy(&fix);
    std::vector<std::pair<const Object*,pAnimation>> nanq;
    for(auto& a : anim){
        if(read->cobjmap().contains(a.first)){
            if(!(a.first = fix.at(a.first)))continue;
            a.second->fixObjects(fix);
            nanq.emplace_back(std::move(a));
        }
    }
    anim = std::move(nanq);
}
ActionResult RWIState::move(Object* bo,coords_t newpos){
    if(!fix.contains(bo)){
        throw cppp::u8_logic_error(u8"Warning: move() argument not in fix list"sv);
    }
    if(cboard().contains(newpos)){
        ActionResult rslt = ActionResult::NOTHING_CHECKED;
        for(Object* const& obj : cboard().at(newpos)){
            rslt &= obj->overlapped(*this,bo);
        }
        if(!rslt.success()){
            return rslt;
        }
    }
    add_animation(bo,pAnimation(new AniMove(where(bo))));
    write.move(fix.at(bo),newpos);
    return ActionResult::SUCCESS;
}
str Level::tick(const WorldAction& pa,rlqu_t* rq){
    clear_animations();
    size_t iter = 0u;
    bool madeProgress = true;
    bool dMadeProgress;
    RWIState rwis{&state};
    size_t pr=0u;
    ActionResult rslt;
    for(const auto& e : rwis.cobjmap()){
        e.first->on_turn_start(rwis,pa);
    }
    rwis.flip(state);
    do{
        dMadeProgress = madeProgress;
        madeProgress = false;
        for(auto& ac : state.acqu()){
            rslt = ac->exec(rwis);
            if(!rslt.success()){
                rwis.add_action(ac);
            }
            madeProgress = madeProgress||rslt.progress();
            ++iter;
            if(iter>126680u){
                throw too_complex();
            }
        }
        rwis.flip(state);
        ++pr;
        if(iter>638u){
            throw too_complex();
        }
    }while(!state.acqu().empty()&&(madeProgress||dMadeProgress));
    rwis.update_rules(baserules);
    rwis.process_rules();
    rwis.flip(state);
    rwis.execute_rules();
    rwis.flip(state);
    rwis.update_rules(baserules);
    rwis.process_rules();
    rwis.flip(state);
    for(const auto& e : rwis.cobjmap()){
        e.first->on_turn_end(rwis,pa);
    }
    for(auto& pa : rwis.anim){
        animations[pa.first].emplace_back(std::move(pa.second));
    }
    rwis.anim.clear();
    rwis.flip(state);
    if(rq){
        *rq = std::move(rwis.rulz);
    }
    return std::move(rwis.get_notice());
}
void AniMove::draw(const Object* obj,const LevelState& lg,const LevelRenderInfo& lri,const float progress) const{
    if(!lg.has_obj(obj)){
        return;
    }
    pygame::Point dst = lri.loc(lg.where(obj));
    obj->draw(lri.bbox(glm::mix(lri.loc(src),dst,progress)),1.0f);
}
