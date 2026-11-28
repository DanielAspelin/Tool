namespace mainframes {
constexpr unsigned abiVersion = 1;
constexpr unsigned maxPayloadBytes = 4096;

enum class Action : unsigned { enter = 1, input, passThrough, output, exit, close };
enum class State : unsigned { idle, entered, accepted, traversed, returned, exited, closed };
enum class Operation : unsigned { none, echo };
enum class Result : unsigned { rejected, accepted };

class Passage {
    State state_ = State::idle;
    unsigned handle_ = 0;

public:
    State state() const { return state_; }

    Result step(Action action, unsigned handle, Operation operation, unsigned payloadBytes) {
        if (handle == 0 || state_ == State::closed) return Result::rejected;
        if (state_ != State::idle && handle != handle_) return Result::rejected;
        if (action == Action::input) {
            if (state_ != State::entered || operation != Operation::echo ||
                payloadBytes > maxPayloadBytes) return Result::rejected;
            state_ = State::accepted;
            return Result::accepted;
        }
        if (operation != Operation::none || payloadBytes != 0) return Result::rejected;

        switch (action) {
        case Action::enter:
            if (state_ != State::idle) return Result::rejected;
            handle_ = handle;
            state_ = State::entered;
            break;
        case Action::passThrough:
            if (state_ != State::accepted) return Result::rejected;
            state_ = State::traversed;
            break;
        case Action::output:
            if (state_ != State::traversed) return Result::rejected;
            state_ = State::returned;
            break;
        case Action::exit:
            if (state_ != State::returned) return Result::rejected;
            state_ = State::exited;
            break;
        case Action::close:
            if (state_ == State::idle) return Result::rejected;
            handle_ = 0;
            state_ = State::closed;
            break;
        default:
            return Result::rejected;
        }
        return Result::accepted;
    }
};
}

