#include "sygus_frontend.h"
//#include "sygus_parser.h"
#include "cegis.h"
#include "literals.h"
#include "array_cegis.h"
#include "array_synth.h"
#include <iostream>

#include <util/cout_message.h>
#include <util/namespace.h>
#include <util/symbol_table.h>
#include <util/config.h>

#include <ansi-c/ansi_c_language.h>

#include <solvers/smt2/smt2_conv.h>

#include <langapi/language_util.h>
#include <langapi/mode.h>

#include <fstream>
#include <chrono>

#include "sygus_interface.h"

int sygus_frontend(const cmdlinet &cmdline)
{
  assert(cmdline.args.size() == 1);

  register_language(new_ansi_c_language);
  config.ansi_c.set_32();

  console_message_handlert message_handler;
  messaget message(message_handler);

  // this is our default verbosity
  unsigned int v = messaget::M_STATISTICS;

  if (cmdline.isset("verbosity"))
  {
    v = std::stol(
        cmdline.get_value("verbosity"));
    ;
    if (v > 10)
      v = 10;
  }

  message_handler.set_verbosity(v);

  std::ifstream in(cmdline.args.front());

  if (!in)
  {
    message.error() << "Failed to open input file" << messaget::eom;
    return 10;
  }

  sygus_parsert parser(in);

  try
  {
    parser.parse();
  }
  catch (const sygus_parsert::smt2_errort &e)
  {
    message.error() << e.get_line_no() << ": "
                    << e.what() << messaget::eom;
    return 20;
  }

  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  cegist cegis(ns);
  cegis.set_message_handler(message_handler);

  if (cmdline.isset("max-program-size"))
    cegis.max_program_size = std::stol(
        cmdline.get_value("max-program-size"));
  else
    cegis.max_program_size = 5; // default

  cegis.enable_bitwise = !cmdline.isset("no-bitwise");
  cegis.use_smt = cmdline.isset("smt");
  cegis.enable_division = cmdline.isset("enable-division");
  cegis.logic = parser.logic;

  problemt problem;
  // for (auto &c : parser.constraints)
  //   parser.expand_function_applications(c, false);
  problem.constraints = parser.constraints;

  problem.id_map = parser.id_map;
  for (const auto &id : parser.id_map)
  {
    if (id.second.kind == smt2_parsert::idt::VARIABLE &&
        id.second.type.id() != ID_mathematical_function &&
        id.second.definition.is_nil())
    {
      problem.free_variables.insert(symbol_exprt(id.first, id.second.type));
    }
  }

  for (const auto &f : parser.synth_fun_set)
    problem.synth_fun_set.insert(f);
  //return 0;

  for (auto &c : problem.constraints)
    parser.expand_function_applications(c);

  if (cmdline.isset("literals"))
    add_literals(problem);

#if 0
  sygus_interfacet sygus_interface;
  sygus_interface.print_problem(problem);
  return 1;
#endif

  auto start_time = std::chrono::steady_clock::now();
  /// ARRAY SYNTHESIS
  if (cmdline.isset("arrays"))
  {
    array_syntht array_synth(cegis.get_message_handler());
    switch (array_synth.array_synth_loop(parser, problem))
    {
    case decision_proceduret::resultt::D_SATISFIABLE:
      for (const auto &f : array_synth.solution.functions)
      {
        std::string stripped_id = id2string(f.first.get_identifier());
        //   std::string(id2string(f.first.get_identifier()), 11, std::string::npos);
        message.result() << messaget::green;

        message.result() << "Result: "
                         << stripped_id
                         << " -> "
                         << from_expr(ns, "", f.second)
                         << '\n';
        message.result() << messaget::reset << messaget::eom;

        smt2_convt smt(ns, "", "", "", smt2_convt::solvert::Z3, message.result());
        message.result() << "SMT: "
                         << id2string(f.first.get_identifier())
                         << " -> ";
        smt.handle(f.second);

        message.result() << '\n';
      }

      message.result() << messaget::eom;
      message.statistics() << messaget::magenta;

      message.statistics() << "Synthesis time: "
                           << std::chrono::duration<double>(
                                  std::chrono::steady_clock::now() - start_time)
                                  .count()
                           << 's'
                           << messaget::reset << messaget::eom;
      break;

    case decision_proceduret::resultt::D_UNSATISFIABLE:
      message.status() << "UNSATn array synth\n";
    case decision_proceduret::resultt::D_ERROR:
      message.status() << "ERROR array synth\n";
      return 1;
    }
  }
  else
  {
    switch (cegis(problem))
    {
    case decision_proceduret::resultt::D_SATISFIABLE:
      for (const auto &f : cegis.solution.functions)
      {
        std::string stripped_id = id2string(f.first.get_identifier());
        //   std::string(id2string(f.first.get_identifier()), 11, std::string::npos);

        message.result() << "Result: "
                         << stripped_id
                         << " -> "
                         << from_expr(ns, "", f.second)
                         << '\n';

        smt2_convt smt(ns, "", "", "", smt2_convt::solvert::Z3, message.result());
        message.result() << "SMT: "
                         << id2string(f.first.get_identifier())
                         << " -> ";
        smt.handle(f.second);

        message.result() << '\n';
      }

      message.result() << messaget::eom;

      message.statistics() << "Synthesis time: "
                           << std::chrono::duration<double>(
                                  std::chrono::steady_clock::now() - start_time)
                                  .count()
                           << 's'
                           << messaget::eom;
      message.statistics() << messaget::reset;
      message.statistics() << messaget::eom;

      break;

    case decision_proceduret::resultt::D_UNSATISFIABLE:
    case decision_proceduret::resultt::D_ERROR:
      return 1;
    }
  }
  return 0;
}
