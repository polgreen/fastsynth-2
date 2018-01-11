#include <util/std_types.h>
#include <util/config.h>

#include "synth_encoding.h"

#include <algorithm>

typet promotion(const typet &t0, const typet &t1)
{
  // same?
  if(t0==t1)
    return t0;

  // one is boolean?
  if(t0.id()==ID_bool)
    return t1;

  if(t1.id()==ID_bool)
    return t0;

  // same encoding but different width
  if(t0.id()==ID_signedbv && t1.id()==ID_signedbv)
  {
    auto t0_width=to_signedbv_type(t0).get_width();
    auto t1_width=to_signedbv_type(t1).get_width();
    return t0_width>=t1_width?t0:t1;
  }
  else if(t0.id()==ID_unsignedbv && t1.id()==ID_unsignedbv)
  {
    auto t0_width=to_unsignedbv_type(t0).get_width();
    auto t1_width=to_unsignedbv_type(t1).get_width();
    return t0_width>=t1_width?t0:t1;
  }
  else
    return t0;
}

exprt promotion(const exprt &expr, const typet &t)
{
  if(expr.type()==t)
    return expr;

  return typecast_exprt(expr, t);
}

typet e_datat::compute_word_type()
{
  typet result=return_type;

  for(const auto & t : parameter_types)
    result=promotion(result, t);

  return result;
}

void e_datat::setup(
  const function_application_exprt &e,
  const std::size_t program_size)
{
  if(setup_done) return;
  setup_done=true;

  function_symbol=e.function();
  const irep_idt &identifier=function_symbol.get_identifier();

  return_type=e.type();

  const auto &arguments=e.arguments();
  parameter_types.resize(arguments.size());
  for(std::size_t i=0; i<parameter_types.size(); i++)
    parameter_types[i]=arguments[i].type();

  word_type=compute_word_type();

  instructions.reserve(program_size);
  for(std::size_t pc=0; pc<program_size; pc++)
  {
    instructions.push_back(instructiont(pc));
    auto &instruction=instructions[pc];

    // constant -- hardwired default, not an option
    irep_idt const_val_id=id2string(identifier)+"_"+std::to_string(pc)+"_cval";
    instruction.constant_val=symbol_exprt(const_val_id, word_type);

    // one of the arguments
    for(std::size_t i=0; i<arguments.size(); i++)
    {
      irep_idt param_sel_id=id2string(identifier)+"_"+
               std::to_string(pc)+"_p"+std::to_string(i)+"sel";
      auto &option=instruction.add_option(param_sel_id);
      option.kind=instructiont::optiont::PARAMETER;
      option.parameter_number=i;
    }

    // a binary operation

    static const irep_idt ops[]=
      { ID_plus, ID_minus, ID_shl, ID_bitand, ID_bitor, ID_bitxor,
        ID_le, ID_lt, ID_equal, ID_notequal };

    std::size_t binary_op_index=0;

    for(const auto operation : ops)
      for(std::size_t operand0=0; operand0<pc; operand0++)
        for(std::size_t operand1=0; operand1<pc; operand1++)
        {
          // there is usually no point applying an operation to two
          // identical operands, with the exception of ID_plus, which
          // produces 2*x
          if(operand0==operand1 && operation!=ID_plus)
            continue;

          // many operators are commutative, no need
          // to have both orderings
          if(operation==ID_plus ||
             operation==ID_bitand ||
             operation==ID_bitor ||
             operation==ID_bitxor ||
             operation==ID_equal ||
             operation==ID_notequal)
          {
            if(operand0>operand1)
              continue;
          }

          irep_idt final_operation=operation;

          if(word_type.id()==ID_bool)
          {
            if(operation==ID_plus ||
               operation==ID_minus ||
               operation==ID_shl ||
               operation==ID_lt ||
               operation==ID_le ||
               operation==ID_notequal) // we got bitxor
             continue;

            if(operation==ID_bitand)
              final_operation=ID_and;
            else if(operation==ID_bitor)
              final_operation=ID_or;
            else if(operation==ID_bitxor)
              final_operation=ID_xor;
          }

          irep_idt sel_id=id2string(identifier)+"_"+
                   std::to_string(pc)+"_b"+
                   std::to_string(binary_op_index)+"sel";

          auto &option=instruction.add_option(sel_id);
          option.operand0=operand0;
          option.operand1=operand1;
          option.operation=final_operation;

          if(final_operation==ID_le ||
             final_operation==ID_lt ||
             final_operation==ID_equal ||
             final_operation==ID_notequal)
            option.kind=instructiont::optiont::BINARY_PREDICATE;
          else
            option.kind=instructiont::optiont::BINARY;

          binary_op_index++;
        }
  }
}

if_exprt e_datat::instructiont::chain(
  const symbol_exprt &selector,
  const exprt &expr_true,
  const exprt &expr_false)
{
  return if_exprt(
    selector,
    expr_true,
    expr_false);
}

