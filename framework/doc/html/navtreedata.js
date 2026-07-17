/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Morse Micro IoT SDK", "index.html", [
    [ "Introduction", "index.html", [
      [ "Morselib Overview", "index.html#autotoc_md46", [
        [ "API production readiness annotations", "index.html#autotoc_md47", null ]
      ] ]
    ] ],
    [ "Morse Micro IoT Reference Platforms", "MMPLATFORMS.html", "MMPLATFORMS" ],
    [ "Getting Started", "GETTING_STARTED.html", [
      [ "Prerequisites", "GETTING_STARTED.html#autotoc_md50", null ],
      [ "Unpacking the software package", "GETTING_STARTED.html#UNPACKING_THE_SW", null ],
      [ "Development PC setup", "GETTING_STARTED.html#GETTING_STARTED_DEV_PC_SETUP", [
        [ "Dependencies", "GETTING_STARTED.html#autotoc_md51", null ],
        [ "Python environment", "GETTING_STARTED.html#autotoc_md52", null ]
      ] ],
      [ "Configure Application", "GETTING_STARTED.html#autotoc_md53", [
        [ "Launching OpenOCD", "GETTING_STARTED.html#GETTING_STARTED_OPENOCD", null ],
        [ "Setting Config Store", "GETTING_STARTED.html#SET_APP_CONFIGURATION", null ]
      ] ],
      [ "Building Firmware", "GETTING_STARTED.html#BUILDING_FIRMWARE", null ],
      [ "Programming", "GETTING_STARTED.html#PROGRAMMING_FIRMWARE", [
        [ "Launching Miniterm", "GETTING_STARTED.html#autotoc_md54", null ],
        [ "Launching Application", "GETTING_STARTED.html#autotoc_md55", null ]
      ] ],
      [ "Troubleshooting", "GETTING_STARTED.html#autotoc_md56", [
        [ "Unable to find BCF file entry in config store", "GETTING_STARTED.html#BCF_CONFIG_STORE", [
          [ "Symptom", "GETTING_STARTED.html#autotoc_md57", null ],
          [ "Possible cause", "GETTING_STARTED.html#autotoc_md58", null ]
        ] ],
        [ "Unable to connect to the MCU with OpenOCD", "GETTING_STARTED.html#autotoc_md59", [
          [ "Symptom", "GETTING_STARTED.html#autotoc_md60", null ],
          [ "Possible cause 1", "GETTING_STARTED.html#autotoc_md61", null ],
          [ "Possible cause 2", "GETTING_STARTED.html#autotoc_md62", null ]
        ] ]
      ] ],
      [ "Additional steps for mm-ekh08-wb55 and mm-ekh18-wb55 platforms", "GETTING_STARTED.html#WB55_EXTRA_STEPS", null ]
    ] ],
    [ "SDK Overview", "OVERVIEW.html", [
      [ "Application Configuration", "OVERVIEW.html#APP_CONFIGURATION", [
        [ "Package Structure", "OVERVIEW.html#autotoc_md63", null ],
        [ "Example Application Structure", "OVERVIEW.html#autotoc_md64", null ],
        [ "Python environment", "OVERVIEW.html#autotoc_md65", null ],
        [ "Board Configuration File", "OVERVIEW.html#ABOUT_BCF", null ]
      ] ]
    ] ],
    [ "Debugging with GDB", "DEBUGGING.html", null ],
    [ "Host Power Save", "MMHOSTPOWERSAVE.html", [
      [ "Overview", "MMHOSTPOWERSAVE.html#autotoc_md68", null ],
      [ "Implementation", "MMHOSTPOWERSAVE.html#autotoc_md69", null ],
      [ "Deep sleep decision logic", "MMHOSTPOWERSAVE.html#autotoc_md70", null ],
      [ "Deep sleep exceptions", "MMHOSTPOWERSAVE.html#autotoc_md71", null ],
      [ "Debugging during deep sleep", "MMHOSTPOWERSAVE.html#MMHOSTPOWERSAVE_DEBUG_DEEP_SLEEP", null ]
    ] ],
    [ "LWIP on-demand timers", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_res_doc_on_demand_timers.html", [
      [ "How it's implemented", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_res_doc_on_demand_timers.html#autotoc_md73", null ],
      [ "When to enable or disable it", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_res_doc_on_demand_timers.html#autotoc_md74", null ],
      [ "Troubleshooting", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_res_doc_on_demand_timers.html#autotoc_md75", null ]
    ] ],
    [ "BCF regulatory channel support", "BCF_REGDOM_SUPPORT.html", [
      [ "Supported regulatory domains", "BCF_REGDOM_SUPPORT.html#autotoc_md76", null ],
      [ "AU", "BCF_REGDOM_SUPPORT.html#autotoc_md77", [
        [ "AU 802.11 2020", "BCF_REGDOM_SUPPORT.html#autotoc_md78", null ],
        [ "AU 802.11 2024", "BCF_REGDOM_SUPPORT.html#autotoc_md79", null ],
        [ "AU 802.11 revMF", "BCF_REGDOM_SUPPORT.html#autotoc_md80", null ]
      ] ],
      [ "CA", "BCF_REGDOM_SUPPORT.html#autotoc_md81", null ],
      [ "EU", "BCF_REGDOM_SUPPORT.html#autotoc_md82", null ],
      [ "GB", "BCF_REGDOM_SUPPORT.html#autotoc_md83", null ],
      [ "JP", "BCF_REGDOM_SUPPORT.html#autotoc_md84", null ],
      [ "US", "BCF_REGDOM_SUPPORT.html#autotoc_md85", null ]
    ] ],
    [ "Morse Micro CLI API", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html", [
      [ "Configuration variables", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md86", null ],
      [ "Module wlan: Wireless LAN management.", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md87", [
        [ "Configuration variables", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md88", null ],
        [ "Commands", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md89", [
          [ "wlan-connect", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md90", null ],
          [ "wlan-disconnect", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md91", null ],
          [ "wlan-scan", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md92", null ],
          [ "wlan-get_rssi", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md93", null ],
          [ "wlan-get_mac_addr", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md94", null ],
          [ "wlan-wnm_sleep", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md95", null ],
          [ "wlan-beacon_monitor_enable", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md96", null ],
          [ "wlan-beacon_monitor_disable", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md97", null ],
          [ "wlan-get_sta_status", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md98", null ],
          [ "wlan-dpp_push_button_start", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md99", null ],
          [ "wlan-dpp_stop", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md100", null ]
        ] ]
      ] ],
      [ "Module ip: IP Stack Management", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md101", [
        [ "Configuration variables", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md102", null ],
        [ "Commands", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md103", [
          [ "ip-status", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md104", null ],
          [ "ip-reload", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md105", null ]
        ] ]
      ] ],
      [ "Module ping: Ping application.", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md106", [
        [ "Configuration variables", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md107", null ],
        [ "Commands", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md108", [
          [ "ping-run", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md109", null ]
        ] ]
      ] ],
      [ "Module iperf: Iperf application.", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md110", [
        [ "Configuration variables", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md111", null ],
        [ "Commands", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md112", [
          [ "iperf-run", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md113", null ]
        ] ]
      ] ],
      [ "Module sys: System management.", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md114", [
        [ "Commands", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md115", [
          [ "sys-reset", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md116", null ],
          [ "sys-deep_sleep", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md117", null ],
          [ "sys-get_version", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md118", null ],
          [ "sys-get_stats", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md119", null ]
        ] ]
      ] ],
      [ "Enum definitions", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#autotoc_md120", [
        [ "Security type", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_security_type", null ],
        [ "Pmf mode", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_pmf_mode", null ],
        [ "Power save mode", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_power_save_mode", null ],
        [ "Mcs10 mode", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_mcs10_mode", null ],
        [ "Duty cycle mode", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_duty_cycle_mode", null ],
        [ "Station type", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_station_type", null ],
        [ "Status", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_status", null ],
        [ "Iperf mode", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_iperf_mode", null ],
        [ "Iperf state", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_iperf_state", null ],
        [ "Ip link state", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_ip_link_state", null ],
        [ "Deep sleep mode", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_deep_sleep_mode", null ],
        [ "Sta state", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_sta_state", null ],
        [ "Sta event", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_sta_event", null ],
        [ "Socket proto", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_socket_proto", null ],
        [ "Subsystem id", "md__home_jenkins_agent_workspace_build_mhs_binaries_mhs_os_mmagic_autogen_mmagic_cli.html#CLI_enum_subsystem_id", null ]
      ] ]
    ] ],
    [ "Example Applications", "MMAPPS.html", null ],
    [ "Deprecated List", "deprecated.html", null ],
    [ "Modules", "modules.html", "modules" ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", "functions_vars" ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"BCF_REGDOM_SUPPORT.html",
"dir_efbc7d0da7de3176d5b71fb32de097b3.html",
"group__MMAGIC__CONTROLLER__INTERNAL.html#gga4bd3a5ae7de626ca722537dd610886a8af0f9c7e1d892ab0652e1d157c8414a97",
"group__MMBUF.html#ga3b6217e681c8471b6bd04b64a6386fc5",
"group__MMIPAL.html#gga766bcb2fa9bef02bbc17203399c84f55a683a4ac6879fb3d4e0eafbc749ef7980",
"group__MMPKT__LIST.html#gadf08f6e58958a37ec41c1ca1d9a584bd",
"group__MMWLAN__STA.html#ga4f44eca015ebb4124bf45c4cd50192fb",
"mmwlan__stats_8h_source.html",
"structmmagic__core__mqtt__stop__agent__cmd__args.html#a25176677576451363b909efb7417e84f",
"structmmwlan__bcf__metadata.html#af0e72dd46bc0cebc74edf50d6a6b885f",
"structstruct__ping__status.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';