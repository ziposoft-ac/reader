#define API_NAME(CLASS,LIST) static ctext API_NAME="/" #CLASS; LIST
#define CMD(_X_) z_status __##_X_(const _X_& data) { return mq_send_msg_t(API_NAME,#_X_,data); }

API;

#undef CMD
#undef API_NAME

#define API_NAME(CLASS,LIST) api##CLASS##_t api##CLASS= { LIST }
#define CMD(_X_) ._X_ = __##_X_,


API;


#undef API
#undef CMD
#undef API_NAME