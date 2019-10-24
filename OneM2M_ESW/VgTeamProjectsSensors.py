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

def _create_desc_and_type(uri, desc, type, unit):
    create_cnt(uri, "description")
    create_data_cin(uri + "/description", desc)

    create_cnt(uri, "type")
    create_data_cin(uri, type)
    
    create_cnt(uri, "unit")
    create_data_cin(uri, unit)
    return

def create_io_device(server, cse, prj, node, cnt, desc, type):
    """
        Method description:
        creates an i/o device application entity(AE) from the OneM2M framework/tree
        under the specified node

        Parameters:
        server : [str] URI of parent CSE
        cse : [str] CSE name
        prj : [str] project name
        node: [str] node name
        cnt : [str] device container name
        desc: [str] device description
        type: [str] type = INPUT if the device is a sensor and 
                           OUTPUT if the io device is  an actuator
    """
    #create io device container
    create_cnt(server + "/" + cse + "/" + prj + "/" + node, cnt)
    
    #create and populate io_device description
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, desc, type, "")
    
    return

#1. Sensor - Outdoor Env
def create_oe(server, cse, prj, node, cnt, prefix):
    #measure - temperature
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "temperature")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "temperature", "Temperature", "Number", "")
    
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "rh")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "rh", "Relative Humidity", "Number", "")
    return

#2. Sensor Fuel Guage
def create_fg(server, cse, prj, node, cnt, prefix):
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "fuel_type")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "fuel_type", "Fuel type (petrol/diesel", "String", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "fuel_capacity")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "fuel_capacity", "Fuel capacity", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "fuel_current_level")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "fuel_current_level", "Fuel current level", "Number", "")
    return

#3. Sensor - Flow Meter
def create_fm(server, cse, prj, node, cnt, prefix):
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "pump_type")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "pump_type", "pump_type", "String", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "pump_capacity")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "pump_capacity", "pump_capacity", "Number", "ltr")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "pump_flowrate")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "pump_flowrate", "Pump flowrate (cubic meter per second)", "Number", "cumecs")
    return

#4. Acutuator - Supply Switch
def create_ss(server, cse, prj, node, cnt, prefix):
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "control_signal")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "control_signal", "control_signal", "Boolean", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "current_status")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "current_status", "current_status", "Boolean", "")
    return

#5. Sensor - Energy Meter
def create_em(server, cse, prj, node, cnt, prefix):
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "watts_total")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "watts_total", "Total power", "Number", "kW")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "watts_r_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "watts_r_phase", "Power R phase", "Number", "kW")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "watts_y_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "watts_y_phase", "Power Y phase", "Number", "kW")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "watts_b_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "watts_b_phase", "Power B phase", "Number", "kW")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "var_total")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "var_total", "var_total", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "var_r_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "var_r_phase", "var_r_phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "var_y_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "var_y_phase", "var_y_phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "var_b_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "var_b_phase", "var_b_phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "pf_avg")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "pf_avg", "Avg PF", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "pf_r_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "pf_r_phase", "PF R phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "pf_y_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "pf_y_phase", "PF Y phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "pf_b_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "pf_b_phase", "PF B phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "va_total")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "va_total", "va_total", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "va_r_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "va_r_phase", "va_r_phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "va_y_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "va_y_phase", "va_y_phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "va_b_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "va_b_phase", "va_b_phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "vll_avg")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "vll_avg", "Voltage line to line", "Number", "V")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "v_ry_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "v_ry_phase", "Voltage ry phase", "Number", "V")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "v_yb_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "v_yb_phase", "Voltage yb phase", "Number", "V")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "v_br_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "v_br_phase", "Voltage br phase", "Number", "V")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "vln_avg")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "vln_avg", "Agerage voltage line to netural", "Number", "V")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "v_r_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "v_r_phase", "Volatge R phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "v_y_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "v_y_phase", "Volatge Y phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "v_b_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "v_b_phase", "Volatge B phase", "Number", "")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "current_total")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "current_total", "Total current", "Number", "A")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "current_r_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "current_r_phase", "Current R phase", "Number", "A")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "current_y_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "current_y_phase", "Current Y phase", "Number", "A")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "current_b_phase")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "current_b_phase", "Current B phase", "Number", "A")
    create_cnt(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt, prefix + "frequency")
    _create_desc_and_type(server + "/" + cse + "/" + prj + "/" + node + "/" + cnt + "/" + prefix + "frequency", "Frequency", "Number", "")
    return
