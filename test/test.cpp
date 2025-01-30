#include <iostream>
#include <iterator>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>

// 如果不能运行，多半是跟Catch2有关
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>

#include "pattern.hpp"
#include "singleton.hpp"

using namespace patterns;

// 希望你的编译器支持EBCO

namespace {
TEST_CASE("simple factory pattern") {
    SECTION("normal usage") {
        class Fruit : public simplefactory::Product<Fruit> {
        public:
            virtual void eat() const = 0;
        };

        class Apple : public Fruit {
        public:
            virtual void eat() const override { std::cout << "eat apple" << std::endl; }
        };

        class Banana : public Fruit {
        public:
            virtual void eat() const override { std::cout << "eat banana" << std::endl; }
        };

        class SimpleFruitFactory : public patterns::simplefactory::SimpleFactory<Fruit> {
        public:
            SimpleFruitFactory() = default;
        };

        auto simple_factory{ SimpleFruitFactory{} };
        auto apple{ simple_factory.Create<Apple>() };
        auto banana{ simple_factory.Create<Banana>() };
        apple->eat();
        banana->eat();
    }
}
} // namespace

namespace {
TEST_CASE("factory pattern") {
    SECTION("normal usage") {
        class Fruit : public factory::Product<Fruit> {
        public:
            virtual void eat() const = 0;
        };

        class Apple : public Fruit {
        public:
            virtual void eat() const override { std::cout << "eat apple" << std::endl; }
        };

        class Banana : public Fruit {
        public:
            virtual void eat() const override { std::cout << "eat banana" << std::endl; }
        };

        class AppleFactory : public factory::Factory<Apple> {
        public:
            AppleFactory() = default;
        };
        class BananaFactory : public factory::Factory<Banana> {
        public:
            BananaFactory() = default;
        };

        using Factory = AppleFactory;

        auto factory{ Factory{} };
        auto fruit = factory.Create();
        fruit->eat();
    }
}
} // namespace

namespace {
TEST_CASE("builder pattern") {
    SECTION("normal usage") {
        struct Meal : public builder::Product<Meal> {
        public:
            std::string_view name;
            std::string_view weight;
        };

        class MealBuilder : public builder::Builder<Meal> {};

        class ChildrenMealBuilder : public MealBuilder {
        public:
            ChildrenMealBuilder() = default;

            // 到这里指针的类型好像是在继承中丢了一次，不得不关心一下具体的类型，而不是使用auto
            virtual void Build(std::shared_ptr<Meal>& product) override {
                SetMealName(product);
                SetMealWeight(product);
            }

        private:
            void SetMealName(std::shared_ptr<Meal>& product) {
                product->name = std::string_view{ "children meal" };
            }
            void SetMealWeight(std::shared_ptr<Meal>& product) {
                product->weight = std::string_view("200g");
            }
        };

        using Builder   = ChildrenMealBuilder;
        using KFCWaiter = builder::Director;
        using Director  = KFCWaiter;

        auto builder{ Builder{} };
        auto waiter{ Director{} };

        // 理想的状态是
        // auto product{ director.Construct<Builder>() };
        // 但是这样很难让builder去动态变化

        // SetBuilder和Construct分开会导致类型擦除，然后C++就比较难构造了，得自己去弄一个映射和存储保存相关的信息

        auto meal{ waiter.Construct(builder) };
        std::cout << meal->name << "\t" << meal->weight << std::endl;
    }
}
} // namespace

namespace {
TEST_CASE("prototype") {
    SECTION("copy constructor") {
        class Apple {
        public:
            Apple()                              = default;
            Apple(Apple const& other)            = default;
            Apple& operator=(Apple const& other) = default;
            Apple(Apple&& other)                 = default;
            Apple& operator=(Apple&& other)      = default;
        };

        auto apple{ Apple{} };
        auto apple2(apple);
        auto apple3(apple);
    }

    SECTION("normal usage") {
        class Apple : public Prototype {
        public:
            Apple() = default;
            // 如果将下面的拷贝构造取消注释，直接无法编译，感觉不如直接重写拷贝构造
            // Apple(Apple const& other) = delete;

            virtual std::shared_ptr<Prototype> Clone() const override {
                return std::make_shared<Apple>(*this);
            }
        };

        auto apple{ Apple{} };
        auto apple2 = apple.Clone();
        auto apple3 = apple.Clone();
    }
}
} // namespace

