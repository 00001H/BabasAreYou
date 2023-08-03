#include"../libBaba/baba.hpp"
#include"words.hpp"
#include"props.hpp"
void load(){
    rTPPacked<Win>("win",ObjectRender::plain_text("win",YELLOW,true));
    rTPPacked<Push>("push",ObjectRender::plain_text("pu\nsh",BROWN,true));
    rTPPacked<Stop>("stop",ObjectRender::plain_text("st\nop",GREEN,true));
    rTPPacked<Still>("still",ObjectRender::plain_text("st\nill",RED,true));
    rTPPacked<You>("you",ObjectRender::plain_text("you",METAPINK,true));
    rTWPacked<Be<V_BIA::IS>>("is",ObjectRender::plain_text("is"));
    rTWPacked<Be<V_BIA::ARE>>("are",ObjectRender::plain_text("are"));
    rTWPacked<Be<V_BIA::BE>>("be",ObjectRender::plain_text("be"));
    rTWPacked<TextifyNoun>("text",ObjectRender::plain_text("te\nxt",METAPINK));
    
    register_object("baba",ObjectRender::plain_text("O",PURPLE));
    register_simple_noun_and_plural("baba","ba\nba",METAPINK);
    register_object("flag",ObjectRender::plain_text("$",YELLOW));
    register_simple_noun_and_plural("flag","fl\nag",YELLOW);
    register_object("cross",ObjectRender::plain_text("X",RED));
    register_simple_noun_and_plural("cross","cr\noss",RED,true);
    register_object("box",ObjectRender::plain_text("#",BROWN));
    register_simple_noun_and_plural("box","box",BROWN,true);
}
