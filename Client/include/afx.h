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

#define QPROPERTY_CREATE(TYPE, M)                             \
    Q_PROPERTY(TYPE p##M MEMBER p##M##_ NOTIFY p##M##Changed) \
public:                                                       \
    Q_SIGNAL void p##M##Changed();                            \
    void set##M(TYPE M){                                      \
        p##M##_ = M;                                          \
        Q_EMIT p##M##Changed();                               \
    }                                                         \
    TYPE get##M() const{                                      \
        return p##M##_;                                       \
    }                                                         \
private:                                                      \
    TYPE p##M##_;

#endif //AFX_H
