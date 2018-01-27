#ifndef CPROVER_FASTSYNTH_LEARN_H_
#define CPROVER_FASTSYNTH_LEARN_H_

#include <util/decision_procedure.h>

#include "cegis_types.h"

/// Interface for classes which provide new candidate solutions for
/// counterexamples.
class learnt:public messaget
{
public:
  /// Virtual destructor for defined behaviour.
  virtual ~learnt() { }

  explicit learnt(message_handlert &_message_handler):
    messaget(_message_handler)
  {
  }

  /// Sets the maximum program size permitted.
  virtual void set_program_size(size_t program_size) = 0;

  /// Finds a new candidate for the current counterexample set.
  /// \return \see decision_proceduret::resultt
  virtual decision_proceduret::resultt operator()() = 0;

  /// Provides the last solution found.
  /// \return \see verify_encodingt::expressions
  virtual solutiont get_solution() const = 0;

  /// Adds an additional counterexample to the search constraint.
  /// \param counterexample New counterexample.
  virtual void add_ce(const counterexamplet &) = 0;
};

#endif /* CPROVER_FASTSYNTH_LEARN_H_ */
