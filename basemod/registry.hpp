#include"../libBaba/baba.hpp"
#include"words.hpp"
#include"props.hpp"
void load(){
    rTPPacked<Win>(u8"win"sv,unew<TextRender>(u8"win"sv,YELLOW,true));
    rTPPacked<Push>(u8"push"sv,unew<TextRender>(u8"pu\nsh"sv,BROWN,true));
    rTPPacked<Sink>(u8"sink"sv,unew<TextRender>(u8"si\nnk"sv,CYAN,true));
    rTPPacked<Stop>(u8"stop"sv,unew<TextRender>(u8"st\nop"sv,GREEN,true));
    rTPPacked<Still>(u8"still"sv,unew<TextRender>(u8"st\nill"sv,RED,true));
    rTPPacked<You>(u8"you"sv,unew<TextRender>(u8"you"sv,METAPINK,true));
    rTPPacked<Select>(u8"select"sv,unew<TextRender>(u8"sel\nect"sv,YELLOW,true));
    rTPPacked<Enterable>(u8"enterable"sv,unew<TextRender>(u8"enter\nable"sv,BLUE,true));
    rTWPacked<Be<V_BIA::IS>>(u8"is"sv,unew<TextRender>(u8"is"sv));
    rTWPacked<Be<V_BIA::ARE>>(u8"are"sv,unew<TextRender>(u8"are"sv));
    rTWPacked<Be<V_BIA::BE>>(u8"be"sv,unew<TextRender>(u8"be"sv));
    rTWPacked<TextifyNoun>(u8"text"sv,unew<TextRender>(u8"te\nxt"sv,METAPINK));

    register_object(u8"baba"sv,unew<TextRender>(u8"O"sv,PURPLE));
    register_simple_noun_and_plural(u8"baba"sv,u8"ba\nba"sv,METAPINK);
    register_object(u8"flag"sv,unew<TextRender>(u8"F"sv,YELLOW));
    register_simple_noun_and_plural(u8"flag"sv,u8"fla\ng"sv,YELLOW);
    register_object(u8"cross"sv,unew<TextRender>(u8"X"sv,RED));
    register_simple_noun_and_plural(u8"cross"sv,u8"cr\noss"sv,RED,true);
    register_object(u8"box"sv,unew<TextRender>(u8"#"sv,BROWN));
    register_simple_noun_and_plural(u8"box"sv,u8"box"sv,BROWN,true);
    
    register_object(u8"water"sv,unew<FilledSquareRender>(CYAN));
    register_simple_noun(u8"water"sv,u8"wa\nter"sv,CYAN,UNCOUNTABLE);
    
    register_object(u8"level"sv,unew<TextRender>(u8"L"sv,BLUE));
    register_simple_noun_and_plural(u8"level"sv,u8"lev\nel"sv,BLUE);
    register_object(u8"cursor"sv,unew<TextRender>(u8"O"sv,YELLOW));
    register_simple_noun_and_plural(u8"cursor"sv,u8"cur\nsor"sv,YELLOW);
}
