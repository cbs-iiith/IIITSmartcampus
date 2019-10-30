# -*- coding: utf-8 -*- {{{
# vim: set fenc=utf-8 ft=python sw=4 ts=4 sts=4 et:
#
# Copyright (c) 2019, Sam Babu, Godithi.
# All rights reserved.
#
#
# IIIT Hyderabad

#}}}
from OneM2M import *
from VgTeamProjectsSensors import *

server = "http://192.168.1.4:8080"
cse = "~/in-cse/in-name"

#project 1 - Team32_Campus_energy_and_demand_dashboard
prj = "Team32_Campus_energy_and_demand_dashboard"
node = "pr_1_esp32_1"
create_io_device(server, cse, prj, node, "oe", "Outdoor Environment", "INPUT")
create_oe(server, cse, prj, node, "oe", "oe_1_")
create_io_device(server, cse, prj, node, "em", "Energy Meter", "INPUT")
create_em(server, cse, prj, node, "em", "em_1_")

#project 2 - Team40_DG_performance_monitoring
prj = "Team40_DG_performance_monitoring"
node = "pr_2_esp32_1"

create_io_device(server, cse, prj, node, "oe", "Outdoor Environment", "INPUT")
create_oe(server, cse, prj, node, "oe", "oe_1_")

create_io_device(server, cse, prj, node, "em", "Energy Meter", "INPUT")
create_em(server, cse, prj, node, "em", "em_1_")

create_io_device(server, cse, prj, node, "fg", "Fuel Guage", "INPUT")
create_fg(server, cse, prj, node, "fg", "fg_1_")

#project 3 - Team15_Hostel_washing_machine_automation
prj = "Team15_Hostel_washing_machine_automation"
node = "pr_3_esp32_1"
create_io_device(server, cse, prj, node, "oe", "Outdoor Environment", "INPUT")
create_oe(server, cse, prj, node, "oe", "oe_1_")
create_io_device(server, cse, prj, node, "em", "Energy Meter", "INPUT")
create_em(server, cse, prj, node, "em", "em_1_")
create_io_device(server, cse, prj, node, "fm", "Flow Meter", "INPUT")
create_fm(server, cse, prj, node, "fm", "fm_1_")
create_io_device(server, cse, prj, node, "ss", "Supply Switch", "OUTPUT")
create_ss(server, cse, prj, node, "ss", "ss_1_")

#project 4 - Team9_Pumps_performance_monitoring
prj = "Team9_Pumps_performance_monitoring"
node = "pr_4_esp32_1"
create_io_device(server, cse, prj, node, "oe", "Outdoor Environment", "INPUT")
create_oe(server, cse, prj, node, "oe", "oe_1_")
create_io_device(server, cse, prj, node, "em", "Energy Meter", "INPUT")
create_em(server, cse, prj, node, "em", "em_1_")
create_io_device(server, cse, prj, node, "fm", "Flow Meter", "INPUT")
create_fm(server, cse, prj, node, "fm", "fm_1_")

#project 5 - Team43_UPS_performance_monitoring
prj = "Team43_UPS_performance_monitoring"
node = "pr_5_esp32_1"
create_io_device(server, cse, prj, node, "oe", "Outdoor Environment", "INPUT")
create_oe(server, cse, prj, node, "oe", "oe_1_")
create_io_device(server, cse, prj, node, "em", "Energy Meter", "INPUT")
create_em(server, cse, prj, node, "em", "em_1_")
create_em(server, cse, prj, node, "em", "em_2_")
