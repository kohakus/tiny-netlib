#ifndef NETLIBCC_CORE_NONCOPYABLE_H_
#define NETLIBCC_CORE_NONCOPYABLE_H_

namespace netlibcc {

class Noncopyable {
public:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_NONCOPYABLE_H_