SRC = fastsynth_main.cpp \
      smt2_frontend.cpp c_frontend.cpp sygus_frontend.cpp \
      cegis.cpp solver.cpp verify.cpp verify_encoding.cpp \
      literals.cpp solver_learn.cpp incremental_solver_learn.cpp \
      synth_encoding.cpp sygus_parser.cpp fourier_motzkin.cpp \
      symex_problem_factory.cpp  sygus_interface.cpp array_bound.cpp \
      array_synth.cpp bitvector2integer.cpp array_generalize.cpp 

OBJ+= ../../$(CPROVER_DIR)/src/ansi-c/ansi-c$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/assembler/assembler$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/linking/linking$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/big-int/big-int$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/goto-symex/goto-symex$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/goto-programs/goto-programs$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/pointer-analysis/pointer-analysis$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/analyses/analyses$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/langapi/langapi$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/solvers/solvers$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/analyses/dirty$(OBJEXT) \
      ../../$(CPROVER_DIR)/src/util/util$(LIBEXT)

INCLUDES= -I ../../$(CPROVER_DIR)/src -I ..

LIBS =

include ../config.inc
include ../../$(CPROVER_DIR)/src/config.inc
include ../../$(CPROVER_DIR)/src/common

CLEANFILES = fastsynth$(EXEEXT)

all: fastsynth$(EXEEXT)

###############################################################################

fastsynth$(EXEEXT): $(OBJ)
	$(LINKBIN)