namespace {
TEST_CASE("singleton") {
    SECTION("normal usage") {
        class SingletonClass : public patterns::singleton<SingletonClass> {
            friend class singleton<SingletonClass>;
            SingletonClass()  = default;
            ~SingletonClass() = default;
        };

        REQUIRE_FALSE(std::is_default_constructible_v<SingletonClass>);

        REQUIRE(&SingletonClass::instance() == &SingletonClass::instance());
    }
}
} // namespace

namespace {
TEST_CASE("adapter") {
    SECTION("normal usage") {
        class Target {
        public:
            virtual void Juice() = 0;
        };

        class Adaptee {
        public:
            void MakeJuice() { std::cout << "make juice" << std::endl; }
        };

        class Adapter : public patterns::Adapter<Target, Adaptee> {
        public:
            Adapter() = default;

            virtual void Juice() override { this->adaptee_.MakeJuice(); }
        };

        auto adapter{ Adapter{} };
        adapter.Juice();
    }
}
} // namespace

namespace {
struct Color {
    std::string_view info;
};

struct RedColor : Color {
    RedColor() { this->info = std::string_view("red"); }
};

struct BlueColor : Color {
    BlueColor() { this->info = std::string_view("blue"); }
};

class Pen {
public:
    virtual ~Pen() {
        if (color_) {
            delete color_;
            color_ = nullptr;
        }
    }

    void SetColor(Color color) {
        if (!color_)
            color_ = new bridge::SpecificBridge<Color>;
        color_->Set(color);
    }

    virtual void Draw(std::string_view object) = 0;

protected:
    Pen() : color_(nullptr) {}

    bridge::Bridge<Color>* color_;
};

class SmallPen : public Pen {
public:
    SmallPen() = default;

    virtual void Draw(std::string_view object) override {
        std::cout << "using small pen draw " << color_->Get().info << ' ' << object << std::endl;
    }
};

class MiddlePen : public Pen {
public:
    MiddlePen() = default;

    virtual void Draw(std::string_view object) override {
        std::cout << "using middle pen draw " << color_->Get().info << ' ' << object << std::endl;
    }
};

using MyColor = RedColor;
using MyPen   = SmallPen;

TEST_CASE("bridge") {
    SECTION("normal usage") {
        auto color{ MyColor{} };
        auto pen{ MyPen{} };

        pen.SetColor(color);
        pen.Draw("flowers");
    }
}
} // namespace

namespace {
class AComponent : public composite::Component<AComponent> {
public:
    AComponent() = default;
};

class ALeaf : public composite::Leaf<AComponent> {};

class AComposite : public composite::Composite<AComponent> {};
} // namespace

namespace {
template <typename T>
class ListNode {
public:
    T value;
    ListNode<T>* next = nullptr;
    ~ListNode() {
        if (next) {
            delete next;
            next = nullptr;
        }
    }
};

template <typename T>
class ListNodeIterator
    : public iterator::Iterator<ListNodeIterator<T>, T, std::forward_iterator_tag> {
    ListNode<T>* current = nullptr;

public:
    T& dereference() const { return current->value; }
};
} // namespace

namespace {
class ImageDisplayable {
public:
    virtual ~ImageDisplayable() = default;
    virtual void display()      = 0;
};

class RealImage : public ImageDisplayable {
public:
    void display() override {}
};

class ProxyImage : public ImageDisplayable {
public:
    ProxyImage() : proxy_([=]() { return std::make_unique<RealImageProxyImpl>(); }) {}

    void display() override {
        auto image = proxy_.GetObject();
        if (image) {
            image->display();
        }
    }

private:
    class RealImageProxyImpl : public RealImage {
    public:
        RealImageProxyImpl() = default;
    };

    proxy::Proxy<RealImageProxyImpl> proxy_;
};

TEST_CASE("proxy") {
    SECTION("normal usage") {
        auto image{ ProxyImage{} };
        image.display();
        image.display();
    }
}
} // namespace

namespace {
struct ComputerPart {};

class Cpu : public visitor::Element<ComputerPart> {
public:
    int price = 300;
};

class Memory : public visitor::Element<ComputerPart> {
public:
    int price = 300;
};

struct Computer {
    std::vector<visitor::Element<ComputerPart>> parts;
};

class ComputerPartVisitor : public visitor::Visitor<ComputerPart> {
public:
    ComputerPartVisitor() = default;

