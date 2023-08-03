#version 460 core

#extension GL_ARB_bindless_texture:require

layout(location = 0) in vec2 texcoords;

out vec4 fragcolor;

layout(bindless_sampler) uniform sampler2D img;
layout(bindless_sampler) uniform sampler2D sub;

uniform vec2 lTop;
uniform vec2 rBtm;

uniform vec4 cmul;
void main(){
    vec4 bg = texture(img,mix(lTop,rBtm,texcoords));
    vec4 neg = texture(sub,vec2(texcoords.x,1.0-texcoords.y));
    if(neg.a>0)discard;
    fragcolor = bg*cmul;
}
