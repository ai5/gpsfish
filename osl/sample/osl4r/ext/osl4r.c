#include "ruby.h"
#include "osl/c/facade.h"
#include "osl/eval/progressEval.h"
#include "osl/eval/ml/openMidEndingEval.h"
#include "osl/rating/featureSet.h"

/* --- variables --- */

/** module OSL */
VALUE rb_cOSL4R; 


/* ------------ */

/**
 *  call-seq:
 *     OSL::setup => void
 *  
 *  Set up the OSL library. This should be called once in a program.
 */
extern "C" VALUE
osl4r_setup(VALUE self)
{
  osl::eval::ml::OpenMidEndingEval::setUp();
  osl::progress::ml::NewProgress::setUp();
  osl::rating::StandardFeatureSet::instance();

  return self;
}

/**
 *  call-seq:
 *     OSL::checkmate_attack(state = :str, limit = :int)  => [win = :bool, limit = :int, move = :str]
 *  
 *  Check if an attacking player can checkmate an opponent.
 *  Returns [true, limit, best_move] if an attacking player is in winning; [false, nil, nil] otherwise.
 */
extern "C" VALUE 
osl4r_checkmate_attack(VALUE self, 
                       VALUE v_state, VALUE v_limit)
{
  const char *state = StringValuePtr(v_state);
  int limit = NUM2INT(v_limit);

  char move[32]; // output
  memset(&move, 0, sizeof(move));
  const int win = checkmate_attack(state, limit, move);

  if (win) {
    VALUE best_move = rb_str_new2(move);
    return rb_ary_new3(3, Qtrue, INT2NUM(limit), best_move);
  } else {
    return rb_ary_new3(3, Qfalse, Qnil, Qnil);
  }
}

/**
 *  call-seq:
 *     OSL::checkmate_escape(state = :str, limit = :int)  => bool
 *  
 *  Check if an escaping player is checkmated. 
 *  Returns true if checkmated; false otherwise.
 */
extern "C" VALUE 
osl4r_checkmate_escape(VALUE self, 
                       VALUE v_state, VALUE v_limit)
{
  const char *state = StringValuePtr(v_state);
  const int limit = NUM2INT(v_limit);

  const int escape = checkmate_escape(state, limit);

  return (escape ? Qtrue : Qfalse);
}

/**
 *  call-seq:
 *     OSL::search(state = :str, seconds = :int, verbose = :bool)  => [eval = :int, move = :str]
 *  
 *  Search a state within seconds, then return an array of an evaluation
 *  value and best move.
 */
extern "C" VALUE 
osl4r_search(VALUE self, 
             VALUE v_state, VALUE v_seconds, VALUE v_verbose)
{
  const char *state = StringValuePtr(v_state);
  const int seconds = NUM2INT(v_seconds);
  const bool verbose = (Qtrue == v_verbose);

  char move[32]; // output
  memset(&move, 0, sizeof(move));
  const int eval = search(state, seconds, verbose, move);
  VALUE best_move = rb_str_new2(move);

  return rb_ary_new3(2, INT2NUM(eval), best_move);
}


extern "C" void
Init_osl4r (void) {
    /* ------ module OSL ------ */
    rb_cOSL4R      = rb_define_class("OSL", rb_cObject);
    rb_define_module_function(rb_cOSL4R, "setup", 
                              reinterpret_cast<VALUE (*)(...)>(osl4r_setup),   0);
    rb_define_module_function(rb_cOSL4R, "search", 
                              reinterpret_cast<VALUE (*)(...)>(osl4r_search), 3);
    rb_define_module_function(rb_cOSL4R, "checkmate_is_winning", 
                              reinterpret_cast<VALUE (*)(...)>(osl4r_checkmate_attack), 2);
    rb_define_module_function(rb_cOSL4R, "checkmate_is_losing", 
                              reinterpret_cast<VALUE (*)(...)>(osl4r_checkmate_escape), 2);
#if 0
    /* utilities */
    rb_define_module_function(rb_cUUID4R, "uuid_v1", uuid4r_uuid_v1, -1);
    rb_define_module_function(rb_cUUID4R, "uuid_v3", uuid4r_uuid_v3, -1);
    rb_define_module_function(rb_cUUID4R, "uuid_v4", uuid4r_uuid_v4, -1);
    rb_define_module_function(rb_cUUID4R, "uuid_v5", uuid4r_uuid_v5, -1);
    rb_define_module_function(rb_cUUID4R, "import",  uuid4r_import,   2);
    
    /* ------ UUID4RCommon ------ */
    rb_cUUID4RCommon = rb_define_class_under(rb_cUUID4R, "UUID4RCommon", rb_cObject);
    rb_define_alloc_func(rb_cUUID4RCommon, uuid4r_alloc);
    rb_define_method(rb_cUUID4RCommon, "export",  uuid4r_export, -1);
    rb_define_method(rb_cUUID4RCommon, "compare", uuid4r_compare, 1);
    rb_define_alias(rb_cUUID4RCommon,  "<=>", "compare");
    
    /* ------ UUID4Rv1 ------ */
    rb_cUUID4Rv1 = rb_define_class_under(rb_cUUID4R, "UUID4Rv1", rb_cUUID4RCommon);
    rb_define_alloc_func(rb_cUUID4Rv1, uuid4r_alloc);
    rb_define_method(rb_cUUID4Rv1, "initialize", uuid4rv1_initialize, 0);
    /* ------ UUID4Rv3 ------ */
    rb_cUUID4Rv3 = rb_define_class_under(rb_cUUID4R, "UUID4Rv3", rb_cUUID4RCommon);
    rb_define_alloc_func(rb_cUUID4Rv3, uuid4r_alloc);
    rb_define_method(rb_cUUID4Rv3, "initialize", uuid4rv3_initialize, 2);
    /* ------ UUID4Rv4 ------ */
    rb_cUUID4Rv4 = rb_define_class_under(rb_cUUID4R, "UUID4Rv4", rb_cUUID4RCommon);
    rb_define_alloc_func(rb_cUUID4Rv4, uuid4r_alloc);
    rb_define_method(rb_cUUID4Rv4, "initialize", uuid4rv4_initialize, 0);
    /* ------ UUID4Rv5 ------ */
    rb_cUUID4Rv5 = rb_define_class_under(rb_cUUID4R, "UUID4Rv5", rb_cUUID4RCommon);
    rb_define_alloc_func(rb_cUUID4Rv5, uuid4r_alloc);
    rb_define_method(rb_cUUID4Rv5, "initialize", uuid4rv5_initialize, 2);
#endif
}

