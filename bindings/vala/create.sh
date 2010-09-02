#!/bin/sh
# Generates the Vala bindings file 'emerillon.vapi' from introspection data (emerillon.gi).
# emerillon.gi can be generated by 'vala-gen-introspect emerillon emerillon'
vapigen --library emerillon --pkg champlain-0.8 --pkg gtk+-2.0 --pkg clutter-1.0 emerillon/emerillon.gi --metadata emerillon/emerillon.metadata

