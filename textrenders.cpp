#include"libBaba/baba.hpp"
using namespace pygame::event;
using namespace pygame::constants;
using namespace pygame::display;
str text{u8"BEEP"sv};
void onchar(GLFWwindow*,const uint32_t codepoint){
    if(codepoint==' '||text_amap.contains(codepoint)){
        text += char(codepoint);
    }
}
glm::vec2 dims={1920.0f,1080.0f};
void aspectie_resz(Window& w){
    w.restore_viewport();
    pygame::setRenderRect(float(w.width()),float(w.height()),texture_shader_colored);
    dims.x = float(w.width());
    dims.y = float(w.width());
}
int main(){
    pygame::init();
    pygame::glVer(4,6);
    Window wn{1600,900,u8"_"sv};
    wn.set_as_OpenGL_target();
    wn.configure_repeat(18u,3u);
    wn.onresize(aspectie_resz);
    pygame::setup_template_0();
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    ld_render();
    glfwSetCharCallback(wn.glfw_handle(),onchar);
    glm::vec2 size;
    while(!wn.should_close()){
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
        wn.tick_repeats();
        for(const Event& e : wn.eventqueue.get()){
            if(e.type==KEYDOWN||e.type==KEYREPEAT){
                const auto& kevt = std::any_cast<KeyEvent>(e.value);
                if(kevt.is_key(GLFW_KEY_BACKSPACE)){
                    if(!text.empty()){
                        text.pop_back();
                    }
                }else if(kevt.is_key(GLFW_KEY_ENTER)){
                    text += '\n';
                }
            }
        }
        size = babatext_info(text,1.0f)*(1.02f+TEXT_OVERFIT);
        babatext(text,WHITE,size_t(std::floor(glm::fract(glfwGetTime()*1.4f)*3.0f)),SCRCNTR,fit(size,dims),true,true);
        wn.swap_buffers();
    }
    return 0;
}
