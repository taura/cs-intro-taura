#
# compile
#
#mk_nb_flags := --dbg 2

nb_srcs :=
nb_srcs += $(wildcard nb_src/source/*/*.sos)

aux_srcs := 
aux_srcs += $(wildcard nb_src/source/cs/counter.h)

users_csv := ldap_users_taulec.csv 

feedback_class := csi
include ../../../jupyter_tools/authoring/jupyter_tools.mk
