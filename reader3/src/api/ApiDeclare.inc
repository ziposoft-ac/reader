#define API_NAME(CLASS,LIST) extern struct api##CLASS##_t { LIST } api##CLASS;
#define CMD(_X_) std::function<z_status(const _X_& data)> _X_;

API;

#undef API
#undef CMD
#undef API_NAME