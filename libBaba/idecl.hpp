#pragma once
#include"tdefs.hpp"
class ObjectType;
class Object;
using sBo = std::unique_ptr<Object>;
using board_t = cppp::dfheq_umap<coords_t,cppp::dfheq_uset<Object*>>;
using objmap_t = cppp::dfheq_umap<sBo,coords_t>;
class Action;
using pAc = std::shared_ptr<Action>;
using acqu_t = std::vector<pAc>;

class Direction;
str debug(Direction);
inline str debug(const coords_t& c){
    return u8'('+cppp::to_u8string(c.x)+u8','+cppp::to_u8string(c.y)+u8')';
}
class Animation;
using pAnimation = std::unique_ptr<Animation>;
using anqu_t = cppp::dfheq_umap<const Object*,std::vector<pAnimation>>;
class WorldAction;
class Level;
class LevelState;
class RWIState;
class Word;
using Sentence = std::vector<const Word*>;
class SentenceScanner;
class NounLike;
class PropertyWord;
class Verb;
const NounLike* nextNoun(SentenceScanner&);
const PropertyWord* nextProperty(SentenceScanner&);
class Rule;
using pRule = cppp::better_shared_ptr<Rule>;
using rlqu_t = std::vector<pRule>;
class Property;
/*
Action results can be either:
FAIL(Nothing is done)
PARTIAL(Action unsuccessful, but changes are made that may allow progress next turn)
SUCCESS(Action successful)

Action result combinations:
FAIL+FAIL = FAIL
FAIL/PARTIAL+PARTIAL/SUCCESS = PARTIAL
SUCCESS+SUCCESS = SUCCESS
*/
class ActionResult{
    public:
        enum{
            FAIL,PARTIAL,SUCCESS,NOTHING_CHECKED
        };
    private:
        using _ar_t = decltype(FAIL);
        _ar_t _f;
    public:
        ActionResult() : _f(SUCCESS){}
        ActionResult(_ar_t a) : _f(a){}
        ActionResult& operator&=(const ActionResult& o){
            if(o.empty()){
                return *this;
            }else if(empty()){
                _f = o._f;
                return *this;
            }else if(o.partial()||(fail()&&o.success())||(success()&&o.fail())){
                _f = PARTIAL;
                return *this;
            }
            return *this;
        }
        bool empty() const{
            return _f==NOTHING_CHECKED;
        }
        bool success() const{
            return _f==SUCCESS||empty();
        }
        bool partial() const{
            return _f==PARTIAL;
        }
        bool fail() const{
            return _f==FAIL;
        }
        bool progress() const{
            return !fail();
        }
};
inline size_t baba_frame(){
    return size_t(std::floor(glm::fract(glfwGetTime()*1.4f)*3.0f));
}

inline float fit(glm::vec2 obj,glm::vec2 space){
    float xc = (obj.x==0.0f?1.0f:space.x/obj.x);
    float yc = (obj.y==0.0f?1.0f:space.y/obj.y);
    return std::min(xc,yc);
}
class LevelRenderInfo{
    pygame::Point ltop;
    float tilesz;
    public:
        LevelRenderInfo(const pygame::Point& lt,const float s) : ltop(lt), tilesz(s){}
        pygame::Point pos() const{
            return ltop;
        }
        pygame::Rect bbox(const pygame::Point& loc) const{
            return {loc,dims()};
        }
        glm::vec2 dims() const{
            return {tilesz,tilesz};
        }
        float tsz() const{
            return tilesz;
        }
        pygame::Point loc(const coords_t& c) const{
            return ltop+glm::vec2(tilesz*float(c.x),tilesz*(c.y));
        }
};
