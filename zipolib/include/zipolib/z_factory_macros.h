/*________________________________________________________________________

z_factory_macros_h

________________________________________________________________________*/


#ifndef z_factory_macros_h
#define z_factory_macros_h


#define ZPROP_X(_VAR_ , _ID_ , _FLAGS_, _DESC_ )	\
	f->zf_prop_add((( (BASECLASS*)0)->_VAR_),_ID_,zp_offsetof_class(BASECLASS,_VAR_),_FLAGS_, _DESC_)


#define ZSTAT_X(_FUNC_,_NAME_,_FLAGS_,_DESC_) {f->add_feature(_NAME_,zf_stat_add<BASECLASS>(&BASECLASS::_FUNC_,_NAME_,_FLAGS_,_DESC_));}
#define ZSTAT(_FUNC_) ZSTAT_X(_FUNC_,#_FUNC_,ZFF_STAT_DEF,"")

#define ZMAP_X(_VAR_ , _ID_ , _FLAGS_, _DESC_ )	\
	f->add_feature(_ID_,f->zf_map_create((( (BASECLASS*)0)->_VAR_),_ID_,zp_offsetof_class(BASECLASS,_VAR_),_FLAGS_))

#define ZMAP_X(_VAR_ , _ID_ , _FLAGS_, _DESC_ )	\
	f->add_feature(_ID_,f->zf_map_create((( (BASECLASS*)0)->_VAR_),_ID_,zp_offsetof_class(BASECLASS,_VAR_),_FLAGS_))

#define ZVECT_X(_VAR_ , _ID_ , _FLAGS_, _DESC_ )	\
	f->add_feature(_ID_,f->zf_vect_create((( (BASECLASS*)0)->_VAR_),_ID_,zp_offsetof_class(BASECLASS,_VAR_),_FLAGS_));

#define ZMAP(_VAR_ )	ZMAP_X(_VAR_ , #_VAR_ , ZFF_PROP_DEF,  #_VAR_  )	
#define ZVECT(_VAR_ )	ZVECT_X(_VAR_ , #_VAR_ , ZFF_PROP_DEF,  #_VAR_  )	


//#define SDEF(sname, ...) S sname __VA_OPT__(= { __VA_ARGS__ })

#define ZPROP(_VAR_)	ZPROP_X(_VAR_ , #_VAR_ , ZFF_PROP_DEF,  #_VAR_  )	
#define ZPROP_F(_VAR_,_FLAGS_)	ZPROP_X(_VAR_ , #_VAR_ , _FLAGS_,  #_VAR_  )	

#define ZPARAM_X(_VAR_,_NAME_,_FLAGS_,_DESC_) ZPROP_X(_VAR_,_NAME_,_FLAGS_,_DESC_)


#define ZPARSE(   _STR_ )	\
	f->add_feature(ZFF_PARSE_STRING,z_new zf_const_string(_STR_,ZFF_PARSE_STRING,ZFF_PARSE_STRING,ZFF_CONST,0));


#define ZOBJ_X(_VAR_,_NAME_,_FLAGS_,_DESC_) \
	f->add_feature(_NAME_,f->zf_child_obj_create((( (BASECLASS*)0)->_VAR_),_NAME_,zp_offsetof_class(BASECLASS,_VAR_),_FLAGS_,_DESC_))

#define ZPOBJ(_VAR_,_NAME_,_FLAGS_,_DESC_) ZPROP_X(_VAR_,_NAME_,_FLAGS_,_DESC_)

#define ZOBJ(_VAR_)	ZOBJ_X(_VAR_ , #_VAR_ , ZFF_PROP_DEF,  #_VAR_  )	




#define ZACTS_X(_ACT_,_NAME_,_FLAGS_,_DESC_) {typedef z_status (BASECLASS::*fn_act)(z_stream& s); \
	fn_act _func_##_ACT_=&BASECLASS::_ACT_;f->add_mfunc_stream(#_ACT_,_NAME_,*(z_ptr_member_func_stream*)(&_func_##_ACT_) ,_FLAGS_,_DESC_);}
#define ZACTS(_ACT_) ZACTS_X(_ACT_,#_ACT_,ZFF_ACT_DEF,"")

#define ZACT_X(_ACT_,_NAME_,_FLAGS_,_DESC_) {typedef z_status (BASECLASS::*fn_act)(); \
	fn_act _func_##_ACT_=&BASECLASS::_ACT_;f->add_mfunc(#_ACT_,_NAME_,*(z_ptr_member_func*)(&_func_##_ACT_) ,_FLAGS_,_DESC_);}
