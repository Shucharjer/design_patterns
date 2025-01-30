#pragma once

namespace patterns {
/**
 * @brief 单例.
 *
 * 用法: CRTP, 在派生类中将`singleton<`Derived`>`设为友元类.
 * @tparam Derived 需要成为单例的类
 */
template <typename Derived>
class singleton {
public:
    using value_type = Derived;
    using self_type  = singleton;

    singleton(const singleton&)            = delete;
    singleton(singleton&&)                 = default;
    singleton& operator=(const singleton&) = delete;
    singleton& operator=(singleton&&)      = default;

    /**
     * @brief 获取实例
     *
     * @return auto& 实例的引用
     */
    [[nodiscard]] static auto& instance() {
        static Derived inst;
        return inst;
    }

protected:
    singleton()  = default;
    ~singleton() = default;
};
} // namespace patterns
