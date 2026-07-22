#define API_NAME(CLASS,LIST) typedef z_status (MQ_HANDLER::*fnMqPtr)(const char* data); \
template <> MqServerMap<CLASS>::entry_t MqServerMap<CLASS>::map[]={  LIST }; \
template <> size_t MqServerMap<CLASS>::map_size=sizeof( MqServerMap<CLASS>::map)/sizeof(MqServerMap<CLASS>::entry_t);


#define CMD(_X_) { #_X_ ,(fnMqPtr) static_cast<z_status(MQ_HANDLER::*)(_X_*)>(&MQ_HANDLER::_X_), sizeof(_X_) },


API;


#undef API
#undef CMD
#undef API_NAME