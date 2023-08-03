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
    std::optional<std::string> ending;
    public:
        std::string operator()(const std::string& te,bool nl=false) const{
            const char* const n = (nl?"\n":"");
            if(ending.has_value()){
                return te+n+ending.value();
            }
            if(te.ends_with('s')/*glasses*/||te.ends_with("se")/*horses*/||te.ends_with('x')/*boxes*/||te.ends_with("ch")/*punches*/||te.ends_with("sh")/*dashes*/){
                return te+n+"es";
            }
            if(te.ends_with('y')){
                return te.substr(0u,te.length()-1u)+n+"ies"/*candies*/;
            }
            return te+n+'s';/*letters*/
        }
        Suffix() : ending(std::nullopt){}
        Suffix(const std::string& suff) : ending(suff){}
};
void inline register_simple_noun(const std::string& object_name,const std::string& dname,const pygame::Color& color,const Countability& c){
    rTWPacked<SimpleNoun>(object_name,ObjectRender::plain_text(dname,color),objtype(object_name),c);
}
void inline register_simple_noun_and_plural(const std::string& object_name,const std::string& dname,const pygame::Color& color,bool suffix_newline=false,const Suffix& suffix={}){
    rTWPacked<SimpleNoun>(object_name,ObjectRender::plain_text(dname,color),objtype(object_name),SINGULAR);
    rTWPacked<SimpleNoun>(suffix(object_name),ObjectRender::plain_text(suffix(dname,suffix_newline),color),objtype(object_name),PLURAL);
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