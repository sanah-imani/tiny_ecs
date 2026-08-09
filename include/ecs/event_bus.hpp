// handler list per event type

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
struct IHandlerList { virtual ~IHandlerList() = default; };

template<typename E>
struct HandlerList : IHandlerList {
    std::vector<std::function<void(const E&)>> handlers;
};

class EventBus {
    std::unordered_map<std::type_index, std::unique_ptr<IHandlerList>>
    listeners;

    std::vector<std::function<void()>> pending;

    public:
    template<typename E>
    void subscribe(std::function<void(const E&)> handler) {
        auto& list = listeners[typeid(E)];
        if (!list) list = std::make_unique<HandlerList<E>>();
        static_cast<HandlerList<E>*>(list.get())->handlers.push_back(std::move(handler));
    }

    template<typename E>
    void emit(const E& event) {
        auto it = listeners.find(typeid(E));
        if (it == listeners.end()) return;
        for (auto& handler : static_cast<HandlerList<E>*>(it->second.get())->handlers)
            handler(event);
    }

    template<typename E>
    void enqueue(const E& event) {
        pending.emplace_back([this, event]() { emit(event); });
    }

    void flush() {
        std::vector<std::function<void()>> batch;
        std::swap(batch, pending);
        for (auto& fn : batch) fn();
    }
};