#pragma once
#include<unordered_map>
#include<unordered_set>
#define PYGAME_NO3D
#include<pygame.hpp>
#define CPPP_NO_NUM
#define CPPP_NO_ITER
#include<exception>
#include<cppp.hpp>
#include<cassert>
#include<cstdint>
#include<utility>
#include<memory>
#include<vector>
#ifdef NODBG
#define ITRY
#define ICATCH(z) if constexpr(false){
#define IPROPAGATE }
#else
#define ITRY try{
#define ICATCH(z) }catch(z){
#define IPROPAGATE throw;}
#endif

using std::uint8_t;
using std::uint32_t;
using std::size_t;
using namespace pygame::constants;
using namespace std::literals;
using cppp::logging::ERR;
using cppp::logging::WARN;
using levelsz_t = uint32_t;
static_assert(sizeof(levelsz_t)>=sizeof(unsigned long),"Fix baba.hpp::ugetnum");
using pos_t = int32_t;
using coords_t = glm::vec<2,pos_t>;
using atlmap_t = std::unordered_map<cppp::codepoint,size_t>;
namespace std{
    template<>
    struct hash<coords_t>{
        size_t operator()(const coords_t& c) const noexcept{
            hash<pos_t> hasher;
            return hasher(c.x) ^ (hasher(c.y) << 1);
        }
    };
}
class too_complex : public std::exception{
    public:
        using std::exception::exception;
};
class level_win : public std::exception{
    public:
        using std::exception::exception;
};

const pygame::Color METAPINK{0.851f,0.224f,0.416f,1.0f};
const pygame::Color BROWN{0.588f,0.294f,0.0f,1.0f};

const pygame::Point ORIGIN{0.0f,0.0f};

cppp::logging::Logger load_logger{"Resource loading"};
cppp::logging::Logger rslv_logger{"Resolver"};

const size_t OBJECT_LIMIT = 100;
