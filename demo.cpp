// #define ENABLE_PR_DEBUG
#include"libBaba/baba.hpp"
#include"basemod/registry.hpp"
using namespace pygame::event;
using namespace pygame::display;
glm::vec2 center(glm::vec2 content, glm::vec2 box){
    return (box-content)/2.0f;
}
const str cmpl_tex{u8"Too Complex!\n(Z to undo, R to reset)"s};
const str cngr_tex{u8"BEEP"s};
class InputHandler{
    float wa_cd;
    float undo_cd;
    static void _cd(float& cd,const float ft){
        cd = std::max(cd-ft,0.0f);
    }
    GameState& gs;
    std::optional<AInputAction> last;
    std::optional<AInputAction> buffered;
    public:
        InputHandler(GameState& gs) : wa_cd(0.0f), undo_cd(0.0f), gs(gs){}
        void fire(const AInputAction& aia,str& notice){
            if(aia.is_world_action()){
                if(wa_cd>0.001f){
                    if(aia!=last){
                        buffered = aia;
                    }
                    return;//ignored
                }
                wa_cd = input_interval;
                last = aia;
                gs.stop_animation();
                notice = gs.tick(aia.world_action());
                gs.begin_animation();
            }else{
                if(gs.can_undo()){
                    if(aia.special_action()==ASpecialAction::UNDO){
                        if(undo_cd>0.001f){
                            return;
                        }
                        undo_cd = input_interval;
                        gs.undo();
                    }else if(aia.special_action()==ASpecialAction::RESET){
                        gs.reset();
                    }
                }
            }
        }
        void tick(const float frame_time,str& notice){
            _cd(wa_cd,frame_time);
            if(buffered&&(wa_cd<0.001f)){
                fire(std::move(*buffered),notice);
                buffered.reset();
            }
            _cd(undo_cd,frame_time);
        }
};
void render(GameState& level,const LevelRenderInfo& lri){
    if(level.err_complex()){
        babatext(cmpl_tex,METAPINK,baba_frame(),SCRCNTR,2.4f,true,true);
    }else if(level.won()){
        babatext(cngr_tex,METAPINK,baba_frame(),SCRCNTR,2.4f,true,true);
    }else{
        level.render(lri);
    }
}
class quit_game : public std::exception{};
str run_level(pygame::display::Window& wn,GameState& level,sv initial_notice){
    try{
        const float scale = fit(level.dimensions(1.0f),SCRDIMS);
        const LevelRenderInfo lri{center(level.dimensions(scale),SCRDIMS),scale};
        pygame::time::Clock clk;
        wn.configure_repeat(13u,8u);
        InputHandler ih{level};
        float win_timer = 0.0f;
        str notice{initial_notice};
        while(!wn.should_close()){
            glClearColor(0.0f,0.0f,0.0f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            wn.tick_repeats();
            glfwPollEvents();
            for(const Event& e : wn.eventqueue.get()){
                if(e.type==KEYUP){//Any key event
                    const auto& kevt = std::any_cast<KeyEvent>(e.value);
                    if(kevt.is_key(GLFW_KEY_F11)){
                        wn.toggleFullscreen();
                    }else if(kevt.is_key(GLFW_KEY_GRAVE_ACCENT)){
                        level.ddumpobj();
                    }
                }
            }
            if(undo_key.check(wn)){
                if(!level.won())
                    ih.fire(ASpecialAction::UNDO,notice);
            }else if(reset_key.check(wn)){
                if(!level.won())
                    ih.fire(ASpecialAction::RESET,notice);
            }else if(up_key.check(wn)){
                ih.fire(WorldAction(Direction::UP),notice);
            }else if(left_key.check(wn)){
                ih.fire(WorldAction(Direction::LEFT),notice);
            }else if(down_key.check(wn)){
                ih.fire(WorldAction(Direction::DOWN),notice);
            }else if(right_key.check(wn)){
                ih.fire(WorldAction(Direction::RIGHT),notice);
            }else if(idle_key.check(wn)){
                ih.fire(WorldAction(WorldAction::IDLE),notice);
            }
            ih.tick(static_cast<float>(clk.last_frame_time()),notice);
            level.tick_animation(clk.last_frame_time()/input_interval*1.6f);
            render(level,lri);
            if(level.won()){
                win_timer += clk.last_frame_time();
                if(win_timer>1.1f){
                    return u8"?parent"s;
                }
            }
    #if (defined ENABLE_PR_DEBUG) && !(defined LOG_STDOUT)
            babatext(DBGMSG,WHITE,baba_frame(),{HSW,0.0f},1.0f,false);
    #endif
            babatext(notice,WHITE,baba_frame(),{14.0f,10.0f},1.0f,false);
            clk.measure();
            wn.swap_buffers();
        }
    }catch(level_transition& t){
        return t.u8what();
    }
    throw quit_game();
}
int play(Window& wn){
    LevelSet wrld{u8"levels"s};
    str lvl{u8"l01"sv};
    try{
        while(true){
            GameState level = wrld.load_level(lvl);
            level.base_rule(parse_sentence({getword(u8"#text"s),getword(u8"#is"s),getword(u8"#push"s)}));
            level.base_rule(parse_sentence({getword(u8"#cursors"s),getword(u8"#are"s),getword(u8"#select"s)}));
            level.base_rule(parse_sentence({getword(u8"#levels"s),getword(u8"#are"s),getword(u8"#enterable"s)}));
            lvl = run_level(wn,level,level.tick(WorldAction::IDLE,nullptr,false));
            if(lvl==u8"?parent"sv){
                lvl = level.metadata().parent;
            }
        }
    }catch(quit_game&){}
}
int main(){
    pygame::init();
{
    pygame::glVer(4,6);
    Window wn{1600,900,u8"_"sv};
    wn.set_as_OpenGL_target();
    wn.onresize(default_resize_fun);
    pygame::setup_template_0();
    ld_render();
    load();
}
    return 0;
}
