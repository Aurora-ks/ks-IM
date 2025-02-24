#ifndef AFX_H
#define AFX_H

#define MEM_CREATE_D(TYPE, M)                          \
public:                                                \
    template<typename T = TYPE>                        \
    std::enable_if_t<!std::is_fundamental_v<T>, void>  \
    set##M(const T &value) { M##_ = value; }           \
    template<typename T = TYPE>                        \
    std::enable_if_t<std::is_fundamental_v<T>, void>   \
    set##M(T value) { M##_ = value; }                  \
    TYPE get##M() const { return M##_; }               \
private:                                               \
    TYPE M##_;

#define MEM_CREATE_D_P(TYPE, M)                        \
public:                                                \
    void set##M(TYPE *value) { M##_ = value; }         \
    TYPE* get##M() { return M##_; }                    \
private:                                               \
    TYPE *M##_{nullptr};


#endif //AFX_H
