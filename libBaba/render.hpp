#pragma once
#include"idecl.hpp"
#include"abc.hpp"
#include"babatext.hpp"
#include"dir.hpp"
class RenderComponent{
    public:
        virtual void render(pygame::Rect bounds,Direction) const = 0;
        virtual ~RenderComponent(){};
};
class FilledSquareRender : public RenderComponent{
    pygame::Color color;
    public:
        FilledSquareRender(pygame::Color c) : color(c){}
        void render(pygame::Rect bounds,Direction) const override{
            pygame::draw::rect(bounds,color);
        }
};
class TextRender : public RenderComponent{
    str text;
    pygame::Color color;
    bool inv;
    public:
        TextRender(const sv text,pygame::Color c=WHITE,bool inv=false) : text(text), color(c), inv(inv){}
        void render(pygame::Rect bounds,Direction) const override{
            static constexpr float FSW = float(SPRITE_DIM);
            static constexpr float FSH = float(SPRITE_DIM);
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
                float rdim = std::min(bounds.w/FSW,bounds.h/FSH);
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
                pygame::Point lt = miscpic->leftTop(0u,frame);
                pygame::Point rb = miscpic->rightBottom(0u,frame);
                float vtx[8] = {
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f
                };
                texture_sub_colored.uv4("cmul",color);
                texture_sub_colored.u2f("imgdims",FSW*rdim,FSH*rdim);
                texture_sub_colored.uimg("img",miscpic->handle());
                texture_sub_colored.uimg("sub",subtract_buf->handle());
                texture_sub_colored.uv2("lTop",lt);
                texture_sub_colored.uv2("rBtm",rb);
                texture_sub_colored.u2f("position",bounds.x,bounds.y);
                texture_sub_colored.u1f("rotation",0.0f);
                pygame::invoke_shader(vtx,8u,4u,texture_sub_colored,GL_TRIANGLE_STRIP,
                pygame::texture_vao,pygame::texture_vbo);
            }else{
                babatext(text,color,frame,bounds.center(),mdim,true,true);
            }
        }
};
using pRenderComponent = std::unique_ptr<RenderComponent>;
class ObjectRender{
    std::vector<pRenderComponent> cpns;
    public:
        ObjectRender() : cpns(){}
        ObjectRender(pRenderComponent&& pr) : cpns(){
            cpns.emplace_back(std::move(pr));
        }
        template<std::convertible_to<pRenderComponent> ...RC>
        ObjectRender(RC&& ...a) : cpns(){
            cppp::expand(cpns,std::forward<RC>(a)...);
        }
        void add_component(RenderComponent*&& rc){
            cpns.emplace_back(rc);
        }
        void render(pygame::Rect bounds,const Direction dir) const{
            for(const auto& rc : cpns){
                rc->render(bounds,dir);
            }
        }
};
class AniMove : public Animation{
    coords_t src;
    public:
        AniMove(const coords_t& c) : src(c){}
        void draw(const pBo&,const LevelState&,const LevelRenderInfo&,float) const override;
};
