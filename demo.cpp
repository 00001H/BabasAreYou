// #define ENABLE_PR_DEBUG
#include<UILa/comp_basic.hpp>
#include<UILa/layout.hpp>
#include"libBaba/baba.hpp"
#include"libBaba\basegameobjects.hpp"
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
enum TestingMode{
    NOTESTING,INVOKE_EDITOR,RETURN_TO_TESTING
};
void leved(Window& wn,LevelState&& initial=LevelState());
str run_level(pygame::display::Window& wn,GameState& level,sv initial_notice,TestingMode testing=NOTESTING){
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
                    }else if(kevt.is_key(GLFW_KEY_F3)){
                        if(testing==RETURN_TO_TESTING)return u8""s;
                        else if(testing==INVOKE_EDITOR){
                            leved(wn,std::move(level).movstate());
                            return u8""s;
                        }
                    }else if(kevt.is_key(GLFW_KEY_ESCAPE)){
                        return u8"!!exit"s;
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
    std::exit(0);
}
void base_rules(GameState& level){
    level.base_rule(parse_sentence({getword(u8"#text"s),getword(u8"#is"s),getword(u8"#push"s)}));
    level.base_rule(parse_sentence({getword(u8"#cursors"s),getword(u8"#are"s),getword(u8"#select"s)}));
    level.base_rule(parse_sentence({getword(u8"#levels"s),getword(u8"#are"s),getword(u8"#enterable"s)}));
}
void play(Window& wn){
    LevelSet wrld{u8"levels"s};
    str lvl{u8"l01"sv};
    while(true){
        GameState level{wrld.load_level(lvl)};
        base_rules(level);
        lvl = run_level(wn,level,level.tick(WorldAction::IDLE,nullptr,false));
        if(lvl==u8"?parent"sv){
            lvl = level.metadata().parent;
        }else if(lvl==u8"!!exit"sv){
            break;
        }
    }
}
void leved(Window& wn,LevelState&& initial){
    using namespace uila;
    const ObjectType* selected = nullptr;
    Direction seldir{Direction::RIGHT};
    pygame::Rect r{0.0f,0.0f,SW,SH};
    PFrame frm;
    PFrame* topbar{nullptr};
    std::vector<str> otks;
    for(const auto& i : object_types){otks.emplace_back(i.first);}
    std::sort(std::execution::par_unseq,otks.begin(),otks.end(),[](sv a,sv b){
        const bool was_a_text{a.starts_with(u8'#')};
        const bool was_b_text{b.starts_with(u8'#')};
        while(a.starts_with(u8'#')){
            a = a.substr(1uz);
        }
        while(b.starts_with(u8'#')){
            b = b.substr(1uz);
        }
        if(a==b){return was_b_text&&!was_a_text;}
        return a<b;
    });
    for(size_t i=0uz;i<otks.size();++i){
        if(!(i%15uz)){
            topbar = &frm.pack(frm.TOP,new PFrame());
        }
        topbar->pack(topbar->LEFT,
            new Button<ObjectSpriteUIC>(
                pygame::Color{0.03f,0.12f,0.21f,0.7f},
                {9.0f,9.0f},
                [&selected,objt=objtype(otks[i])](const MouseButtonEvent&) -> bool {
                    selected = objt;
                    return true;
                },
                objtype(otks[i]),
                Direction::RIGHT
            )
        );
    }
    EmptySpace& es{frm.pack(TOP,new EmptySpace())};
    frm.move(r);
    EventManager em{frm};
    constexpr float TILESZ{SPRITE_DIM*3.4f};
    constexpr pos_t LW{20};
    constexpr pos_t LH{10};
    constexpr float HPAD{(SW-(LW*TILESZ))/2.0f};
    LevelState lev{std::move(initial)};
    glm::vec2 lltop;
    bool inr;
    const anqu_t N{};
    bool saved = true;
    auto save = [&lev,&LW,&LH](sv fn){
        cppp::BinFile write{fn,std::ios_base::out|std::ios_base::trunc|std::ios_base::binary};
        write.write(u8"map\n"sv);
        write.write(cppp::to_u8string(LW));
        write.write(u8' ');
        write.write(cppp::to_u8string(LH));
        write.write(u8'\n');
        for(const auto& m : lev.cobjmap()){
            write.write(cppp::to_u8string(m.second.x));
            write.write(u8' ');
            write.write(cppp::to_u8string(m.second.y));
            write.write(u8' ');
            write.write(m.first->direction().ascii());
            write.write(u8' ');
            write.write(objname(m.first->get_type()));
            write.write(u8'\n');
        }
    };
    while(!wn.should_close()){
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        lltop = es.getstate().r.ltop()+glm::vec2{HPAD,0.0f};
        glfwPollEvents();
        coords_t mouspos{glm::floor((wn.mouse_pos()-lltop)/TILESZ)};
        inr = mouspos.x>=0&&mouspos.y>=0&&mouspos.x<LW&&mouspos.y<LH;
        std::vector<Event> upe{em.handle(wn.eventqueue.get())};
        frm.draw();
        lev.render({lltop,TILESZ},N,1.0f);
        if(inr){
            for(const auto& e : upe){
                if(e.type==MOUSEBUTTONUP){
                    const auto& m{e.v<MouseButtonEvent>()};
                    if(selected||m.btn!=GLFW_MOUSE_BUTTON_LEFT){
                        if(auto existing=lev.cboard().find(mouspos);existing!=lev.cboard().cend()){
                            auto tbrcopy{existing->second};
                            for(const auto& tbr : tbrcopy){
                                lev.remove(tbr);
                                saved = false;
                            }
                        }
                    }
                    if(selected&&m.btn==GLFW_MOUSE_BUTTON_LEFT){
                        lev.add(mouspos,new Object(selected,seldir));
                        saved = false;
                    }
                }else if(e.type==KEYUP){
                    const auto& ke{e.v<KeyEvent>()};
                    if(ke.is_key(GLFW_KEY_UP)){
                        seldir = Direction::UP;
                    }else if(ke.is_key(GLFW_KEY_DOWN)){
                        seldir = Direction::DOWN;
                    }else if(ke.is_key(GLFW_KEY_LEFT)){
                        seldir = Direction::LEFT;
                    }else if(ke.is_key(GLFW_KEY_RIGHT)){
                        seldir = Direction::RIGHT;
                    }else if(ke.is_key(GLFW_KEY_F3)){
                        GameState gs{
                            levelsz_t(LW),
                            levelsz_t(LH),
                            lev.deep_copy(),
                            GameState::MetaLD{
                                u8""sv
                            }
                        };
                        base_rules(gs);
                        run_level(wn,gs,u8""sv,RETURN_TO_TESTING);
                    }else if(ke.is_key(GLFW_KEY_F9)){
                        save(u8"saved.ld"sv);
                        saved = true;
                    }else if(ke.is_key(GLFW_KEY_ESCAPE)){
                        if(!saved){
                            save(u8"editor-unsaved-quit.ld"sv);
                        }
                        return;
                    }
                }
            }
            if(selected){
                selected->draw({lltop+pygame::Point(mouspos)*TILESZ,{TILESZ,TILESZ}},{.mfr=0uz,.d=seldir},0.6f);
            }
        }
        wn.tick_repeats();
        wn.swap_buffers();
    }
}
void clevsel(Window& wn){
    using namespace uila;
    Direction seldir{Direction::RIGHT};
    pygame::Rect r{0.0f,0.0f,SW,SH};
    PFrame frm;
    EventManager em{frm};
    using dit = std::filesystem::directory_iterator;
    const dit e{};
    for(dit b{std::filesystem::current_path()/u8"customs"sv};b!=e;++b){
        if(b->is_regular_file()){
            frm.pack(TOP,
                new Button<>(
                    {0.1f,0.1f,0.56f,0.8f},
                    {9.0f,9.0f},
                    [&wn,fn=b->path().u8string()](const MouseButtonEvent&){
                        GameState l{load_level(pygame::load_string_file(fn))};
                        base_rules(l);
                        run_level(wn,l,u8""sv,INVOKE_EDITOR);
                        return true;
                    },
                    b->path().filename().replace_extension().u8string(),
                    pygame::chlib.getfont(u8"MFont"s),
                    pygame::Color{0.3f,0.7f,0.2f,1.0f}
                )
            );
        }
    }
    frm.move(r);
    while(!wn.should_close()){
        glClearColor(0.23f,0.3f,0.7f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
        std::vector<Event> upe{em.handle(wn.eventqueue.get())};
        for(const auto& e : upe){
            if(e.type==KEYUP){
                if(e.v<KeyEvent>().is_key(GLFW_KEY_ESCAPE)){
                    return;
                }
            }
        }
        frm.draw();
        wn.tick_repeats();
        wn.swap_buffers();
    }
}
int main(){
    pygame::init();
{
    pygame::gl_ver(4,6);
    Window wn{1600,900,u8"_"sv};
    wn.set_as_OpenGL_target();
    wn.onresize(default_resize_fun);
    pygame::setup_template_0();
    ld_render();
    load();
    std::atexit(pygame::quit);
    using namespace uila;
    PFrame f{};
    EventManager em{f};
    pygame::chlib.loadfont(u8"MFont"s,u8"rsrc/courier_new.ttf"s).set_dimensions(60u,0u);
    f.pack(TOP,new EmptySpace({99999.9f,SH*.22f}));
    f.pack(TOP,new Button<>(
        pygame::Color{0.9f,0.77f,0.6f,1.0f},
        {9.0f,9.0f},
        [&wn](const MouseButtonEvent&){
            play(wn);
            return true;
        },
        u8"Play main game"sv,
        pygame::chlib.getfont(u8"MFont"s),
        pygame::Color{0.3f,0.7f,0.2f,1.0f}
    ));
    f.pack(TOP,new EmptySpace({99999.9f,SH*.22f}));
    f.pack(TOP,new Button<>(
        pygame::Color{0.9f,0.77f,0.6f,1.0f},
        {9.0f,9.0f},
        [&wn](const MouseButtonEvent&){
            clevsel(wn);
            return true;
        },
        u8"Play custom"sv,
        pygame::chlib.getfont(u8"MFont"s),
        pygame::Color{0.3f,0.7f,0.2f,1.0f}
    ));
    f.pack(TOP,new EmptySpace({99999.9f,SH*.22f}));
    f.pack(TOP,new Button<>(
        pygame::Color{0.9f,0.77f,0.6f,1.0f},
        {9.0f,9.0f},
        [&wn](const MouseButtonEvent&){
            leved(wn);
            return true;
        },
        u8"Level editor"sv,
        pygame::chlib.getfont(u8"MFont"s),
        pygame::Color{0.3f,0.7f,0.2f,1.0f}
    ));
    f.move({0.0f,0.0f,SW,SH});
    while(!wn.should_close()){
        glClearColor(0.23f,0.3f,0.7f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
        em.handle(wn.eventqueue.get());
        f.draw();
        wn.tick_repeats();
        wn.swap_buffers();
    }
    leved(wn);
}
    return 0;
}
