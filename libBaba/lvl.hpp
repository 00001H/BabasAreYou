#pragma once
#include"idecl.hpp"
#include"dir.hpp"
#include"obj.hpp"
#include"rstry.hpp"
#include"word.hpp"
class LevelState{
    private:
        board_t board;
        acqu_t actions;
        objmap_t objmap;
        Object* del_from_board(const Object* bo){
            coords_t loc = where(bo);
            auto it = board.at(loc).find(bo);
            Object* p = *it;
            board.at(loc).erase(it);
            if(board.at(loc).empty()){
                board.erase(loc);
            }
            return p;
        }
    public:
        void ddumpobj(){
            for(const auto& [o,p] : objmap){
                cppp::fcout << objname(o->get_type()) << u8' ' << debug(o->direction()) << u8" at "sv << debug(p) << std::endl;
            }
        }
        bool has_obj(const Object* o) const{
            return objmap.find(o)!=objmap.cend();
        }
        void clear_acqu(){
            actions.clear();
        }
        LevelState() : board(), actions(), objmap(){}
        LevelState(const LevelState&) = delete;
        LevelState(LevelState&&) = default;
        LevelState& operator=(const LevelState&) = delete;
        LevelState& operator=(LevelState&&) = default;
        void clear(){
            board.clear();
            actions.clear();
            objmap.clear();
        }
        acqu_t& acqu(){
            return actions;
        }
        const acqu_t& acqu() const{
            return actions;
        }
        void add_action(const pAc& ac){
            actions.emplace_back(ac);
        }
        size_t getcount() const{
            return objmap.size();
        }
        const board_t& cboard() const{
            return board;
        }
        const objmap_t& cobjmap() const{
            return objmap;
        }
        void add(const coords_t& c,Object*&& bo){
            if(objmap.size()>=OBJECT_LIMIT){
                objmap.clear();
                board.clear();
                throw too_complex();
            }
            board[c].emplace(bo);
            objmap.emplace(bo,c);
        }
        coords_t where(const Object* bo) const{
            return objmap.find(bo)->second;
        }
        const board_t::mapped_type overlapping(const Object* obj) const{
            return board.at(where(obj));
        }
        void remove(const Object* bo){
            if(!bo)return;
            auto it = objmap.find(bo);
            if(it==objmap.cend())return;
            del_from_board(bo);
            objmap.erase(it);
        }
        void move(const Object* bo,coords_t newpos){
            if(!bo)return;
            if(!objmap.contains(bo))return;
            board[newpos].emplace(del_from_board(bo));
            objmap.find(bo)->second = newpos;
        }
        //Note: Actions are NOT copied!(intentional)
        LevelState deep_copy(std::unordered_map<const Object*,Object*>* r=nullptr) const{
            LevelState nw;
            Object* p=nullptr;
            for(const auto& [o,c] : objmap){
                p = nw.objmap.emplace(new Object(*o),c).first->first.get();
                nw.board[c].emplace(p);
                if(r)r->emplace(o.get(),p);
            }
            return nw;
        }
        void render(const LevelRenderInfo& lri,const anqu_t& skip_animated) const{
            for(const auto& pbj : skip_animated){
                if(!cobjmap().contains(pbj.first)){
                    cppp::fcout << u8"WARN: Stray animation source"sv << std::endl;
                }
            }
            using pcm = std::pair<coords_t,const Object*>;
            using dwve = std::pair<float,pcm>;
            std::vector<dwve> dw;
            for(const auto& [loc,contents] : cboard()){
                for(const auto& obj : contents){
                    if(skip_animated.find(obj)!=skip_animated.cend())continue;
                    dw.emplace_back(obj->get_type()->zl(),pcm(loc,obj));
                }
            }
            std::sort(dw.begin(),dw.end(),[](const dwve& a,const dwve& b){
                return a.first<b.first;
            });
            for(const auto& e : dw){
                e.second.second->draw({lri.loc(e.second.first),lri.dims()});
            }
        }
};
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
void find_rules(rlqu_t& rulz,const LevelState& ls,const rlqu_t* baserules=nullptr){
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
class RWIState{
    std::unordered_map<const Object*,Object*> fix;
    const LevelState* read;
    LevelState write;
    rlqu_t rulz;
    friend class Level;
    LevelState done_state(){
        return std::move(write);
    }
    void mvInto(LevelState& buf){
        buf = std::move(write);
    
    }
    std::vector<std::pair<const Object*,pAnimation>> anim;
/*
Mutability of `notice`: What does `const RWIState` really mean?
Currently, `Property::on_turn_end` takes a `const RWIState&`. This is
reasonable: any writes to it will not actually take effect, so there
is no reason to allow any writes in the first place, as making it
writable might confuse users. However, `notice` came along; `notice`
is NOT r/w isolated, and changing it in `Property::on_turn_end` is
actually reasonable(and useful). Since the philosophy of `const` is
to improve maintainability, I decided that making `notice` `mutable`
is better: the alternative would be to make functions like `on_turn_end`
take a mutable `RWIState&`. Then, we would have to seperately document
that you should not change it(except for adding notices) in that method.
*/
    mutable str notice;
    public:
        RWIState(const LevelState* ls) : fix(), read(ls), write(read->deep_copy(&fix)){}
        RWIState(const RWIState&) = delete;
        RWIState& operator=(const RWIState&) = delete;
        RWIState(RWIState&&) = default;
        void add_notice(sv ntc) const{
            notice += u8"\n"sv+ntc;
        }
        Object* writestate(const Object* r) const{
            return fix.at(r);
        }
        str& get_notice() const{
            return notice;
        }
        void add_action(const pAc& ac){
            write.add_action(ac);
            write.acqu().back()->fixObjects(fix);
        }
        void add_animation(const Object* bo,pAnimation&& pa){
            Object* f = fix.at(bo);
            if(!f)return;
            anim.emplace_back(f,std::move(pa));
            anim.back().second->fixObjects(fix);
        }
        ActionResult test_nudge(const Direction ndg,Object* obj){
            ActionResult mr = obj->moved(*this,ndg);
            if(mr.success()){
                coords_t dest = cobjmap().find(obj)->second+ndg.off();
                if(cboard().contains(dest)){
                    for(const auto& mobj : cboard().at(dest)){
                        mr &= mobj->pushed(*this,ndg,obj);
                        //do NOT break until all processing is done for this space
                    }
                }
            }
            return mr;
        }
        void flip(LevelState& buf){
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
        const acqu_t& acq() const{
            return read->acqu();
        }
        void update_rules(const rlqu_t& baserules){
            rulz.clear();
            find_rules(rulz,*read,&baserules);
        }
        void process_rules(){
            for(const auto& e : cobjmap()){
                if(Object* f=fix.at(e.first.get()))f->reset_props();
            }
            for(auto& rl : rulz){
                rl->process(*this);
            }
        }
        void execute_rules(){
            for(auto& rl : rulz){
                rl->exec(*this);
            }
        }
        void apply_property(const Object* src,const Property* pp){
            if(Object* d=fix.at(src))d->add_prop(pp);
        }
        void transform_object(const Object* obj,const ObjectType* dst){
            if(read->cobjmap().contains(obj)){
                write.remove(fix.at(obj));
                fix.at(obj) = nullptr;
                write.add(read->cobjmap().find(obj)->second,new Object(dst,obj->direction(),obj->special()));
            }
        }
        size_t getcount() const{
            return read->getcount();
        }
        const board_t& cboard() const{
            return read->cboard();
        }
        const board_t::mapped_type overlapping(const Object* obj) const{
            return read->overlapping(obj);
        }
        const objmap_t& cobjmap() const{
            return read->cobjmap();
        }
        coords_t where(const Object* bo) const{
            return read->where(bo);
        }
        void add(const coords_t& c,Object*&& b){
            write.add(c,std::move(b));
        }
        void remove(const Object* bo){
            if(!fix.contains(bo)){
                cppp::fcerr << u8"Warning: remove() argument not in transformation list"sv << std::endl;
                return;
            }
            write.remove(fix.at(bo));
        }
        ActionResult move(Object* bo,coords_t newpos){
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
};
class Level{
    private:
        LevelState state;
        levelsz_t w,h;
        rlqu_t baserules;
        anqu_t animations;
    public:
        void ddumpobj(){
            cppp::fcout << u8"--Level dump("sv << w << u8'x' << h << u8"):\n~  Objdump:\n"sv;
            state.ddumpobj();
            cppp::fcout << u8"~  Base rules:\n"sv;
            for(const auto& e : baserules){
                cppp::fcout << e->stringify() << u8'\n';
            }
            cppp::fcout << u8"--\n"sv;
        }
        const board_t& board() const{
            return state.cboard();
        }
        const objmap_t& posmap() const{
            return state.cobjmap();
        }
        void clear_animations(){
            animations.clear();
        }
        bool has_obj(const Object* o) const{
            return state.has_obj(o);
        }
        void clear(){
            state.clear();
            clear_animations();
        }
        void find_rules(rlqu_t& rq){
            ::find_rules(rq,state,&baserules);
        }
        Level(const levelsz_t& w,const levelsz_t& h) : state(), w(w), h(h){
        }
        void base_rule(const pRule& pr){
            assert(pr);
            baserules.emplace_back(pr);
        }
        LevelState dup_state() const{
            return state.deep_copy();
        }
        /*
        Level processing procedure:
        1. Process actions
          ·Add player action to the first round
        2. Parse and process rules
        3. Execute rules
        4. Reparse and reprocess rules
          ·Needed to ensure the player action processing of the next round is done properly
          ·Don't move this check to before the action processing of the next round, as
           doing it this way also allows us to return the newest rules for display
        */
        str tick(const WorldAction& pa,rlqu_t* rq=nullptr){
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
        void set_state(LevelState&& stt){
            animations.clear();
            state = std::move(stt);
        }
        template<cppp::function_type<void,Object*> fun>
        void for_each_object(fun each){
            for(auto& elem : board()){
                for(auto& obj : elem.second){
                    each(obj);
                }
            }
        }
        glm::vec2 dimensions(const float size) const{
            return {float(w)*size,float(h)*size};
        }
        void add(const coords_t& c,Object*&& b){
            state.add(c,std::move(b));
        }
        void render(const LevelRenderInfo& lri,const float animate=0.0f) const{
            state.render(lri,animations);
            for(const auto& [k,v] : animations){
                for(const auto& anim : v){
                    anim->draw(k,state,lri,animate);
                }
            }
        }
};
void AniMove::draw(const Object* obj,const LevelState& lg,const LevelRenderInfo& lri,const float progress) const{
    if(!lg.has_obj(obj)){
        return;
    }
    pygame::Point dst = lri.loc(lg.where(obj));
    obj->draw(lri.bbox(glm::mix(lri.loc(src),dst,progress)));
}
class GameState{
    Level lvl;
    std::vector<LevelState> past;
    bool _won;
    bool complex;
    bool _animating=false;
    float animate=0.0f;
    void _state_reset(){
        _won = complex = false;
    }
    public:
        struct MetaLD{
            str parent;
            MetaLD(sv v) : parent(v){}
        };
    private:
        MetaLD meta;
    public:
        GameState(const levelsz_t& w,const levelsz_t& h,const MetaLD& mt) : lvl(w,h), past(), _won(false), complex(false), meta(mt){}
        GameState(const levelsz_t& w,const levelsz_t& h,MetaLD&& mt) : lvl(w,h), past(), _won(false), complex(false), meta(std::move(mt)){}
        MetaLD& metadata(){
            return meta;
        }
        const MetaLD& metadata() const{
            return meta;
        }
        const board_t& board() const{
            return lvl.board();
        }
        const objmap_t& objmap() const{
            return lvl.posmap();
        }
        void add(const coords_t& c,Object*&& bo){
            try{
                lvl.add(c,std::move(bo));
            }catch(too_complex&){
                lvl.clear();
                complex = true;
            }
        }
        void ddumpobj(){
            lvl.ddumpobj();
        }
        void base_rule(const pRule& pr){
            lvl.base_rule(pr);
        }
        glm::vec2 dimensions(const float scale) const{
            return lvl.dimensions(scale);
        }
        void undo(rlqu_t* rq=nullptr){
            if(!can_undo())return;
            lvl.set_state(std::move(past.back()));
            past.pop_back();
            _state_reset();
            if(rq){
                lvl.find_rules(*rq);
            }
        }
        void reset(rlqu_t* rq=nullptr){
            if(!can_undo())return;
            lvl.set_state(std::move(past.front()));
            past.clear();
            _state_reset();
            if(rq){
                lvl.find_rules(*rq);
            }
        }
        bool can_undo() const{
            return !past.empty();
        }
        bool err_complex() const{
            return complex;
        }
        bool won() const{
            return _won;
        }
        bool done() const{
            return _won||complex;
        }
        void begin_animation(){
            _animating = true;
            animate = 0.0f;
        }
        void tick_animation(const float amount){
            if(!_animating)return;
            animate += amount;
            if(animate>=1.0f){
                stop_animation();
            }
        }
        void stop_animation(){
            _animating = false;
            animate = 0.0f;
            lvl.clear_animations();
        }
        bool animating() const{
            return _animating;
        }
        str tick(const WorldAction& pa,rlqu_t* rq=nullptr,bool history=true){
            if(history){
                past.emplace_back(lvl.dup_state());
            }
            try{
                return lvl.tick(pa,rq);
            }catch(too_complex&){
                complex = true;
            }catch(level_win&){
                _won = true;
            }
            return u8""s;
        }
        void render(const LevelRenderInfo& lri) const{
            lvl.render(lri,animate);
        }
};
