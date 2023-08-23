#pragma once
#include"tdefs.hpp"
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
        rows(tex->height()/SPRITE_DIM){
        }
        GLuint64 handle() const{
            return tex->handle();
        }
        pygame::Point leftTop(size_t sprite,size_t frame) const{
            return to_atlas_texc(sprite,frame);
        }
        pygame::Point rightBottom(size_t sprite,size_t frame) const{
            return leftTop(sprite,frame)+tex->pix2tc(spritedim,spritedim);
        }
        void draw(size_t sprite,size_t frame, pygame::Point pos, pygame::Color c, float size) const{
            pygame::Point tc_bgn = leftTop(sprite,frame);
            pygame::Point tc_end = rightBottom(sprite,frame);
            float vtx[8] = {
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 0.0f,
                1.0f, 1.0f
            };
            texture_shader_colored.use();
            texture_shader_colored.uimg("img",tex->handle());
            texture_shader_colored.uv2("position",pos);
            texture_shader_colored.u2f("imgdims",spritedim*size,spritedim*size);
            texture_shader_colored.u4f("region",tc_bgn.x,tc_bgn.y,tc_end.x,tc_end.y);
            texture_shader_colored.uv4("cmul",c);
            pygame::invoke_shader(vtx,8u,4u,texture_shader_colored,GL_TRIANGLE_STRIP,pygame::texture_vao,pygame::texture_vbo);
        }
};
std::unique_ptr<AnimationAtlas> letters=nullptr;
std::unique_ptr<AnimationAtlas> miscpic=nullptr;
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
            letters->draw(ind,frame,pos+glm::vec2{drx,dry},color,size);
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
void ld_babatex(){
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
    letters.reset(new AnimationAtlas(u8"sprites/atlas1.png"s,SPRITE_DIM,3uz));
    read_atlmap(text_amap,u8"sprites/atlas1.txt"sv);
    miscpic.reset(new AnimationAtlas(u8"sprites/atlas2.png"s,SPRITE_DIM,3uz));
}
