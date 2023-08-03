#pragma once
#include"idecl.hpp"
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
        template<cppp::aret_fun<const pBo&> fn>
        void allAccepts(const objmap_t&,const fn&) const;
        virtual const ObjectType* transformTarget(const BabaObject&) const = 0;
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
        virtual std::string stringify() const{
            return "<UNKNOWN>";
        }
        virtual void process(RWIState&) const{}
        virtual void exec(RWIState&) const{}
        virtual ~Rule(){}
};
class Property{
    public:
        virtual ActionResult on_moved(const pBo&,RWIState&,const Direction&) const{
            return ActionResult::NOTHING_CHECKED;
        }
        virtual ActionResult on_pushed(const pBo&,RWIState&,const Direction&,const pBo&) const{
            return ActionResult::NOTHING_CHECKED;
        }
        virtual void on_turn_start(const pBo&,RWIState&,const PlayerAction&) const{}
        virtual void on_turn_end(const pBo&,const RWIState&,const PlayerAction&) const{}
        virtual void draw_vfx(const pygame::Rect&,const BabaObject&) const{}
        virtual ~Property(){};
};
class Action{
    public:
        virtual ActionResult exec(RWIState&) const = 0;
        virtual void fixObjects(const std::unordered_map<pBo,pBo>&){}
        virtual ~Action(){}
};
class Animation{
    public:
        virtual void draw(const pBo&,const LevelState&,const LevelRenderInfo&,float progress) const = 0;
        virtual void fixObjects(const std::unordered_map<pBo,pBo>&){}
        virtual ~Animation(){}
};
