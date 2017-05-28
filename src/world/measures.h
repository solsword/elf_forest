#ifndef MEASURES_H
#define MEASURES_H

// measures.h
// Typedefs for standard measures.

#include <stdint.h>

#include "elfscript/elfscript_gl.h"

#include "util.h"

/*********
 * Types *
 *********/

// Various material properties:

// For solid and liquid density, water (density ~1000 kg/m^3) is set at a
// value of 12, so the lightest representable material (value 1) has a
// density of ~83 kg/m^3 while the heaviest (value 255) weighs 21250 kg/m^3.
// For gas density, air (~1.2 kg/m^3) is set at a value of 12, so the
// lightest representable gas has a density of ~0.1 kg/m^3 while the heaviest
// has a density of ~25.5 kg/m^3. See http://en.wikipedia.org/wiki/Density
// for a table of densities for various substances.
typedef uint8_t density;

// As with density, specific heat is defined on a 0-255 scale, with the
// specific heat of air being set at 16.
typedef uint8_t specific_heat;

// Temperature is just in degrees Celsius
typedef int16_t temperature;

// Plasticity is on a 0-255 scale, where 0 is perfectly brittle, and 255 is
// perfectly plastic. It's a simplification that captures both malleability and
// ductility. Non-plastic substances like glass and ceramics are at 0, while
// most metals have at least 10 plasticity, even when extremely chilled.
// Highly malleable metals like copper have values around 120 near room
// temperature, while the scale beyond that is used to represent materials
// which are easily deformable even though they're not technically plastic,
// such as clay, as well as values for partially-melted materials.
typedef uint8_t plasticity;

// Viscosity is measured in centiPoise. A few reference points:
//   water @ 10C = 1.3
//   water @ 20C = 1.0
//   water @ 100C = 0.28
//
//      air @ 27C = 0.018
// hydrogen @ 27C = 0.009
//
//   blood @ 37C ~ 3-4
//
//          honey ~ 2,000-10,000
//   molten glass ~ 10,000-1,000,000
//        ketchup ~ 50,000-100,000
//           lard ~ 100,000
//     shortening ~ 250,000
//
//       acetone ~ 0.3
// sulfuric acid ~ 24
//     olive oil ~ 80
//    castor oil ~ 1000
//    corn syrup ~ 1400
//         pitch ~ 2.3e11
typedef float viscosity;

// Hardness is an arbitrary 0-255 scale, with wood around 60 and most stone in
// the 100-220 range.
typedef uint8_t hardness;

// pH values are on the standard scale from 0 to 14.
typedef float pH;

/*************
 * Constants *
 *************/
//
// Density of water (~1000 kg/m^3) on the 0-255 scale used for solid and liquid
// densities and of air (~1.2 kg/m^3) on the 0-255 scale used for gas
// densities.
static density const BASE_DENSITY = 12;
ELFSCRIPT_GL(i, BASE_DENSITY);

// Specific heat of air (~1 J/gK) on the 0-255 scale used for specific heat.
static specific_heat const BASE_SPECIFIC_HEAT;
ELFSCRIPT_GL(i, BASE_SPECIFIC_HEAT);

// Viscosity of water in centiPoise at ~20C:
static viscosity const NO_VISCOSITY = 0.0001;
static viscosity const GAS_VISCOSITY = 0.01;
static viscosity const WATER_VISCOSITY = 1.0;
static viscosity const SMOOTH_OIL_VISCOSITY = 100.0;
static viscosity const THICK_OIL_VISCOSITY = 1000.0;
static viscosity const HONEY_VISCOSITY = 5000.0;
static viscosity const LARD_VISCOSITY = 100000.0;
static viscosity const PITCH_VISCOSITY = 2e11;

ELFSCRIPT_GL(n, NO_VISCOSITY)
ELFSCRIPT_GL(n, GAS_VISCOSITY)
ELFSCRIPT_GL(n, WATER_VISCOSITY)
ELFSCRIPT_GL(n, SMOOTH_OIL_VISCOSITY)
ELFSCRIPT_GL(n, THICK_OIL_VISCOSITY)
ELFSCRIPT_GL(n, HONEY_VISCOSITY)
ELFSCRIPT_GL(n, LARD_VISCOSITY)
ELFSCRIPT_GL(n, PITCH_VISCOSITY)

// Hardness is an arbitrary scale, but the following constants pin it down:
static hardness const GENERIC_WOOD_HARDNESS = 60;
static hardness const GENERIC_STONE_HARDNESS = 180;

ELFSCRIPT_GL(i, GENERIC_WOOD_HARDNESS)
ELFSCRIPT_GL(i, GENERIC_STONE_HARDNESS)

// Plasticity is an arbitrary scale; these are some landmarks:
static plasticity const NO_PLASTICITY = 0; // glass, ceramic
static plasticity const LOW_PLASTICITY = 10; // chilled metals
static plasticity const MEDIUM_PLASTICITY = 120; // copper
static plasticity const HIGH_PLASTICITY = 200; // clay

ELFSCRIPT_GL(i, NO_PLASTICITY)
ELFSCRIPT_GL(i, LOW_PLASTICITY)
ELFSCRIPT_GL(i, MEDIUM_PLASTICITY)
ELFSCRIPT_GL(i, HIGH_PLASTICITY)

// pH constants are those you'd expect:
static pH const PH_PURE_ACID = 0.0;
static pH const PH_NEUTRAL = 7.0;
static pH const PH_PURE_BASE = 14.0;

ELFSCRIPT_GL(n, PH_PURE_ACID)
ELFSCRIPT_GL(n, PH_NEUTRAL)
ELFSCRIPT_GL(n, PH_PURE_BASE)

/**********
 * Limits *
 **********/

static density const MAT_MIN_DENSITY = 0;
static density const MAT_MAX_DENSITY = 255;

ELFSCRIPT_GL(i, MAT_MIN_DENSITY)
ELFSCRIPT_GL(i, MAT_MAX_DENSITY)

static specific_heat const MAT_MIN_SP_HEAT = 0;
static specific_heat const MAT_MAX_SP_HEAT = 255;

ELFSCRIPT_GL(i, MAT_MIN_SP_HEAT)
ELFSCRIPT_GL(i, MAT_MAX_SP_HEAT)

static temperature const MAT_MIN_TEMP = 0;
static temperature const MAT_MAX_TEMP = 65535;

ELFSCRIPT_GL(i, MAT_MIN_TEMP)
ELFSCRIPT_GL(i, MAT_MAX_TEMP)

static plasticity const MAT_MIN_PLASTICITY = 0;
static plasticity const MAT_MAX_PLASTICITY = 255;

ELFSCRIPT_GL(i, MAT_MIN_PLASTICITY)
ELFSCRIPT_GL(i, MAT_MAX_PLASTICITY)

#endif // #ifndef MEASURES_H