    virtual void visit(Cpu cpu)       = 0;
    virtual void visit(Memory memory) = 0;
};

class PriceCalculator final : public ComputerPartVisitor {
public:
    void visit(Cpu cpu) override { total_ += cpu.price; }

    void visit(Memory memory) override { total_ += memory.price; }

    int get_totol_price() const { return total_; }

private:
    int total_ = 0;
};

TEST_CASE("visitor") {
    SECTION("normal usage") { auto computer{ Computer{} }; }
}
} // namespace

namespace {

class Sorter {
public:
    void sort() { invoker_.Invoke(); }

    template <typename F>
    void set_function(F&& f) {
        invoker_.SetStrategy(std::forward<F>(f));
    }

private:
    StrategyInvoker<void()> invoker_;
};

TEST_CASE("strategy") {
    SECTION("normal usage") {
        auto sorter{ Sorter{} };
        sorter.set_function([]() {});
        sorter.sort();
    }
}
} // namespace

namespace {
class AbsCommand {};

class ConCommandA : public command::Command<AbsCommand> {
public:
    virtual void Execute() override { std::cout << "executed concrete command A" << std::endl; }
};

class ConCommandB : public command::Command<AbsCommand> {
    virtual void Execute() override { std::cout << "executed concrete command B" << std::endl; }
};

class CommandList : public command::CommandList<AbsCommand> {
public:
    CommandList() = default;
};

class Invoker : public command::Invoker<AbsCommand> {
public:
    Invoker() = default;
};

TEST_CASE("command") {
    SECTION("normal usage") {
        auto invoker{ Invoker{} };

        auto list1 = CommandList{};
        list1.Add(ConCommandA{});
        list1.Add(ConCommandA{});
        list1.Add(ConCommandB{});
        list1.Add(ConCommandA{});
        list1.Add(ConCommandA{});
        auto list2 = CommandList{};
        list2.Add(ConCommandB{});
        list2.Add(ConCommandB{});
        list2.Add(ConCommandB{});
        invoker.Invoke(list1);
        invoker.Invoke(list2);
    }
}
} // namespace

namespace {
class Thread {};

class New : public state::State<Thread> {};
class Runnable : public state::State<Thread> {};
class Dead : public state::State<Thread> {};
class Blocked : public state::State<Thread> {};
class Running : public state::State<Thread> {};

class ThreadContext {
public:
    ThreadContext() = default;

    void Start() {
        if (context_.Castable<New>()) {
            //...
        }
    }

    float GetTime() {
        if (context_.Castable<Runnable>()) {
            // ...
        }

        return 0.0f;
    }

    void Suspend() {
        if (context_.Castable<Running>()) {
            //...
        }
    }

    void Resume() {
        if (context_.Castable<Blocked>()) {
            //...
        }
    }

    void Stop() {
        if (context_.Castable<Running>()) {
            //...
        }
    }

protected:
    state::Context<Thread> context_;
};

TEST_CASE("state") {
    SECTION("normal usage") {
        auto tc = ThreadContext{};
        tc.Start();
        tc.Stop();
    }
}
} // namespace

namespace {
class TeamMember {};

class TeamLeader : public Handler<TeamMember> {
public:
    TeamLeader() = default;

    void Handle(int days) {
        if (days < 7) {
            //...
        }
        else if (next_) {
            //...
        }
        else {
            //...
        }
    }
};

class Manager : public Handler<TeamMember> {
public:
    Manager() = default;

    void Handle(int days) {
        if (days < 15) {
            //...
        }
        else if (next_) {
            //...
        }
        else {
            //...
        }
    }
};

class Ceo : public Handler<TeamMember> {
public:
    Ceo() = default;

    void Handle(int days) {
        if (days < 30) {
            //...
        }
        else if (next_) {
            //...
        }
        else {
            //...
        }
    }
};

TEST_CASE("handler") {
    SECTION("normal usage") {
        auto teamleader = TeamLeader{};
        auto manager    = Manager{};
        auto ceo        = Ceo{};

        teamleader.SetNextHandler(manager);
        manager.SetNextHandler(ceo);

        teamleader.Handle(5);

        std::tuple<int, int, int> t;
    }
}
} // namespace

