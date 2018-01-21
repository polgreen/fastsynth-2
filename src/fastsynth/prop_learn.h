#ifndef CPROVER_FASTSYNTH_PROP_LEARN_H_
#define CPROVER_FASTSYNTH_PROP_LEARN_H_

#include <fastsynth/learn.h>
#include <fastsynth/cegis.h>
#include <fastsynth/cancellable_solver.h>

#include <solvers/sat/satcheck.h>

#include <memory>

/// Default learner implementation. Generates a constraint using synth_encodingt
/// and solves it using a configurable propt instance.
class prop_learnt : public learnt
{
  /// Namespace passed on to decision procedure.
  const namespacet &ns;

  /// Synthesis problem to solve.
  const cegist::problemt &problem;

  /// \see learnt::set_program_size(size_t)
  size_t program_size;

  /// Counterexample set to synthesise against.
  std::vector<verify_encodingt::counterexamplet> counterexamples;

  /// Solution created in the last invocation of prop_learnt::operator()().
  std::map<symbol_exprt, exprt> last_solution;

  /// Solver instance.
  std::weak_ptr<cancellable_solvert<satcheckt>> synth_satcheck;

public:
  /// Creates a non-incremental learner.
  /// \param msg \see msg prop_learnt::msg
  /// \param ns \see ns prop_learnt::ns
  /// \param problem \see prop_learnt::problem
  prop_learnt(
    messaget &msg,
    const namespacet &ns,
    const cegist::problemt &problem);

  /// \see learnt::set_program_size(size_t)
  void set_program_size(size_t program_size) override;

  /// \see learnt::operator()()
  decision_proceduret::resultt operator()() override;

  /// \see learnt::get_expressions()
  std::map<symbol_exprt, exprt> get_expressions() const override;

  /// \see learnt::add(const verify_encodingt::counterexamplet &counterexample)
  void add(const verify_encodingt::counterexamplet &counterexample) override;

  /// \see learnt::cancel()
  void cancel() override;
};

#endif /* CPROVER_FASTSYNTH_PROP_LEARN_H_ */
