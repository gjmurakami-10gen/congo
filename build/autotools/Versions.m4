m4_define([congo_major_version], [0])
m4_define([congo_minor_version], [0])
m4_define([congo_micro_version], [1])
m4_define([congo_version], [congo_major_version.congo_minor_version.congo_micro_version])

# bump up by 1 for every micro release with no API changes, otherwise
# set to 0. after release, bump up by 1
m4_define([congo_interface_age], [0])
m4_define([congo_binary_age], [m4_eval(100 * congo_minor_version + congo_micro_version)])

m4_define([lt_current], [m4_eval(100 * congo_minor_version + congo_micro_version - congo_interface_age)])
m4_define([lt_revision], [congo_interface_age])
m4_define([lt_age], [m4_eval(congo_binary_age - congo_interface_age)])

m4_define([libbson_required_version], [0.6.4])
