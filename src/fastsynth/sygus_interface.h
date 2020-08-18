/*
 * cvc4_interface.h
 *
 *  Created on: 9 Dec 2019
 *      Author: elipol
 */

#ifndef SRC_FASTSYNTH_CVC4_INTERFACE_H_
#define SRC_FASTSYNTH_CVC4_INTERFACE_H_

#include "sygus_parser.h"
#include <sstream>
#include "cegis_types.h"
#include <solvers/decision_procedure.h>

std::string type2sygus(const typet &type);
std::string expr2sygus(const exprt &expr);
std::string expr2sygus(const exprt &expr, bool use_integers);
std::string clean_id(const irep_idt &id);

class sygus_interfacet
{
public:
    // output sygus file
    decision_proceduret::resultt doit(problemt &problem);
    decision_proceduret::resultt fudge();
    decision_proceduret::resultt doit(problemt &problem, bool use_ints, bool use_grammar, const int bound, const int timeout = 0);
    std::string declare_vars;
    std::string synth_fun;
    std::string constraints;
    std::string logic;
    std::string literal_string;
    bool use_grammar;

    decision_proceduret::resultt solve(const int timeout);
    void clear();
    std::map<irep_idt, exprt> result;
    solutiont solution;

protected:
    decision_proceduret::resultt read_result(std::istream &in);
    std::string build_grammar(const symbol_exprt &function_symbol, const int &bound, const std::string &literal);
    void build_query(problemt &problem, int bound);
    bool use_integers;
};

#endif /* SRC_FASTSYNTH_CVC4_INTERFACE_H_ */
