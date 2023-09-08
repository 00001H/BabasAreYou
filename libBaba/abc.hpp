#pragma once
#include"idecl.hpp"
#include"dir.hpp"
class Word{
    public:
        virtual bool isSentenceStart() const = 0;
        virtual ~Word(){}
};
enum Countability{
    UNCOUNTABLE,SINGULAR,PLURAL,SINGULAR_AND_PLURAL
};
class NounLike : public Word{
    protected:
        virtual bool accepts(const ObjectType* ot) const = 0;
    public:
        bool isSentenceStart() const override{
            return true;
        }
        virtual Countability countability() const = 0;
        template<cppp::aret_fun<Object*> fn>
        void allAccepts(const objmap_t&,const fn&) const;
        virtual const ObjectType* transformTarget(const Object&) const = 0;
};
class Verb : public Word{
    public:
        virtual pRule formRule(const Sentence&) const{
            return nullptr;
        }
        bool isSentenceStart() const override{
            return false;
        }
        virtual void run(RWIState&) const{}
};
class PropertyWord : public Word{
    const Property* p;
    public:
        PropertyWord(const Property* p) : p(p){}
        const Property* prop() const{
            return p;
        }
        bool isSentenceStart() const override{
            return false;
        }
};
class Rule{
    public:
        virtual str stringify() const{
            return u8"<UNKNOWN>"s;
        }
        virtual void process(RWIState&) const{}
        virtual void exec(RWIState&) const{}
        virtual ~Rule(){}
};
class Property{
    public:
        virtual ActionResult on_moved([[maybe_unused]] Object* dst,RWIState&,Direction) const{
            return ActionResult::NOTHING_CHECKED;
        }
        virtual ActionResult on_pushed([[maybe_unused]] Object* dst,RWIState&,Direction,[[maybe_unused]] Object* src) const{
            return ActionResult::NOTHING_CHECKED;
        }
        virtual ActionResult overlapped([[maybe_unused]] Object* dst,RWIState&,[[maybe_unused]] Object* src) const{
            return ActionResult::NOTHING_CHECKED;
        }
        virtual void on_turn_start(Object*,RWIState&,const WorldAction&) const{}
        virtual void on_turn_end(const Object*,const RWIState&,const WorldAction&) const{}
        virtual void draw_vfx(const pygame::Rect&,const Object&) const{}
        virtual ~Property(){};
};
class Action{
    public:
        virtual ActionResult exec(RWIState&) const = 0;
        virtual void fixObjects(const std::unordered_map<const Object*,Object*>&){}
        virtual ~Action(){}
};
class Animation{
    public:
        virtual void draw(const Object*,const LevelState&,const LevelRenderInfo&,const float progress) const = 0;
        virtual void fixObjects(const std::unordered_map<const Object*,Object*>&){}
        virtual ~Animation(){}
};
