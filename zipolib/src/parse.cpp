#include "pch.h"
#include "zipolib/parser.h"
#include "zipolib/z_variable.h"
#include "zipolib/z_filesystem.h"


/*_______________________________________________________________________________________________
	SINGLE TESTS
*/

z_status zp_test_char::test_run(zp_parser* p)
{
	z_status s = p->test_char(_c);
	PT("char %c = %d", _c, s);
	return s;
};
z_status zp_test_str::test_run(zp_parser* p)
{
	z_status s = p->test_string(_s);
	if (s == zs_matched)
	{
		PT("matched %s",_s.c_str());
	}
	return s;
};
zp_test& zp_test::assign_prop(zf_prop* prop)
{
	_prop = prop;
	return *this;

}

ctext zp_test::get_name()
{
	const std::type_info&  x = typeid(*this);
	return x.name();


}

z_status zp_test_wsp::test_run(zp_parser* p)
{
	z_status s = p->test_whitespace();
	if (_save && (s == zs_ok))
	{
		return Z_ERROR_NOT_IMPLEMENTED;
		/*
		zp_result_string* r = z_new zp_result_string(p);
		test_parent->add_child_result(this, ppResultParent, r);
		PT("IDENT=%s ", r->_match.c_str());
		*/
	}
	return s;
};
z_void_obj* zp_test::parse_v(ctext data, z_void_obj* vo)
{
	zp_parser parser;
	if(data)
		parser.set_source(data);
	z_status status;// = _parser.test(&test);
	status = parser.start(this, vo);

	if (status)
	{

		//parser.print_context();
		Z_ERROR(status);
		return 0;

	}
	//if success, at least create an empty object
	if (!vo)
		vo = new_object();
	return vo;

}
/*_______________________________________________________________________________________________
	GROUP TESTS
*/


/* WTF? */
zp_test& zp_test_group::ident(ctext prop_name )
{
	return  test<zp_test_ident>(prop_name);
}


zf_prop* zp_test_group::find_prop(ctext prop_name)
{
	zf_prop *prop = 0;
	z_factory* f = get_factory();
	if (f)
	{
		prop = f->get_feature_t<zf_prop>(prop_name);
	}
	return prop;
}




zp_test_group_or& operator | (zp_test& t1, zp_test& t2)
{
	zp_test_group_or* gor = z_new zp_test_group_or();
	*gor << t1 << t2;
	return *gor;
}

zp_test_group_or& operator | (zp_test& t1, char x)
{

	zp_test_group_or* gor = z_new zp_test_group_or();
	*gor << t1 << x;
	return *gor;
}
zp_test_group_or& operator | (char x, zp_test& t1)
{
	zp_test_group_or* gor = z_new zp_test_group_or();
	*gor << x << t1;
	return *gor;

}


zp_test_group_seq& operator << (zp_test& t1, zp_test& t2)
{
	zp_test_group_seq* g = z_new zp_test_group_seq();
	*g << t1 << t2;
	return *g;
}

zp_test_group_seq& operator <<  (char x, zp_test& t1)
{
	zp_test_group_seq* g = z_new zp_test_group_seq();
	*g << x << t1;
	return *g;
}
zp_test_group_seq& operator <<  (zp_test& t1, char x )
{
	zp_test_group_seq* g = z_new zp_test_group_seq();
	*g <<  t1<<x;
	return *g;
}

z_status zp_test_group::test_run(zp_parser* p)
{

	PT("%s start", get_name());

	if (p->eob())
		return zs_eof;
	p->level_down();

	Tests& tests = get_tests();

	bool repeat_group = false;
	z_status s = zs_ok;
	ctext data_start = p->get_index();
	bool commited = false;
	zp_test* last_stage = nullptr;
	bool created_object = false;

	if (get_opts().debug)
	{
		bool dummy = true;
	}

	for (auto stage : tests)
	{
		last_stage = stage;
		bool at_least_one = false;
		while (1) //multi for the child stage
		{
			if (get_opts().ignore_ws)
			{
				p->skip_ws();

			}
			s = stage->test_run(p);
			if (s == zs_ok)
			{
				if (stage->flags().commited)
				{
					commited = true;
					data_start = p->get_index();
				}
				at_least_one = true;
				if (stage->flags().multi)
					continue;
			}
			if (s == zs_no_match)
			{
				s = zs_no_match;
			}
			if ((s <= zs_eof) && (stage->flags().required == false))
			{
				// special case where last stage is optional
				s = zs_ok;
			}

			break;
		} //multi
		if (s >= zs_fatal_error)
			break;

		if (at_least_one)
		{
			s = zs_ok;
			if (get_opts().group_type==group_or)
				break; // OR group is done

			continue; // Stage is continue
		}
		if (get_opts().group_type == group_or)
			continue;
		if (stage->flags().required)
			break;

	}

	p->level_up();

	if (s == zs_ok)
	{
		test_pass(p, data_start, s);
	}
	else
	{
		//if (result_seq) 			delete result_seq;
		p->set_index(data_start);
		if((commited)&&(s <zs_fatal_error))
		{
				// TODO - REPORT EXPECTING
				s= Z_ERROR_MSG(zs_syntax_error, "Error while parsing %s ", get_name());
		}
		else
		{
			PT("%s NOMATCH", get_name());
		}
	}
	return s;
};

z_status zp_test_group::test_pass(zp_parser* p, ctext match_start,z_status status)
{

	zf_prop* prop = get_prop();

	if (prop)
	{
		z_void_obj* obj_parent = 0;
		obj_parent = p->get_current_obj(true);
		z_string s;
		p->get_match_from(match_start, s);
		PT("MATCH=%s", s.c_str());
		prop->set_from_string(s, obj_parent);

	}

	return status;


}


/*_______________________________________________________________________________________________
zp_test_obj
*/

z_status zp_test::test_pass(zp_parser* p)
{


	z_string s;
	p->get_match(s);
	zf_prop* prop = get_prop();
	if (prop)
	{
		z_void_obj* obj = p->get_current_obj(true);
		z_factory* fact=p->get_current_fact();
		if (obj)
		{
				PT("setting '%s(%x)' prop '%s' to '%s'", fact->get_name(), obj, prop->get_name(), s.c_str());
				prop->set_from_string(s, obj);

		}
		else
		{
			return Z_ERROR_MSG(zs_bad_parameter, "Should always be an object! (?)");
			PT("prop but no object");
		}


	}
	else
	{
		PT("MATCH = %s , but no prop",s.c_str());
	}


	return zs_ok;


}
