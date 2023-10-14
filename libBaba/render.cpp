#include"render.hpp"
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
void TextRender::render(pygame::Rect bounds,ASInfo,float tra) const{
    const pygame::Color color{_color*pygame::Color(1.0f,1.0f,1.0f,tra)};
    size_t frame = baba_frame();
    glm::vec2 size = babatext_info(text,1.0f);
    if(size.x==0.0f){
        size.x=1.0f;
    }
    if(size.y==0.0f){
        size.y=1.0f;
    }
    float mdim = std::min(bounds.width()/size.x,bounds.height()/size.y);
    if(inv){
        constexpr static size_t PROP_BG_ATL1_IDX = 40uz;
        float ddim = std::min(bounds.width(),bounds.height());
        ivfbo->bind();
        glViewport(0,0,SUBRENDER_QUALITY,SUBRENDER_QUALITY);
        glClearColor(0.0f,0.0f,0.0f,0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        pygame::set_render_rect(1.0f,1.0f,texture_shader_colored);
        babatext(text,WHITE,frame,{0.5f,0.5f},std::min(1.0f/size.x,1.0f/size.y),true,true);
        pygame::set_render_rect(1920.0f,1080.0f,texture_shader_colored);
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
        texture_sub_colored.uv2("position",bounds.pos());
        texture_sub_colored.u1f("rotation",0.0f);
        pygame::invoke_shader(4,texture_sub_colored,pygame::rect_db);
    }else{
        babatext(text,color,frame,bounds.center(),mdim,true,true);
    }
}
void babatext(const sv text,pygame::Color color,size_t frame,pygame::Point pos,float size,
bool horcntr,bool vercntr){
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
glm::vec2 babatext_info(const sv text,float size,std::vector<str>* lns,
bool ignore_overfit,bool ignore_cram){
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
