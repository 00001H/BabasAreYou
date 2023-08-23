#include<memory>
#include<set>
template<typename T>
class Configurable{
    std::unique_ptr<T> value;
    using cref = const T&;
    public:
        Configurable(const Configurable& other) : value(new T(*other.value)){}
        Configurable(Configurable&&) = default;
        Configurable(T&& va) : value(new T(std::move(va))){}
        Configurable(const T& va) : value(new T(va)){}
        template<typename ...Args>
        Configurable(Args&& ...a) : value(new T(std::forward<Args>(a)...)){}
        Configurable& operator=(const Configurable& other){
            return ((*this) = (*other.value));
        }
        Configurable& operator=(Configurable&&) = default;
        T& get(){
            return *value;
        }
        const T& get() const{
            return *value;
        }
        operator T&(){
            return get();
        }
        operator cref() const{
            return get();
        }
        Configurable& operator=(T&& va){
            value.reset(new T(std::move(va)));
            return *this;
        }
        Configurable& operator=(const T& va){
            value.reset(new T(va));
            return *this;
        }
        template<typename ...Args>
        void emplace(Args&& ...a){
            value.reset(new T(std::forward<Args>(a)...));
        }
};
using key_t = std::remove_reference_t<decltype(GLFW_KEY_UNKNOWN)>;
class KeyConfig{
    std::set<key_t> keys;//a few keys: unordered_set is unneccesary
    public:
        KeyConfig() : keys(){ }
        template<std::convertible_to<key_t> ...A>
        KeyConfig(A&& ...a) : keys{a...}{}
        void add_key(key_t k){
            keys.insert(k);
        }
        bool check(pygame::display::Window& wn) const{
            for(const key_t& k : keys){
                if(wn.get_key(k)){
                    return true;
                }
            }
            return false;
        }
        void remove_key(key_t k){
            keys.erase(k);
        }
        void clear(){
            keys.clear();
        }
};

Configurable<float> input_interval{0.1f};//seconds


KeyConfig reset_key{GLFW_KEY_R};
KeyConfig undo_key{GLFW_KEY_Z};

KeyConfig up_key{GLFW_KEY_W,GLFW_KEY_UP};
KeyConfig left_key{GLFW_KEY_A,GLFW_KEY_LEFT};
KeyConfig down_key{GLFW_KEY_S,GLFW_KEY_DOWN};
KeyConfig right_key{GLFW_KEY_D,GLFW_KEY_RIGHT};
KeyConfig idle_key{GLFW_KEY_SPACE};
