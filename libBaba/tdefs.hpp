#pragma once
#include<unordered_map>
#include<unordered_set>
#include<pygame.hpp>
#include<exception>
#include<cassert>
#include<cstdint>
#include<utility>
#include<memory>
#include<vector>
using namespace cppp::strconcat;
#ifdef NODBG
#define ITRY
#define ICATCH(z) if constexpr(false){
#define IPROPAGATE }
#else
#define ITRY try{
#define ICATCH(z) }catch(z){
#define IPROPAGATE throw;}
#endif
using namespace cppp::fsint;
using namespace pygame::constants;
using namespace std::literals;
using cppp::logging::ERR;
using cppp::logging::WARN;
using cppp::unew;
using cppp::Lazy;
using levelsz_t = uint32_t;
using pos_t = int32_t;
using str = std::u8string;
using sv = std::u8string_view;
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
class level_transition : public cppp::u8_runtime_error{
    public:
        using cppp::u8_runtime_error::u8_runtime_error;
};
const pygame::Color METAPINK{0.851f,0.224f,0.416f,1.0f};
const pygame::Color BROWN{0.588f,0.294f,0.0f,1.0f};

const pygame::Point ORIGIN{0.0f,0.0f};

cppp::logging::Logger load_logger{u8"Resource loading"sv};
cppp::logging::Logger rslv_logger{u8"Resolver"sv};

const size_t OBJECT_LIMIT = 100uz;
