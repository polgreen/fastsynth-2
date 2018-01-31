#include <util/decision_procedure.h>
#include <solvers/prop/prop.h>

class solvert
{
public:
  decision_proceduret &get()
  {
    return *decision_procedure;
  }

  solvert(
    bool use_smt,
    const std::string &logic,
    const namespacet &,
    message_handlert &);

protected:
  std::unique_ptr<propt> prop;
  std::unique_ptr<decision_proceduret> decision_procedure;
};