namespace {
class Piece {
public:
    explicit Piece(std::string_view position) : position_(position) {}
    virtual ~Piece() = default;

protected:
    std::string_view position_;
};

class BlackPiece : public flyweight::Flyweight<Piece, std::string_view> {
public:
    BlackPiece(std::string_view position)
        : flyweight::Flyweight<Piece, std::string_view>(position) {
        std::cout << "black piece was put on position: " << position << std::endl;
    }
};

class WhitePiece : public flyweight::Flyweight<Piece, std::string_view> {
public:
    WhitePiece(std::string_view position)
        : flyweight::Flyweight<Piece, std::string_view>(position) {
        std::cout << "white piece was put on position: " << position << std::endl;
    }
};

class Factory : public flyweight::FlyweightFactory<Piece, std::string_view> {
public:
    Factory() = default;
};

TEST_CASE("flywegiht") {
    SECTION("normal usage") {
        auto factory      = Factory{};
        auto black_piece1 = factory.Get<BlackPiece>(std::string_view("12, 2"));
        auto white_piece1 = factory.Get<WhitePiece>(std::string_view("18, 18"));
    }
}
} // namespace

namespace {
class cla {};
using T            = cla;
using message_type = bool;

class Subject : public observer::Subject<Subject, message_type> {
public:
    virtual ~Subject() = default;

    explicit Subject(int value) : value(value), last_value(value) {}

    void SetState(int value) {
        last_value  = this->value;
        this->value = value;

        Notify(value > last_value);
    }

protected:
    int last_value;
    int value;
};

class ObserverR : public observer::Observer<Subject, message_type> {
public:
    ObserverR() = default;

    void Update(message_type message) {
        if (message) {
            std::cout << "yes!" << std::endl;
        }
        else {
            // 天台见
            std::cout << "no!!!!!!!" << std::endl;
        }
    }
};

class ObserverNr : public observer::Observer<Subject, message_type> {
public:
    ObserverNr() = default;

    void Update(message_type message) {
        if (!message) {
            std::cout << "yes!" << std::endl;
        }
        else {
            std::cout << "no!!!!!!!" << std::endl;
        }
    }
};

TEST_CASE("observer") {
    SECTION("normal usage") {
        auto subject1  = Subject{ 3 };
        auto subject2  = Subject{ 2 };
        auto observer1 = ObserverR{};
        auto observer2 = ObserverNr{};

        subject1.Register(observer1);
        subject1.Register(observer2);
        subject2.Register(observer1);

        subject1.SetState(4);
        subject2.SetState(1);
    }
}
} // namespace

namespace {
class Coffee {
public:
    virtual ~Coffee() = default;
};

class SimpleCoffee : public Coffee {
public:
};

class CoffeeDecorator : public decorator::Decorator<Coffee> {
public:
    using decorated_type = Coffee;
    using shared_pointer = std::shared_ptr<decorated_type>;

    CoffeeDecorator(shared_pointer pointer) : decorator::Decorator<Coffee>(pointer) {}
};

class Mocha : public CoffeeDecorator {
public:
    Mocha(shared_pointer pointer) : CoffeeDecorator(pointer) {}
};

class Whip : public CoffeeDecorator {
public:
    Whip(shared_pointer pointer) : CoffeeDecorator(pointer) {}
};

TEST_CASE("decorator") {
    SECTION("normal usage") {
        auto simple_coffee     = std::make_shared<SimpleCoffee>();
        auto mocha_coffee      = std::make_shared<Mocha>(simple_coffee);
        auto whip_mocha_coffee = std::make_shared<Whip>(mocha_coffee);
    }
}
} // namespace

namespace {
class ChatRoom : public mediator::Mediator<ChatRoom> {
public:
    ChatRoom() = default;
};

static auto on_receive_message = []() {};

class User : public mediator::Colleague<ChatRoom> {
public:
    User() = default;
};

TEST_CASE("mediator") {
    SECTION("normal usage") {
        auto user1 = User{};
        auto user2 = User{};
        auto user3 = User{};

        auto room = ChatRoom{};

        user1.SendMessage(room, user2, 114514);
    }
}
} // namespace

namespace {
TEST_CASE("memento") {
    SECTION("normal usage") {
        auto originator = memento::Originator<int>{};
        auto caretaker  = memento::Caretaker<int>{};

        originator.SetState(114);
        caretaker.SetMemento(originator.Save());
        originator.SetState(514);
        originator.Restore(caretaker.GetMemento());
    }
}
} // namespace