#define ZACT(_ACT_) ZACT_X(_ACT_,#_ACT_,ZFF_ACT_DEF,"")

#define ZACT_JSON_X(_ACT_,_NAME_,_FLAGS_,_DESC_) {typedef z_status (BASECLASS::*fn_act)(z_json_stream& s,z_json_obj& obj); \
	fn_act _func_##_ACT_=&BASECLASS::_ACT_;f->add_mfunc_json(#_ACT_,_NAME_,*(z_ptr_member_func_json*)(&_func_##_ACT_) ,_FLAGS_,_DESC_);}
#define ZACT_JSON(_ACT_) ZACT_JSON_X(_ACT_,#_ACT_,ZFF_ACT_DEF,"")




#if 0
// This is noarg member function, but uses props as "params" when invoked through the controller
#define ZACT_XP(_ACT_,_NAME_,_FLAGS_,_DESC_,_N_,...) {typedef z_status (BASECLASS::*fn_act)(); \
	fn_act _func_##_ACT_=&BASECLASS::_ACT_;\
	f->add_act_params(#_ACT_,_NAME_,*(z_ptr_member_func*)(&_func_##_ACT_),_FLAGS_ ,_DESC_,_N_,__VA_ARGS__);}
#endif

#define ZPRM(_TYPE_,_NAME_,_DEFAULT_,_DESC_,_FLAGS_) std::ref(f->param_add<_TYPE_>(params,#_NAME_,_DEFAULT_,_DESC_,_FLAGS_))

#define ZCMD(_NAME_,_FLAGS_,_DESC_,...) { \
   auto mfunc=&BASECLASS::_NAME_;\
   zf_param_list *params =z_new zf_param_list(); \
   auto parambind=std::bind(mfunc,std::placeholders::_1, __VA_ARGS__ ); \
	zf_cmd* cmd=f->cmd_add<BASECLASS>(params,parambind,#_NAME_,_FLAGS_,_DESC_);\
	}

#define ZCMDS(_NAME_,_FLAGS_,_DESC_,...) { \
   auto mfunc=&BASECLASS::_NAME_;\
   zf_param_list *params =z_new zf_param_list(); \
   auto parambind=std::bind(mfunc,std::placeholders::_1,std::placeholders::_2, __VA_ARGS__ ); \
	zf_cmd* cmd=f->cmds_add<BASECLASS>(params,parambind,#_NAME_,_FLAGS_,_DESC_);\
	}



#define Z_FACT_ACT(_NAME_,_FLAGS_,_DESC_) { \
  auto mfunc=&BASECLASS::_NAME_;\
	zf_param_list *params = z_new zf_param_list(); \
   auto parambind = std::bind(mfunc, std::placeholders::_1, std::placeholders::_2,std::placeholders::_3);\
	zf_cmd* cmd=fact_cmd_add<BASECLASS>(params,parambind,#_NAME_,_FLAGS_,_DESC_);\
	}


#define ZBASE(_BASE_) z_factory_t<_BASE_>::static_add_members_class<BASECLASS>(f);
#define ZFACT_NEW(_CNAME_ ) template <>  _CNAME_* ::z_factory_t<_CNAME_>	::static_create_new_obj() { return z_new _CNAME_(); }; 
#define ZFACT_NEW_VIRT(_CNAME_ ) template <>  _CNAME_* ::z_factory_t<_CNAME_>	::static_create_new_obj() { return 0; };

#define ZFACT_INST(_CNAME_ ) template <> z_factory_t<_CNAME_> z_factory_t<_CNAME_>::_fact_obj(#_CNAME_);


#define ZMETA_DEF(_CNAME_) ZFACT_INST(_CNAME_);ZFACT_NEW(_CNAME_);
#define ZMETA_DEFV(_CNAME_) ZFACT_INST(_CNAME_);ZFACT_NEW_VIRT(_CNAME_);

#define ZMETA_DECL(_CNAME_) template <>  template<class BASECLASS> void z_factory_t<_CNAME_>::static_add_members_class(z_factory *f)



#define ZMETA(_CNAME_ ) \
ZMETA_DEF(_CNAME_)\
ZMETA_DECL(_CNAME_ )


#define ZCLS_VIRT(_CNAME_ ) \
ZMETA_DEFV(_CNAME_)\
ZMETA_DECL(_CNAME_ )





#endif