exprt e_datat::instructiont::constraint(
  const typet &word_type,
  const std::vector<exprt> &arguments,
  const std::vector<exprt> &results)
{
  // constant, which is last resort
  exprt result_expr=constant_val;

  for(const auto &option : options)
  {
    switch(option.kind)
    {
    case optiont::PARAMETER:
      {
        exprt promoted_arg=
          promotion(arguments[option.parameter_number], word_type);
        result_expr=chain(option.sel, promoted_arg, result_expr);
      }
      break;

    case optiont::UNARY:
      // TBD
      break;

    case optiont::BINARY: // a binary operation
      {
        assert(option.operand0<results.size());
        assert(option.operand1<results.size());

        const auto &op0=results[option.operand0];
        const auto &op1=results[option.operand1];

        binary_exprt binary_expr(option.operation, word_type);
        binary_expr.op0()=op0;
        binary_expr.op1()=op1;

        result_expr=chain(option.sel, binary_expr, result_expr);
      }
      break;

    case optiont::BINARY_PREDICATE: // a predicate
      {
        assert(option.operand0<results.size());
        assert(option.operand1<results.size());

        const auto &op0=results[option.operand0];
        const auto &op1=results[option.operand1];

        binary_exprt binary_expr(option.operation, bool_typet());
        binary_expr.op0()=op0;
        binary_expr.op1()=op1;

        exprt promoted=promotion(binary_expr, word_type);

        result_expr=chain(option.sel, promoted, result_expr);
      }
      break;

    default:
      UNREACHABLE;
    }
  }

  return result_expr;
}

std::size_t e_datat::instance_number(const argumentst &arguments)
{
  const auto res=instances.insert(
    std::pair<argumentst, std::size_t>(arguments, instances.size()));

  return res.first->second;
}

exprt e_datat::result(const argumentst &arguments)
{
  // find out which instance this is
  std::size_t instance_number=this->instance_number(arguments);

  std::vector<exprt> results;
  results.resize(instructions.size(), nil_exprt());

  constraints.clear();

  const irep_idt &identifier=function_symbol.get_identifier();

  for(std::size_t pc=0; pc<instructions.size(); pc++)
  {
    exprt c=instructions[pc].constraint(word_type, arguments, results);

    // results vary by instance
    irep_idt result_identifier=
      id2string(identifier)+"_inst"+std::to_string(instance_number)+
      "_result_"+std::to_string(pc);

    results[pc]=symbol_exprt(result_identifier, c.type());

    constraints.push_back(equal_exprt(results[pc], c));
  }

  assert(!results.empty());

  return promotion(results.back(), return_type);
}

exprt e_datat::get_expression(
  const decision_proceduret &solver) const
{
  assert(!instructions.empty());

  std::vector<exprt> results;
  results.resize(instructions.size(), nil_exprt());

  for(std::size_t pc=0; pc<instructions.size(); pc++)
  {
    const auto &instruction=instructions[pc];
    exprt &result=results[pc];
    result=nil_exprt();

    // we now go _backwards_ through the options, as we've
    // built the ite inside-out

    for(instructiont::optionst::const_reverse_iterator
        o_it=instruction.options.rbegin();
        result.is_nil() && o_it!=instruction.options.rend();
        o_it++)
    {
      if(solver.get(o_it->sel).is_true())
      {
        switch(o_it->kind)
        {
        case instructiont::optiont::PARAMETER: // a parameter
          {
            irep_idt p_identifier="synth::parameter"+
                     std::to_string(o_it->parameter_number);
            result=promotion(
              symbol_exprt(p_identifier, parameter_types[o_it->parameter_number]),
              word_type);
          }
          break;

        case instructiont::optiont::UNARY:
          // TBD
          break;

        case instructiont::optiont::BINARY:
          {
            const auto &binary_op=*o_it;

            assert(binary_op.operand0<results.size());
            assert(binary_op.operand1<results.size());

            result=binary_exprt(
              results[binary_op.operand0],
              binary_op.operation,
              results[binary_op.operand1],
              word_type);
          }
          break;

        case instructiont::optiont::BINARY_PREDICATE:
          {
            const auto &binary_op=*o_it;

            assert(binary_op.operand0<results.size());
            assert(binary_op.operand1<results.size());

            result=binary_exprt(
              results[binary_op.operand0],
              binary_op.operation,
              results[binary_op.operand1],
              bool_typet());

            result=promotion(result, word_type);
          }
          break;

        default:
          UNREACHABLE;
        }
      }
    }

    // constant, this is the last resort when none of the
    // selectors is true
    if(result.is_nil())
      result=solver.get(instruction.constant_val);
  }

  return promotion(results.back(), return_type);
}

exprt synth_encodingt::operator()(const exprt &expr)
{
  if(expr.id()==ID_function_application)
  {
    auto tmp=to_function_application_expr(expr);

    // apply recursively to arguments
    for(auto &op : tmp.arguments())
      op=(*this)(op);

    e_datat &e_data=e_data_map[tmp.function()];
    exprt final_result=e_data(tmp, program_size);

    for(const auto &c : e_data.constraints)
      constraints.push_back(c);

    return final_result;
  }
  else if(expr.id()==ID_symbol)
  {
    // add the suffix
    symbol_exprt tmp=to_symbol_expr(expr);
    tmp.set_identifier(id2string(tmp.get_identifier())+suffix);
    return tmp;
  }
  else if(expr.id()==ID_nondet_symbol)
  {
    // add the suffix
    nondet_symbol_exprt tmp=to_nondet_symbol_expr(expr);
    irep_idt identifier=tmp.get_identifier();
    tmp.set_identifier(id2string(identifier)+suffix);
    return tmp;
  }
  else
  {
    exprt tmp=expr;

    for(auto &op : tmp.operands())
      op=(*this)(op); // recursive call

    return tmp;
  }
}

exprt synth_encodingt::get_expression(
  const symbol_exprt &function_symbol,
  const decision_proceduret &solver) const
{
  const auto it=e_data_map.find(function_symbol);
  if(it==e_data_map.end()) return nil_exprt();
  return it->second.get_expression(solver);
}

std::map<symbol_exprt, exprt> synth_encodingt::get_expressions(
  const decision_proceduret &solver) const
{
  std::map<symbol_exprt, exprt> result;

  for(const auto &it : e_data_map)
  {
    result[it.first]=it.second.get_expression(solver);
  }

  return result;
}
