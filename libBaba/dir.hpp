#pragma once
#include"tdefs.hpp"
class Direction{
    public:
        enum : uint8_t{
            UP=0u,LEFT=1u,DOWN=2u,RIGHT=3u
        };
    private:
        using _Di = decltype(UP);
        _Di d;
        friend struct std::hash<Direction>;
    public:
        pos_t axis_offset() const{
            if(d==LEFT||d==UP)return -1;
            return 1;
        }
        bool operator==(const Direction other) const{
            return d==other.d;
        }
        bool operator!=(const Direction other) const{
            return d!=other.d;
        }
        bool is_x() const{
            return d==LEFT||d==RIGHT;
        }
        bool is_y() const{
            return d==UP||d==DOWN;
        }
        constexpr Direction(_Di d) : d(d){}
        static Direction fromTo(coords_t src, coords_t dst){
            coords_t dt = dst-src;
            bool x_mjr = (std::abs(dt.x)>std::abs(dt.y));//45° is by default y-mjr
            bool negative = std::signbit(x_mjr?dt.x:dt.y);
            return (negative?(x_mjr?LEFT:UP):(x_mjr?RIGHT:DOWN));
        }
        pos_t xoff() const{
            return (d==RIGHT?1:(d==LEFT?-1:0));
        }
        pos_t yoff() const{
            return (d==DOWN?1:(d==UP?-1:0));
        }
        coords_t off() const{
            return {xoff(),yoff()};
        }
        Direction opposite() const{
            return static_cast<_Di>(uint8_t((uint8_t(d)+2u)%4u));
        }
        Direction cw() const{
            return static_cast<_Di>(uint8_t((uint8_t(d)+3u)%4u));
        }
        Direction ccw() const{
            return static_cast<_Di>(uint8_t((uint8_t(d)+1u)%4u));
        }
};
str debug(const Direction c){
    if(c==Direction::UP){
        return u8"UP"s;
    }else if(c==Direction::DOWN){
        return u8"DOWN"s;
    }else if(c==Direction::LEFT){
        return u8"LEFT"s;
    }else if(c==Direction::RIGHT){
        return u8"RIGHT"s;
    }
    return u8"Error Direction"s;
}
class WorldAction{
    std::optional<Direction> dir;
    class idle_t{};
    friend struct std::hash<WorldAction>;
    public:
        bool operator==(const WorldAction& other) const{
            return dir==other.dir;
        }
        bool operator!=(const  WorldAction& other) const{
            return dir!=other.dir;
        }
        static constexpr idle_t IDLE{};
        WorldAction(const Direction d) : dir(d){}
        WorldAction(const idle_t&) : dir(std::nullopt){}
        bool idle() const{
            return !dir.has_value();
        }
        Direction direction() const{
            return dir.value_or(Direction::LEFT);
        }
};
namespace std{
    template<>
    struct hash<::Direction>{
        size_t operator()(const ::Direction& di) const noexcept{
            return hash<uint8_t>()(di.d);
        }
    };
    template<>
    struct hash<::WorldAction>{
        size_t operator()(const ::WorldAction& pa) const noexcept{
            if(pa.dir.has_value()){
                return hash<::Direction>()(pa.dir.value());
            }
            return 12345uz;
        }
    };
}
