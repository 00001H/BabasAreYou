#pragma once
#include<UILa/component.hpp>
#include"idecl.hpp"
#include"abc.hpp"
#include"dir.hpp"
inline pygame::Shader texture_shader_colored;
inline pygame::Shader texture_sub_colored;
class AnimationAtlas{
    pygame::sTexture tex;
    const size_t spritedim;
    const size_t frames;
    const size_t width;
    const size_t rows;
    pygame::Point to_atlas_texc(size_t spr,size_t frm) const{
        const size_t x = spr%width;
        const size_t y = (spr/width)*frames+frm;
        return tex->pix2tc(x*spritedim,y*spritedim);
    }
    public:
        AnimationAtlas(const str& fn,GLsizei SPRITE_DIM, size_t framecnt) :
        tex(pygame::loadTexture2D(fn.c_str())),
        spritedim(SPRITE_DIM),
        frames(framecnt),
        width(tex->width()/SPRITE_DIM),
        rows(tex->height()/SPRITE_DIM){}
        GLuint64 handle() const{
            return tex->handle();
        }
        pygame::Point leftTop(size_t sprite,size_t frame) const{
            return to_atlas_texc(sprite,frame);
        }
        pygame::Point rightBottom(size_t sprite,size_t frame) const{
            return leftTop(sprite,frame)+tex->pix2tc(spritedim,spritedim);
        }
        void draw(size_t sprite,size_t frame, pygame::Point pos, pygame::Color c,float size) const{
            pygame::Point tc_bgn = leftTop(sprite,frame);
            pygame::Point tc_end = rightBottom(sprite,frame);
            texture_shader_colored.use();
            texture_shader_colored.uimg("img",tex->handle());
            texture_shader_colored.uv2("position",pos);
            texture_shader_colored.u2f("imgdims",spritedim*size,spritedim*size);
            texture_shader_colored.u4f("region",tc_bgn.x,tc_bgn.y,tc_end.x,tc_end.y);
            texture_shader_colored.uv4("cmul",c);
            pygame::invoke_shader(4u,texture_shader_colored,pygame::rect_db);
        }
};
inline Lazy<AnimationAtlas> atl1;
inline Lazy<AnimationAtlas> atl2;
constexpr float TEXT_OVERFIT = 0.02f;
constexpr float TEXT_X_CRAM = 0.12f;
constexpr float TEXT_Y_CRAM = 0.00f;
constexpr size_t SPRITE_DIM = 24u;
constexpr float FSDIM = float(SPRITE_DIM);
glm::vec2 babatext_info(const sv text,float size,std::vector<str>* lns=nullptr,
bool ignore_overfit=false,bool ignore_cram=false);
const size_t SUBRENDER_QUALITY = 128u;

namespace{
    atlmap_t text_amap;
    std::unique_ptr<pygame::Framebuffer> ivfbo=nullptr;
    pygame::sTexture subtract_buf=nullptr;
}
void babatext(const sv text,pygame::Color color,size_t frame,pygame::Point pos,float size,
bool horcntr=true,bool vercntr=false);
inline void read_atlmap(atlmap_t& amap,const sv fn){
    str st = pygame::load_string_file(fn);
    size_t id = 0u;
    for(const cppp::codepoint& c : cppp::codepoints_of(st)){
        if(c==' '){
            ++id;
            continue;
        }else{
            amap.emplace(c,id);
        }
    }
}
struct ASInfo{
    size_t mfr;
    Direction d;
};
class AniSeq{
    public:
        virtual size_t offset(ASInfo&&) const{
            return 0uz;
        }
        virtual size_t mfc() const{
            return 1uz;
        }
        virtual ~AniSeq(){}
};
class CharAniSeq : public AniSeq{
    static size_t ruld(Direction&& d){
        if(d==d.RIGHT)return 0uz;
        if(d==d.UP)return 5uz;
        if(d==d.LEFT)return 10uz;
        return 15uz;
    }
    public:
        size_t offset(ASInfo&& v) const override{
            return ruld(std::move(v.d))+v.mfr+1uz;//TODO: sleep sprites
        }
        size_t mfc() const override{
            return 4uz;
        }
};
using sAniSeq = std::unique_ptr<AniSeq>;
inline cppp::dfheq_umap<str,sAniSeq> animodes;
class AtlasItem{
    AniSeq& aniseq;
    AnimationAtlas& at;
    size_t bgn;
    public:
        AtlasItem(AnimationAtlas& aa,size_t bgn,AniSeq& aniseq) : aniseq(aniseq), at(aa), bgn(bgn){}
        AtlasItem(AnimationAtlas& aa,size_t bgn,sv asn) : AtlasItem(aa,bgn,*animodes.find(asn)->second){}
        void draw(ASInfo ai,size_t frame,pygame::Point pos,pygame::Color color,float size) const{
            at.draw(bgn+aniseq.offset(std::move(ai)),frame,pos,color,size);
        }
        size_t mfc() const{
            return aniseq.mfc();
        }
};
class Render{
    public:
        virtual void render(pygame::Rect bounds,ASInfo,float) const = 0;
        virtual size_t mfcount() const{
            return 1uz;
        }
        virtual ~Render(){};
};
class FilledSquareRender : public Render{
    pygame::Color color;
    public:
        FilledSquareRender(pygame::Color c) : color(c){}
        void render(pygame::Rect bounds,ASInfo,float tra) const override{
            pygame::draw::rect(bounds,color*pygame::Color(1.0f,1.0f,1.0f,tra));
        }
};
class TextRender : public Render{
    str text;
    pygame::Color _color;
    bool inv;
    public:
        TextRender(const sv text,pygame::Color c=WHITE,bool inv=false) : text(text), _color(c), inv(inv){}
        void render(pygame::Rect,ASInfo,float tra) const override;
};
class AtlasRender : public Render{
    AtlasItem atd;
    pygame::Color color;
    public:
        template<typename at> requires(std::same_as<std::remove_cvref_t<at>,AtlasItem>)
        AtlasRender(at&& d,pygame::Color c=WHITE) : atd(std::forward<at>(d)), color(c){}
        void render(pygame::Rect bounds,ASInfo ai,float tra) const override{
            float rdim = std::min(bounds.width(),bounds.height())/FSDIM;
            atd.draw(ai,baba_frame(),bounds.ltop(),color*pygame::Color(1.0f,1.0f,1.0f,tra),rdim);
        }
        size_t mfcount() const override{
            return atd.mfc();
        }
};
using pRender = std::unique_ptr<Render>;
class AniMove : public Animation{
    coords_t src;
    public:
        AniMove(const coords_t& c) : src(c){}
        void draw(const Object*,const LevelState&,const LevelRenderInfo&,float) const override;
};
void ld_render();
