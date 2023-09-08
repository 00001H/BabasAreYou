#pragma once
#include"idecl.hpp"
#include"abc.hpp"
#include"dir.hpp"
pygame::Shader texture_shader_colored;
pygame::Shader texture_sub_colored;
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
Lazy<AnimationAtlas> atl1;
Lazy<AnimationAtlas> atl2;
constexpr float TEXT_OVERFIT = 0.02f;
constexpr float TEXT_X_CRAM = 0.12f;
constexpr float TEXT_Y_CRAM = 0.00f;
constexpr size_t SPRITE_DIM = 24u;
constexpr float FSDIM = float(SPRITE_DIM);
glm::vec2 babatext_info(const sv text, float size,std::vector<str>* lns=nullptr,
bool ignore_overfit=false,bool ignore_cram=false){
    float ycharsz = (ignore_cram?size*FSDIM:size*FSDIM*(1.0f-TEXT_Y_CRAM));
    float xcharsz = (ignore_cram?size*FSDIM:size*FSDIM*(1.0f-TEXT_X_CRAM));
    str ln;
    float x = 0.0f;
    float maxwd = 0.0f;
    float ht = 0.0f;
    for(const char8_t& c : text){
        if(c=='\n'){
            ht += ycharsz;
            x = 0.0f;
            if(lns!=nullptr){
                lns->push_back(ln);
                ln.clear();
            }
        }else{
            x += xcharsz;
            maxwd = std::max(maxwd,x);
            if(lns!=nullptr){
                ln += c;
            }
        }
    }
    ht += ycharsz;
    if(!ln.empty()){
        if(lns!=nullptr)lns->push_back(ln);
    }
    if(ignore_overfit){
        return {maxwd,ht};
    }
    return {maxwd/(1.0f+TEXT_OVERFIT),ht/(1.0f+TEXT_OVERFIT)};
}
const size_t SUBRENDER_QUALITY = 128u;

