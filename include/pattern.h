#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace pattern {
    // 用上了毕生所学的C++
    // partten realized in modern c++
    // 通过模板编程使设计模式可以被直接复用
    // 注释中涉及到的内容可能在之后多次涉及到，之后并不再提出
    // 通过将构造函数声明为protected，保证不直接使用这里提供的类（而是使用它的子类，并将子类的构造函数声明为public）
    // 到处都是模板和CRTP，就不实现模板模式了
    // 迭代器模式的模板实现中也用到了门面（外观）模式，照着C++ Templates实现的，下面会提到

    // singleton
    template <typename Derived>
    class Singleton {
    public:
        using value_type = Derived;
        using self_type = Singleton;

        virtual ~Singleton() = default;

        [[nodiscard]] static Derived &GetInstance() {
            static Derived instance;
            return instance;
        }

        Singleton(Singleton const &other) = delete;
        Singleton &operator=(Singleton const &other) = delete;

    protected:
        Singleton() = default;
    };

    namespace simplefactory {
        template <typename T>
        class Product {
        public:
            using value_type = T;
            using self_type = Product;

            virtual ~Product() = default;

        protected:
            Product() = default;
        };

        template <typename ProductType>
        requires std::is_base_of_v<Product<ProductType>, ProductType>
        class SimpleFactory {
        public:
            using product_type = ProductType;
            using self_type = SimpleFactory;

            virtual ~SimpleFactory() = default;

            template <typename SpecificProductType, typename... Args>
            requires std::is_base_of_v<ProductType, SpecificProductType>
            [[nodiscard]] std::shared_ptr<SpecificProductType> Create(Args &&...args) {
                return std::make_shared<SpecificProductType>(std::forward<Args>(args)...);
            }

        protected:
            SimpleFactory() = default;
        };
    }  // namespace simplefactory

    namespace factory {
        template <typename T>
        class Product {
        public:
            using value_type = T;
            using self_type = Product;

            virtual ~Product() = default;

        protected:
            Product() = default;
        };

        template <typename SpecificProductType>
        class Factory {
        public:
            using product_type = SpecificProductType;
            using type = Factory;

            virtual ~Factory() = default;

            template <typename... Args>
            [[nodiscard]] std::shared_ptr<SpecificProductType> Create(Args &&...args) {
                return std::make_shared<SpecificProductType>(std::forward<Args>(args)...);
            }

        protected:
            Factory() = default;
        };
    }  // namespace factory

    namespace builder {

        template <typename T>
        class Product {
        public:
            using value_type = T;
            using self_type = Product;

            virtual ~Product() = default;

        protected:
            Product() = default;
        };

        template <typename ProductType>
        requires std::is_base_of_v<Product<ProductType>, ProductType>
        class Builder {
        public:
            using product_type = ProductType;
            using self_type = Builder;

            virtual ~Builder() = default;

            virtual void Build(std::shared_ptr<ProductType> &product) = 0;

        protected:
            Builder() = default;
        };

        // Waiter
        class Director {
        public:
            virtual ~Director() = default;

            template <typename Builder>
            [[nodiscard]] auto Construct(Builder &builder) -> std::shared_ptr<typename Builder::product_type> {
                auto product{std::make_shared<typename Builder::product_type>()};
                builder.Build(product);
                return product;
            }
        };
    }  // namespace builder

    class Prototype {
    public:
        virtual ~Prototype() = default;
        virtual std::shared_ptr<Prototype> Clone() const = 0;
    };

    template <typename Target, typename Adaptee>
    class Adapter : public Target {
    public:
        virtual ~Adapter() = default;

    protected:
        Adapter() = default;
        Adaptee adaptee_;
    };

    namespace bridge {
        template <typename T>
        class Bridge {
        public:
            using value_type = T;
            using self_type = Bridge;

            virtual ~Bridge() = default;
            [[nodiscard]] T &Get() { return object_; }
            void Set(T object) { object_ = object; }

        protected:
            Bridge() = default;
            T object_;
        };

        // 类型擦除
        template <typename T>
        class SpecificBridge : public Bridge<T> {
        public:
            using value_type = T;
            using self_type = SpecificBridge<value_type>;
        };
    }  // namespace bridge

    namespace composite {
        template <typename T>
        class Component {
        public:
            using value_type = T;
            using self_type = Component;

            virtual ~Component() = default;
            virtual void Add(std::shared_ptr<Component> component) = 0;
            virtual void Remove(std::shared_ptr<Component> component) = 0;
        };

        template <typename T>
        class Leaf : public Component<T> {
        public:
            virtual void Add(std::shared_ptr<Component<T>> component) override {
                throw std::runtime_error("Cannot add to a leaf");
            }
            virtual void Remove(std::shared_ptr<Component<T>> component) override {
                throw std::runtime_error("Cannot add to a leaf");
            };
        };

        template <typename T>
        class Composite : public Component<T> {
        public:
            virtual void Add(std::shared_ptr<Component<T>> component) override { children_.push_back(component); }

            virtual void Remove(std::shared_ptr<Component<T>> component) override {
                if (auto it = std::find(children_.begin(), children_.end(), component); it != children_.end()) {
                    children_.erase(it);
                }
            }

        private:
            std::vector<std::shared_ptr<Component<T>>> children_;
        };
    }  // namespace composite

    namespace proxy {
        template <typename T>
        class Proxy : public T {
        public:
            virtual ~Proxy() = default;

            T *GetObject() {
                if (!proxied_object_) {
                    proxied_object_ = object_create_method_();
                }
                return proxied_object_.get();
            }
            Proxy(std::function<std::unique_ptr<T>()> object_create_method)
                : object_create_method_(object_create_method), proxied_object_(nullptr) {}

        private:
            std::function<std::unique_ptr<T>()> object_create_method_;
            mutable std::unique_ptr<T> proxied_object_;
        };
    }  // namespace proxy

    namespace iterator {

        // facade pattern realized iterator, referenced "C++ Templates 2nd"
        template <typename Derived, typename Value, typename Category, typename Distance = std::ptrdiff_t>
        class Iterator {
        public:
            using value_type = typename std::remove_const_t<Value>;
            using self_type = Iterator;
            using reference = Derived &;
            using pointer = Derived *;
            using difference_type = Distance;
            using iterator_category = Category;

            virtual ~Iterator() = default;

            virtual bool is_same(self_type const &other) const { return ptr_ == other.ptr_; }

            // input
            virtual reference operator*() const = 0;
            virtual pointer operator->() const = 0;
            virtual Derived &operator++() = 0;

            // both
            virtual Derived &operator--() {};

            // random
            virtual reference operator[](difference_type n) const {}
            Derived &operator+=(difference_type n) {}

        protected:
            Iterator() = default;

        private:
            pointer ptr_;
        };
    }  // namespace iterator

    namespace visitor {
        template <typename T>
        class Element {
        public:
            using value_type = T;
            using self_type = Element;

            virtual ~Element() = default;

        protected:
            Element() = default;
        };

        template <typename T>
        class Visitor {
            using value_type = T;
            using self_type = Visitor;
        };
    }  // namespace visitor

    template <typename>
    class StrategyInvoker;

    template <typename Ret, typename... Args>
    class StrategyInvoker<Ret(Args...)> {
    public:
        void SetStrategy(std::function<Ret(Args...)> strategy) { strategy_ = strategy; }

        Ret Invoke(Args &&...args) { strategy_(std::forward<Args>(args)...); }

    private:
        std::function<Ret(Args...)> strategy_;
    };

    namespace command {

        template <typename T>
        class Command {
        public:
            using self_type = Command;

            virtual ~Command() = default;
            virtual void Execute() {}

        protected:
            Command() = default;
        };

        template <typename>
        class Invoker;

        template <typename T>
        class CommandList {
            friend class Invoker<T>;

        public:
            using self_type = CommandList;

            virtual ~CommandList() = default;

            template <typename Type>
            requires std::is_base_of_v<Command<T>, std::decay_t<Type>>
            void Add(Type &&command) {
                commands_.push_back(command);
            }

        protected:
            CommandList() = default;

        private:
            std::vector<Command<T>> commands_;
        };

        template <typename T>
        class Invoker {
        public:
            using self_type = Invoker;

            virtual ~Invoker() = default;

            template <typename List>
            requires std::is_base_of_v<CommandList<T>, std::decay_t<List>>
            void Invoke(List &&list) {
                for (auto command : list.commands_) {
                    command.Execute();
                }
            }

        protected:
            Invoker() = default;
        };
    }  // namespace command

    template <typename T>
    class Handler {
    public:
        virtual void SetNextHandler(Handler<T> &next) { next_ = &next; }

    protected:
        Handler<T> *next_;
    };

    namespace state {
        template <typename T>
        class State {
        public:
            virtual ~State() = default;
            State() = default;
        };

        template <typename T>
        class Context {
        public:
            virtual ~Context() = default;

            template <typename StateType>
            requires std::is_base_of_v<State<T>, StateType>
            void Set(StateType state) {
                state_.reset(std::make_unique<StateType>());
            }

            template <typename StateType>
            bool Castable() const {
                return dynamic_cast<StateType *>(state_.get());
            }

            Context() : state_(std::make_unique<State<T>>()) {}

        protected:
            std::unique_ptr<State<T>> state_;
        };
    }  // namespace state

    namespace observer {
        template <typename T, typename MessageType>
        class Observer {
        public:
            virtual ~Observer() = default;

            virtual void Update(MessageType message) {}

        protected:
            Observer() = default;
        };

        template <typename T, typename MessageType>
        class Subject {
        public:
            virtual ~Subject() = default;

            virtual void Register(Observer<T, MessageType> &observer) {
                if (observer_ptrs.find(&observer) == observer_ptrs.end()) {
                    observer_ptrs.emplace(&observer);
                }
            }
            virtual void Remove(Observer<T, MessageType> &observer) {
                if (auto iter = observer_ptrs.find(&observer); iter != observer_ptrs.end()) {
                    observer_ptrs.erase(iter);
                }
            }
            virtual void Notify(MessageType message) {
                for (auto observer_ptr : observer_ptrs) {
                    observer_ptr->Update(message);
                }
            }

        protected:
            Subject() = default;

            std::unordered_set<Observer<T, MessageType> *> observer_ptrs;
        };
    }  // namespace observer

    namespace flyweight {
        template <typename T, typename InternalState>
        class Flyweight : public T {
        public:
            using value_type = T;
            using self_type = Flyweight;

            virtual ~Flyweight() = default;

        protected:
            Flyweight(InternalState internal_state) : T(internal_state) {}
        };

        template <typename T, typename InternalState>
        class FlyweightFactory {
        public:
            using value_type = T;
            using self_type = FlyweightFactory;

            virtual ~FlyweightFactory() = default;

            template <typename ConcreteState, typename InternalState_>
            requires std::is_base_of_v<Flyweight<T, InternalState>, ConcreteState>
            auto &Get(InternalState_ &&internal_state) {
                if (pool_.find(internal_state) == pool_.end()) {
                    pool_[internal_state] =
                        std::make_shared<ConcreteState>(std::forward<InternalState>(internal_state));
                }

                return pool_[internal_state];
            }

        protected:
            FlyweightFactory() = default;

            std::unordered_map<InternalState, std::shared_ptr<T>> pool_;
        };
    }  // namespace flyweight

    namespace mediator {

        template <typename>
        class Mediator;

        template <typename T>
        class Colleague {
        public:
            virtual ~Colleague() = default;

            template <typename Media, typename Clg, typename... Message>
            requires std::is_base_of_v<Mediator<T>, Media>
            void SendMessage(Media &mediator, Clg &colleague, Message &&...message) {
                mediator.SendMessage(this, &colleague, std::forward<Message>(message)...);
            }

            virtual void OnReceiveMessage() {}

        protected:
            Colleague() = default;
        };

        template <typename T>
        class Mediator {
        public:
            virtual ~Mediator() = default;

            template <typename... Message>
            void SendMessage(Colleague<T> *sender, Colleague<T> *recver, Message &&...message) {
                if (auto iter = colleagues_.find(recver); iter != colleagues_.end()) {
                    if (OnReceiveMessage()) {
                        recver->OnReceiveMessage();
                    }
                }
            }

            virtual bool OnReceiveMessage() { return true; }

        protected:
            Mediator() = default;

            std::unordered_set<Colleague<T> *> colleagues_;
        };
    }  // namespace mediator

    namespace memento {
        template <typename>
        class Memento;

        template <typename StateType>
        class Originator {
        public:
            virtual ~Originator() = default;

            template <typename T>
            requires std::is_same_v<StateType, std::decay_t<T>>
            void SetState(T &&state) {
                state_ = std::forward<T>(state);
            }

            [[nodiscard]] Memento<StateType> Save() const { return Memento<StateType>(state_); }

            void Restore(Memento<StateType> const &memento) { state_ = memento.GetState(); }

        protected:
            StateType state_;
        };

        template <typename StateType>
        class Memento {
        public:
            virtual ~Memento() = default;

            template <typename T>
            requires std::is_same_v<StateType, std::decay_t<T>>
            explicit Memento(T &&state) : state_(state) {}

            StateType GetState() const { return state_; }

        protected:
            StateType state_;
        };

        template <typename StateType>
        class Caretaker {
        public:
            virtual ~Caretaker() {}
            Caretaker() : memento_(nullptr) {}

            template <typename T>
            requires std::is_same_v<Memento<StateType>, std::decay_t<T>>
            void SetMemento(T &&memento) {
                memento_ = std::make_unique<Memento<StateType>>(memento);
            }

            Memento<StateType> const &GetMemento() const {
                return *static_cast<Memento<StateType> *const>(memento_.get());
            }

        protected:
            std::unique_ptr<Memento<StateType>> memento_;
        };
    }  // namespace memento

    namespace interpreter {
        class Expression {
        public:
            virtual ~Expression() = default;
            virtual void interpret(std::vector<int> &context) const = 0;

        protected:
            Expression() = default;
        };
    }  // namespace interpreter

    // wrapper
    namespace decorator {
        template <typename T>
        class Decorator : public T {
        public:
            virtual ~Decorator() = default;

        protected:
            std::shared_ptr<T> decorated_data_;

            Decorator(std::shared_ptr<T> decorated_data) : decorated_data_(decorated_data) {}
        };
    }  // namespace decorator
}  // namespace pattern