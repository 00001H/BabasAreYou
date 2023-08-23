#pragma once
#include"idecl.hpp"
#include"rstry.hpp"
#include"dir.hpp"
#include"abc.hpp"
class SimpleNoun : public NounLike{
    const ObjectType* obj;
    Countability cb;
    protected:
        bool accepts(const ObjectType* ot) const override{
            return ot==obj;
        }
    public:
        virtual Countability countability() const override{
            return cb;
        }
        SimpleNoun(const ObjectType* obj,const Countability& c=SINGULAR) : obj(obj), cb(c){
        }
        bool isSentenceStart() const override{
            return true;
        }
        const ObjectType* transformTarget(const BabaObject&) const override{
            return obj;
        }
};
class Suffix{
    std::optional<str> ending;
    public:
        str operator()(const sv te,bool nl=false) const{
            str n = (nl?u8"\n"s:u8""s);
            if(ending.has_value()){
                return te+n+ending.value();
            }
            if(te.ends_with(u8's')/*glasses*/||te.ends_with(u8"se")/*horses*/||te.ends_with(u8'x')/*boxes*/||te.ends_with(u8"se")/*punches*/||te.ends_with(u8"se"sv)/*dashes*/){
                return te+n+u8"es"sv;
            }
            if(te.ends_with('y')){
                return te.substr(0u,te.length()-1u)+n+u8"ies"sv/*candies*/;
            }
            return te+n+u8's';/*letters*/
        }
        Suffix() : ending(std::nullopt){}
        Suffix(const sv suff) : ending(suff){}
};
void inline register_simple_noun(sv object_name,const sv dname,const pygame::Color& color,const Countability& c){
    rTWPacked<SimpleNoun>(object_name,unew<TextRender>(dname,color),objtype(object_name),c);
}
void inline register_simple_noun_and_plural(sv object_name,const sv dname,const pygame::Color& color,bool suffix_newline=false,const Suffix& suffix={}){
    rTWPacked<SimpleNoun>(object_name,unew<TextRender>(dname,color),objtype(object_name),SINGULAR);
    rTWPacked<SimpleNoun>(suffix(object_name),unew<TextRender>(suffix(dname,suffix_newline),color),objtype(object_name),PLURAL);
}
//WARNING: Non-owning. Please keep the referenced object alive.
class SentenceScanner{
    const Sentence& stc;
    size_t p;
    public:
        SentenceScanner(const Sentence& s) : stc(s), p(0){}
        bool done() const{
            return p>=stc.size();
        }
        const Word* next_word(){
            if(done()){
                return nullptr;
            }
            return stc[p++];
        }
        void unread(){if(p)--p;}
        void advance(){if(!done())++p;};
        template<cppp::aret_fun<SentenceScanner&> fun>
        std::invoke_result_t<fun,SentenceScanner&> safe_invoke(fun&& f) requires(std::is_pointer_v<std::invoke_result_t<fun,SentenceScanner&>>){
            size_t buf = p;
            if(auto* const r = f(*this)){
                return r;
            }
            p = buf;//restore
            return nullptr;
        }
        template<cppp::aret_fun<SentenceScanner&> fun>
        std::invoke_result_t<fun,SentenceScanner&> skip_to(fun&& f) requires(std::is_pointer_v<std::invoke_result_t<fun,SentenceScanner&>>){
            std::invoke_result_t<fun,SentenceScanner&> rval;
            while(!done()){
                if((rval = safe_invoke<fun>(std::forward<fun>(f)))){
                    return rval;
                }else{
                    advance();
                }
            }
            return nullptr;
        }
};
pRule parse_sentence(const Sentence& stc){
    for(const auto& wd : stc){
        if(const Verb* v = dynamic_cast<const Verb*>(wd)){
            return v->formRule(stc);
        }
    }
    return nullptr;
}
