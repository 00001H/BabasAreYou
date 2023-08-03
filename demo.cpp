// #define ENABLE_PR_DEBUG
#include"libBaba/baba.hpp"
#include"basemod/registry.hpp"
using namespace pygame::event;
using namespace pygame::display;
glm::vec2 center(glm::vec2 content, glm::vec2 box){
    return (box-content)/2.0f;
}
const cppp::BiMap<int,PlayerAction> keyacmap = cppp::BiMap<int,PlayerAction>::of({
    {GLFW_KEY_W,{Direction::UP}},
    {GLFW_KEY_S,{Direction::DOWN}},
    {GLFW_KEY_A,{Direction::LEFT}},
    {GLFW_KEY_D,{Direction::RIGHT}},
    {GLFW_KEY_UP,{Direction::UP}},
    {GLFW_KEY_DOWN,{Direction::DOWN}},
    {GLFW_KEY_LEFT,{Direction::LEFT}},
    {GLFW_KEY_RIGHT,{Direction::RIGHT}},
    {GLFW_KEY_SPACE,{PlayerAction::IDLE}}
});
const std::string cmpl_tex{"Too Complex!\n(Z to undo, R to reset)"};
const std::string cngr_tex{"Congrunlatis!\nAlthough there is no\nlevel map\nin this demo"};
int main(){
    pygame::init();
{
    pygame::glVer(4,6);
    Window wn{1600,900,"_"};
    wn.set_as_OpenGL_target();
    wn.onresize(default_resize_fun);
    pygame::setupTemplate0();
    ld_babatex();
    load();
    GameState level = load_level(pygame::loadStringFile("levels/l01.ld"));
    level.base_rule(parse_sentence({getword("#text"),getword("#is"),getword("#push")}));
    const float scale = fit(level.dimensions(1.0f),SCRDIMS);
    const LevelRenderInfo lri{center(level.dimensions(scale),SCRDIMS),scale};
    level.tick(PlayerAction::IDLE,nullptr,false);
    pygame::time::Clock clk;
    wn.configureRepeat(13u,8u);
    std::array<float,GLFW_KEY_LAST> inputCooldown;
    inputCooldown.fill(0uz);
    while(!wn.should_close()){
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        wn.tick_repeats();
        glfwPollEvents();
        for(const Event& e : wn.eventqueue.get()){
            if(e.type>0xfb00&&e.type<0xfb04){//Any key event
                const auto& kevt = std::any_cast<KeyEvent>(e.value);
                if(e.type==KEYDOWN||e.type==KEYREPEAT){
                    if(kevt.is_key(GLFW_KEY_Z)&&level.can_undo()){
                        level.undo();
                    }else if(kevt.is_key(GLFW_KEY_R)&&level.can_undo()){
                        level.reset();
                    }
                }else{//KEYUP
                    if(kevt.is_key(GLFW_KEY_F11)){
                        wn.toggleFullscreen();
                    }
                }
                if(e.type!=KEYREPEAT&&kevt.glfw_key!=GLFW_KEY_UNKNOWN){
                    inputCooldown[kevt.glfw_key] = 0uz;
                }
            }
        }
        if(!(level.done()||level.animating())){
            for(const auto& [k,ac] : keyacmap){
                if(wn.get_key(k)&&!inputCooldown[k]){
                    inputCooldown[k] = input_interval;
                    level.tick(ac);
                    level.beginAnimation();
                }
            }
        }
        for(auto& iv : inputCooldown){
            iv -= clk.lastFrameTime();
            if(iv<0.0f){
                iv = 0.0f;
            }
        }
        if(level.err_complex()){
            babatext(cmpl_tex,METAPINK,babaFrame(),SCRCNTR,2.4f,true,true);
        }else if(level.won()){
            babatext(cngr_tex,METAPINK,babaFrame(),SCRCNTR,2.4f,true,true);
        }else{
            level.render(lri);
        }
        level.tickAnimation(clk.lastFrameTime()/input_interval*2.04f);
#if (defined ENABLE_PR_DEBUG) && !(defined LOG_STDOUT)
        babatext(DBGMSG,WHITE,babaFrame(),{HSW,0.0f},1.0f,false);
#endif
        clk.measure();
        wn.swap_buffers();
    }
}
    return 0;
}
