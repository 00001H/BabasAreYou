#include"baba.hpp"
#include"words.hpp"
#include"props.hpp"
inline void load(){
    animodes.try_emplace(u8"none"s,unew<AniSeq>());
    animodes.try_emplace(u8"char"s,unew<CharAniSeq>());

    rTPPacked<Win>(u8"win"sv,new TextRender(u8"win"sv,YELLOW,true));
    rTPPacked<Push>(u8"push"sv,new TextRender(u8"pu\nsh"sv,BROWN,true));
    rTPPacked<Sink>(u8"sink"sv,new TextRender(u8"si\nnk"sv,CYAN,true));
    rTPPacked<Stop>(u8"stop"sv,new TextRender(u8"st\nop"sv,GREEN,true));
    rTPPacked<Still>(u8"still"sv,new TextRender(u8"st\nill"sv,RED,true));
    rTPPacked<You>(u8"you"sv,new TextRender(u8"you"sv,METAPINK,true));
    rTPPacked<Select>(u8"select"sv,new TextRender(u8"sel\nect"sv,YELLOW,true));
    rTPPacked<Enterable>(u8"enterable"sv,new TextRender(u8"enter\nable"sv,BLUE,true));
    rTWPacked<Be<V_BIA::IS>>(u8"is"sv,new TextRender(u8"is"sv));
    rTWPacked<Be<V_BIA::ARE>>(u8"are"sv,new TextRender(u8"are"sv));
    rTWPacked<Be<V_BIA::BE>>(u8"be"sv,new TextRender(u8"be"sv));
    rTWPacked<TextifyNoun>(u8"text"sv,new TextRender(u8"te\nxt"sv,METAPINK));

    register_object(u8"baba"sv,new AtlasRender(AtlasItem(*atl2,1uz,u8"char"sv),WHITE),3.2f);
    register_simple_noun_and_plural(u8"baba"sv,u8"ba\nba"sv,METAPINK);
    register_object(u8"flag"sv,new TextRender(u8"F"sv,YELLOW),3.1f);
    register_simple_noun_and_plural(u8"flag"sv,u8"fla\ng"sv,YELLOW);
    register_object(u8"cross"sv,new TextRender(u8"X"sv,RED),2.2f);
    register_simple_noun_and_plural(u8"cross"sv,u8"cr\noss"sv,RED,true);
    register_object(u8"box"sv,new AtlasRender(AtlasItem(*atl2,0uz,u8"none"sv),BROWN),2.6f);
    register_simple_noun_and_plural(u8"box"sv,u8"box"sv,BROWN,true);
    
    register_object(u8"water"sv,new FilledSquareRender(CYAN));
    register_simple_noun(u8"water"sv,u8"wa\nter"sv,CYAN,UNCOUNTABLE);
    
    register_object(u8"level"sv,new TextRender(u8"L"sv,BLUE),1.1f);
    register_simple_noun_and_plural(u8"level"sv,u8"lev\nel"sv,BLUE);
    register_object(u8"cursor"sv,new TextRender(u8"O"sv,YELLOW));
    register_simple_noun_and_plural(u8"cursor"sv,u8"cur\nsor"sv,YELLOW,4.0f);
}
