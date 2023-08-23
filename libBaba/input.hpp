#include<variant>
#include<vector>
#include"dir.hpp"
enum class ASpecialAction{
    UNDO,RESET
};
class bad_inputaction_access : public std::logic_error{
    using std::logic_error::logic_error;
};
class AInputAction{
    std::variant<ASpecialAction,WorldAction> action;
    public:
        AInputAction(ASpecialAction sa) : action(sa){}
        AInputAction(const WorldAction& pa) : action(pa){}
        bool is_world_action() const{
            return std::holds_alternative<WorldAction>(action);
        }
        WorldAction world_action() const{
            if(is_world_action()){
                return std::get<WorldAction>(action);
            }
            throw bad_inputaction_access("Attempting to get world action from an AInputAction holding an ASpecialAction");
        }
        ASpecialAction special_action() const{
            if(!is_world_action()){
                return std::get<ASpecialAction>(action);
            }
            throw bad_inputaction_access("Attempting to get special action from an AInputAction holding a WorldAction");
        }
        bool operator==(const AInputAction& other) const{
            return action==other.action;
        }
};
