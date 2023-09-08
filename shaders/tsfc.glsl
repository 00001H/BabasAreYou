#version 460 core

#extension GL_ARB_bindless_texture:require

layout(location = 0) in vec2 texcoords;

out vec4 fragcolor;

layout(bindless_sampler) uniform sampler2D img;

uniform vec4 cmul;

void main(){
    fragcolor = texture(img,texcoords)*cmul;
    if(fragcolor.a<0.01){
        discard;
    }
}
