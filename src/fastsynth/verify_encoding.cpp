#include "verify_encoding.h"

#include <util/arith_tools.h>

//#include <langapi/language_util.h>

exprt verify_encodingt::operator()(const exprt &expr) const
{
  if(expr.id()==ID_function_application)
  {
    const auto &e=to_function_application_expr(expr);

    auto f_it=functions.find(e.function());
    
    exprt result=f_it==functions.end()?
      from_integer(0, e.type()):f_it->second;

    // need to instantiate parameters with arguments
    exprt instance=instantiate(result, e);

    return instance;
  }
  else
  {
    exprt tmp=expr;

    for(auto &op : tmp.operands())
      op=(*this)(op);

    return tmp;
  }
}

exprt verify_encodingt::instantiate(
  const exprt &expr,
  const function_application_exprt &e) const
{
  if(expr.id()==ID_symbol)
  {
    irep_idt identifier=to_symbol_expr(expr).get_identifier();
    static const std::string parameter_prefix="synth::parameter";

    if(std::string(id2string(identifier), 0, parameter_prefix.size())==parameter_prefix)
    {
      std::string suffix(id2string(identifier), parameter_prefix.size(), std::string::npos);
      std::size_t count=std::stoul(suffix);
      assert(count<e.arguments().size());
      return e.arguments()[count];
    }
    else
      return expr;
  }
  else
  {
    exprt tmp=expr;

    for(auto &op : tmp.operands())
      op=instantiate(op, e);

    return tmp;
  }
}

counterexamplet verify_encodingt::get_counterexample(
  const decision_proceduret &solver) const
{
  counterexamplet result;

  // iterate over nondeterministic symbols, and get their value
  for(const auto &var : free_variables)
  {
    exprt value=solver.get(var);
    result.assignment[var]=value;
  }

  return result;
}

