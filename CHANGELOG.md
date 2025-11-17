
v0.0.4

Fix: Bug related to incremental build when a file is skipped. .o/.obj was not in the list to linked.
Feature: Add cb_msvc_parse_reset to offload msvc include files parsing, and make it easier to understand.

v0.0.3

Extension: Properly impelement cb_baked_binary_already_exists.
Feature: Add internal function cb_current_project_name.

v0.0.2

Fix: Bug where file with same name but different path would generate .o or .obj overwriting each other.
Feature: Add CB_VERSION and CB_VERSION_NUM version.
Feature: Add CHANGELOG.md.
Plugin: Created cbp_incremental_build.h.
Extension: Created cb_platform.h.

v0.0.1

Feature: Can build executable.
Feature: Can build static library.
Feature: Can build shared library.
Feature: msvc is supported.
Feature: gcc is supported.