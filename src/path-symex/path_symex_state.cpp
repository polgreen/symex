/*******************************************************************\

Module: State of path-based symbolic simulator

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// State of path-based symbolic simulator

#include "path_symex_state.h"

#include <util/simplify_expr.h>
#include <util/arith_tools.h>
#include <util/decision_procedure.h>

#include <util/c_types.h>

#include <pointer-analysis/dereference.h>

#include <goto-symex/adjust_float_expressions.h>

#ifdef DEBUG
#include <iostream>
#include <langapi/language_util.h>
#endif

path_symex_statet initial_state(
  var_mapt &var_map,
  const locst &locs,
  path_symex_historyt &path_symex_history)
{
  path_symex_statet s(var_map, locs, path_symex_history);

  // create one new thread
  path_symex_statet::threadt &thread=s.add_thread();
  thread.pc=locs.entry_loc; // set its PC
  s.set_current_thread(0);

  return s;
}

void path_symex_statet::output(const threadt &thread, std::ostream &out) const
{
  out << "  PC: " << thread.pc << '\n';
  out << "  Call stack:";
  for(call_stackt::const_iterator
      it=thread.call_stack.begin();
      it!=thread.call_stack.end();
      it++)
    out << " " << it->return_location << '\n';
  out << '\n';
}

void path_symex_statet::output(std::ostream &out) const
{
  for(unsigned t=0; t<threads.size(); t++)
  {
    out << "*** Thread " << t << '\n';
    output(threads[t], out);
    out << '\n';
  }
}

path_symex_statet::var_statet &path_symex_statet::get_var_state(
  const var_mapt::var_infot &var_info)
{
  assert(current_thread<threads.size());

  var_valt &var_val=
    var_info.is_shared()?shared_vars:threads[current_thread].local_vars;
  if(var_val.size()<=var_info.number)
    var_val.resize(var_info.number+1);
  return var_val[var_info.number];
}

void path_symex_statet::record_step()
{
  // is there a context switch happening?
  if(!history.is_nil() &&
     history->thread_nr!=current_thread)
    no_thread_interleavings++;

  // add the step
  history.generate_successor();
  stept &step=*history;

  // copy PC
  assert(current_thread<threads.size());
  step.pc=threads[current_thread].pc;
  step.thread_nr=current_thread;

  // set hide flag
  step.hidden=get_hide();
}

bool path_symex_statet::is_feasible(
  decision_proceduret &decision_procedure) const
{
  // feed path constraint to decision procedure
  decision_procedure << history;

  // check whether SAT
  switch(decision_procedure())
  {
  case decision_proceduret::resultt::D_SATISFIABLE: return true;

  case decision_proceduret::resultt::D_UNSATISFIABLE: return false;

  case decision_proceduret::resultt::D_ERROR:
    throw "error from decision procedure";
  }

  return true; // not really reachable
}

bool path_symex_statet::check_assertion(
  decision_proceduret &decision_procedure)
{
  const goto_programt::instructiont &instruction=*get_instruction();

  // assert that this is an assertion
  assert(instruction.is_assert());

  // the assertion in SSA
  exprt assertion=read(instruction.guard);

  // trivial?
  if(assertion.is_true())
    return true; // no error

  // the path constraint
  decision_procedure << history;

  // negate the assertion
  decision_procedure.set_to(assertion, false);

  // check whether SAT
  switch(decision_procedure.dec_solve())
  {
  case decision_proceduret::resultt::D_SATISFIABLE:
    return false; // error

  case decision_proceduret::resultt::D_UNSATISFIABLE:
    return true; // no error

  default:
    throw "error from decision procedure";
  }

  return true; // not really reachable
}
