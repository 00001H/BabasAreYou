#pragma once
#include"../libBaba/baba.hpp"
class TextifyNoun : public NounLike{
    protected:
        bool accepts(const ObjectType* ot) const override{
            return words.hasKey(ot);
        }
    public:
        Countability countability() const override{
            return UNCOUNTABLE;
        }
        bool isSentenceStart() const override{
            return true;
        }
        const ObjectType* transformTarget(const BabaObject&) const override{
            return wordobj(this);
        }
};
class Transform : public Rule{
    const NounLike* src;
    const NounLike* dst;
    public:
        std::string stringify() const override{
            return "transform "+wordname(src)+" into "+wordname(dst);
        }
        Transform(const NounLike* x, const NounLike* y) : src(x), dst(y){}
        void exec(RWIState& lvl) const override{
            src->allAccepts(lvl.cobjmap(),[&lvl,this](const pBo& obj){
                lvl.transform_object(obj,dst->transformTarget(*obj));
            });
        }
};
enum class V_BIA{
    BE,IS,ARE//No AM, sorry!
};
template<V_BIA stz>
consteval const char* BV_NAME(){
    switch(stz){
        case V_BIA::IS:
            return "IS";
        case V_BIA::ARE:
            return "ARE";
        default:
            return "BE";
    }
}
//NOTE: V_BIA template parameter is only "cosmetic", and this class will NOT check for its validity
template<V_BIA stz>
class ApplyProperty : public Rule{
    const NounLike* src;
    const PropertyWord* dst;
    public:
        ApplyProperty(const NounLike* s,const PropertyWord* p) : src(s), dst(p){}
        std::string stringify() const override{
            return wordname(src)+' '+BV_NAME<stz>()+' '+wordname(dst);
        }
        void process(RWIState& lvl) const override{
            src->allAccepts(lvl.cobjmap(),[&lvl,this](const pBo& obj){
                lvl.apply_property(obj,dst->prop());
            });
        }
};
template<V_BIA v>
constexpr inline bool compatible(Countability c){
    if constexpr(v==V_BIA::IS){
        return c != PLURAL;
    }else if constexpr(v==V_BIA::ARE){
        return c == PLURAL || c == SINGULAR_AND_PLURAL;
    }
    return true;
}
template<V_BIA v>
class Be : public Verb{
    public:
        pRule formRule(const Sentence& stc) const override{
            SentenceScanner sc{stc};
            if(stc.empty())return nullptr;
            const NounLike* subj;
            if(!(subj = sc.skip_to(&nextNoun))){
                return nullptr;
            }
            if(!compatible<v>(subj->countability())){
                return nullptr;
            }
            if(sc.next_word()!=this){
                return nullptr;
            }
            if(const NounLike* dst = sc.safe_invoke(nextNoun)){
                if(!compatible<v>(dst->countability())){
                    return nullptr;
                }
                return pRule(new Transform(subj,dst));
            }else if(const PropertyWord* dst = sc.safe_invoke(nextProperty)){
                return pRule(new ApplyProperty<v>(subj,dst));
            }
            return nullptr;
        }
};

const NounLike* nextNoun(SentenceScanner& ssc){
    return dynamic_cast<const NounLike*>(ssc.next_word());
}
const PropertyWord* nextProperty(SentenceScanner& ssc){
    return dynamic_cast<const PropertyWord*>(ssc.next_word());
}
