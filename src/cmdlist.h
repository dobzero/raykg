#pragma once
#include "droid/droid.h"

// pkg_ref_name: a reference name,tip: could be gtasa for com.rockstargames.gtasa
void droid_get_apk(droid_bundle_t * bundle, const char *pkg_ref_name);
void droid_display_useful_strings(const droid_bundle_t * bundle);