namespace{
    atlmap_t text_amap;
    std::unique_ptr<pygame::Framebuffer> ivfbo=nullptr;
    pygame::sTexture subtract_buf=nullptr;
}
void babatext(const sv text, pygame::Color color, size_t frame, pygame::Point pos, float size,
bool horcntr=true, bool vercntr=false){
    const float SMFD = size*FSDIM;
    const float ycharsz = SMFD*(1.0f-TEXT_Y_CRAM);
    const float xcharsz = SMFD*(1.0f-TEXT_X_CRAM);
    std::vector<str> lines;
    float tpzy = babatext_info(text,size,&lines,true,true).y;
    float x;
    float y = 0.0f;
    size_t ind;
    float drx, dry, lnw;
    for(const auto& line : lines){
        const auto& cps = cppp::codepoints_of(line);
        lnw = xcharsz*float(cps.size())+TEXT_X_CRAM*SMFD;
        x = 0.0f;
        for(const cppp::codepoint& c : cps){
            if(text_amap.contains(c)){
                ind = text_amap.at(c);
            }else{
                x += xcharsz;
                continue;
            }
            drx = x;
            if(horcntr){
                drx -= lnw/2.0f;
            }
            dry = y;
            if(vercntr){
                dry -= tpzy/2.0f;
            }
            atl1->draw(ind,frame,pos+glm::vec2{drx,dry},color,size);
            x += xcharsz;
        }
        y += ycharsz;
    }
}
void read_atlmap(atlmap_t& amap,const sv fn){
    str st = pygame::loadStringFile(fn);
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
cppp::dfheq_umap<str,sAniSeq> animodes;
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
        virtual void render(pygame::Rect bounds,ASInfo) const = 0;
        virtual size_t mfcount() const{
            return 1uz;
        }
        virtual ~Render(){};
};
class FilledSquareRender : public Render{
    pygame::Color color;
    public:
        FilledSquareRender(pygame::Color c) : color(c){}
        void render(pygame::Rect bounds,ASInfo) const override{
            pygame::draw::rect(bounds,color);
        }
};
class TextRender : public Render{
    str text;
    pygame::Color color;
    bool inv;
    public:
        TextRender(const sv text,pygame::Color c=WHITE,bool inv=false) : text(text), color(c), inv(inv){}
        void render(pygame::Rect bounds,ASInfo) const override{
            size_t frame = baba_frame();
            glm::vec2 size = babatext_info(text,1.0f);
            if(size.x==0.0f){
                size.x=1.0f;
            }
            if(size.y==0.0f){
                size.y=1.0f;
            }
            float mdim = std::min(bounds.w/size.x,bounds.h/size.y);
            if(inv){
                constexpr static size_t PROP_BG_ATL1_IDX = 40uz;
                float ddim = std::min(bounds.w,bounds.h);
                ivfbo->bind();
                glViewport(0,0,SUBRENDER_QUALITY,SUBRENDER_QUALITY);
                glClearColor(0.0f,0.0f,0.0f,0.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                pygame::setRenderRect(1.0f,1.0f,texture_shader_colored);
                babatext(text,WHITE,frame,{0.5f,0.5f},std::min(1.0f/size.x,1.0f/size.y),true,true);
                pygame::setRenderRect(1920.0f,1080.0f,texture_shader_colored);
                pygame::Framebuffer::unbind();
                if(pygame::display::glCtx==nullptr){
                    throw cppp::u8_logic_error(u8"GL context window not found?!"sv);
                }
                pygame::display::glCtx->restore_viewport();
                texture_sub_colored.use();
                pygame::Point lt = atl1->leftTop(PROP_BG_ATL1_IDX,frame);
                pygame::Point rb = atl1->rightBottom(PROP_BG_ATL1_IDX,frame);
                texture_sub_colored.uv4("cmul",color);
                texture_sub_colored.u2f("imgdims",ddim,ddim);
                texture_sub_colored.uimg("img",atl1->handle());
                texture_sub_colored.uimg("sub",subtract_buf->handle());
                texture_sub_colored.uv2("lTop",lt);
                texture_sub_colored.uv2("rBtm",rb);
                texture_sub_colored.u2f("position",bounds.x,bounds.y);
                texture_sub_colored.u1f("rotation",0.0f);
                pygame::invoke_shader(4,texture_sub_colored,pygame::rect_db);
            }else{
                babatext(text,color,frame,bounds.center(),mdim,true,true);
            }
        }
};
class AtlasRender : public Render{
    AtlasItem atd;
    pygame::Color color;
    public:
        template<typename at> requires(std::same_as<std::remove_cvref_t<at>,AtlasItem>)
        AtlasRender(at&& d,pygame::Color c=WHITE) : atd(std::forward<at>(d)), color(c){}
        void render(pygame::Rect bounds,ASInfo ai) const override{
            float rdim = std::min(bounds.w,bounds.h)/FSDIM;
            atd.draw(ai,baba_frame(),bounds.ltop(),color,rdim);
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
void ld_render(){
    ivfbo.reset(new pygame::Framebuffer());
    ivfbo->bind();
    pygame::Framebuffer::unbind();
    subtract_buf.reset(new pygame::Texture(nullptr,SUBRENDER_QUALITY,SUBRENDER_QUALITY));
    ivfbo->attach_texture(*subtract_buf);
    if(!ivfbo->is_complete()){
        cppp::fcerr << u8"Incomplete Fbuf:"sv << glCheckNamedFramebufferStatus(ivfbo->id(),GL_FRAMEBUFFER) << std::endl;
        cppp::fcerr << u8"Err:"sv << glGetError() << std::endl;
        pygame::quit();
        std::abort();
    }
    texture_shader_colored.program = pygame::loadprogram(
        u8"rsrc/2d_textured_vertex.glsl"sv,
        u8"shaders/tsfc.glsl"sv
    );
    texture_sub_colored.program = pygame::loadprogram(
        u8"rsrc/2d_textured_vertex.glsl"sv,
        u8"shaders/tssubc.glsl"sv
    );
    atl1.emplace(u8"sprites/atlas1.png"s,SPRITE_DIM,3uz);
    read_atlmap(text_amap,u8"sprites/atlas1.txt"sv);
    atl2.emplace(u8"sprites/atlas2.png"s,SPRITE_DIM,3uz);
}
