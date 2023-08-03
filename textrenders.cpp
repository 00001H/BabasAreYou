#include<pygame.hpp>
#include"libBaba/baba.hpp"
using namespace pygame::event;
using namespace pygame::constants;
using namespace pygame::display;
std::string text{"BEEP"};
void onchar(GLFWwindow*,uint32_t codepoint){
    if(codepoint==' '||text_amap.contains(codepoint)){
        text += char(codepoint);
    }
}
glm::vec2 dims={1920.0f,1080.0f};
void aspectie_resz(Window&,int wi,int ht){
    glViewport(0,0,wi,ht);
    pygame::setRenderRect(wi,ht,texture_shader_colored);
    dims.x = float(wi);
    dims.y = float(ht);
}
int main(){
    pygame::init();
    pygame::glVer(4,6);
    Window wn{1600,900,"_"};
    wn.set_as_OpenGL_target();
    wn.configureRepeat(18u,3u);
    wn.onresize(aspectie_resz);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    pygame::setupTemplate0();
    ld_babatex();
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
