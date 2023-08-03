#include<memory>
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
Configurable<float> input_interval{0.12f};
