package require -exact qsys 12.1

# module properties
set_module_property NAME {unsaved_export}
set_module_property DISPLAY_NAME {unsaved_export_display}

# default module properties
set_module_property VERSION {1.0}
set_module_property GROUP {default group}
set_module_property DESCRIPTION {default description}
set_module_property AUTHOR {author}

# Instances and instance parameters
# (disabled instances are intentionally culled)
add_instance clk_0 clock_source 13.0
set_instance_parameter_value clk_0 clockFrequency {50000000.0}
set_instance_parameter_value clk_0 clockFrequencyKnown {1}
set_instance_parameter_value clk_0 resetSynchronousEdges {NONE}

# connections and connection parameters
# exported interfaces
add_interface clk clock sink
set_interface_property clk EXPORT_OF clk_0.clk_in
add_interface reset reset sink
set_interface_property reset EXPORT_OF clk_0.clk_in_reset
