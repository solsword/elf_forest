#ifndef SPECIES_H
#define SPECIES_H
// Mineral-based species types.

/*********
 * Enums *
 *********/

// Categories for the primary composition of minerals:
enum mineral_primary_e {
  MC_PR_SILLICATE,
  MC_PR_CARBONATE,
  MC_PR_OXIDE,
  MC_PR_HALIDE,
  MC_PR_SULFIDE,
  MC_PR_SULFATE,
  MC_PR_PHOSPHATE,
  MC_PR_RARE_METAL
};
typedef enum mineral_primary_e mineral_primary;

// Categories for mineral secondary composition
enum mineral_secondary_e {
  // Common elements:
  MC_SC_IRON,
  MC_SC_MAGNESIUM,
  MC_SC_ALUMINUM,
  MC_SC_SODIUM,
  MC_SC_CALCIUM,
  MC_SC_POTASSIUM,

  // Halide constituents:
  MC_SC_CHLORINE,
  MC_SC_BROMINE,
  MC_SC_FLUORINE, // flux for iron smelting; sometimes fluorescent

  // Less common metals:
  MC_SC_LEAD,
  MC_SC_TIN,
  MC_SC_ZINC,
  MC_SC_NICKEL,
  MC_SC_COPPER,
  MC_SC_SILVER,

  // Less common elements known in ancient times:
  MC_SC_MERCURY, // alchemy
  MC_SC_COBALT, // blue dye
  MC_SC_CHROMIUM, // red and yellow pigments; ore coloration
  MC_SC_MANGANESE, // dark-colored ores; used in glass dyeing; ancient pigments

  // Other less common elements
  MC_SC_MOLYBDENUM, // lubricating, like graphite
  MC_SC_TITANIUM, // white paint (modern process)
  MC_SC_BARIUM, // heavy
  MC_SC_BERRYLIUM, // gems (emerald; aquamarine)
  MC_SC_PLATINUM // precious metal
};
typedef enum mineral_secondary_e mineral_secondary;

/**************
 * Structures *
 **************/

struct dirt_species_s;
typedef struct dirt_species_s dirt_species;
struct clay_species_s;
typedef struct clay_species_s clay_species;
struct stone_species_s;
typedef struct stone_species_s stone_species;
struct metal_species_s;
typedef struct metal_species_s metal_species;

/*************
 * Constants *
 *************/

#define MAX_STONE_TRACE_MINERALS 3

/*************************
 * Structure Definitions *
 *************************/

struct dirt_species_s {
  material material;
  // dirt_texture_params appearance;

  // These three primary properties should add up to 100. Along with
  // organic_content and acidity, they determine most of the other properties
  // of the soil. Note that the sand_percent should not exceed 75 and the
  // clay_percent should not exceed 50. Under these conditions, the "dirt"
  // would be better classified as either sand or clay, and the use of B_SAND
  // or B_CLAY along with a stone_species or clay_species is more appropriate.
  // B_DIRT and dirt_species thus represent both pure silts and all manner of
  // loams that fill the rest of a sand/clay/silt triangle composition graph.
  uint8_t sand_percent;
  uint8_t clay_percent;
  uint8_t silt_percent;

  // Organic content ranges from 0 to 100, indicating the percentage of
  // non-mineral soil content. The rest of the soil is composed of sand, clay,
  // and silt as indicated by their respective percentage parameters.
  //
  // Data for Europe:
  //
  // http://eusoils.jrc.ec.europa.eu/projects/Soil_Atlas/HTmL/images/
  //   80-128%2022_img_1.jpg
  //
  // A rough guide is as follows:
  //
  //    0 - 1      inorganic (found in deserts)
  //    1 - 2      low (typically less-fertile)
  //    3 - 6      medium-low (includes prime agricultural land)
  //    7 - 12     medium-high (includes prime agricultural land)
  //   13 - 20     high (conditions must be favorable for deposition)
  //   21 - 40     very high (decomposition rates are probably low)
  //     41+       extreme (high deposition and/or low decomposition)
  uint8_t organic_content;

  // pH ranges from 0 to 14, but for most soils falls within 4.6 to 8.3 (the
  // range in which most plants can grow). Acidic soils are often the product
  // of ecological activity or simply accumulated organic matter, whereas basic
  // soils are usually found in regions were clay is present. Note that the
  // base_ph value represents a soil species' intrinsic pH value, but this can
  // be adjusted according to the block data information (see gen/soil.h).
  // Some ranges:
  //
  //     <4.6       extremely acid (most plants cannot grow)
  //   4.6 - 5.5    highly acidic (only specially-adapted or tolerant plants)
  //   5.5 - 6.0    acidic (favors acid-loving plants)
  //   6.0 - 6.8    slightly acidic (most plants can tolerate this range)
  //   6.8 - 7.2    "near-neutral;" few specific effects
  //   7.2 - 7.5    slightly basic (most plants can tolerate this range)
  //   7.5 - 8.3    basic (favors basic-loving plants)
  //     >8.3       extremely basic; most plants cannot grow
  //
  //  In general, most plants do well in the 6-7 range, while some acid-loving
  //  plants like blueberries, azaleas, and conifers tolerate and even thrive
  //  at pH 5-6. Some plants directly affect soil acidity (TODO: model this?).
  //  Above 7.5 pH, only a few plants can thrive.
  float base_ph;

  // These are the primary nutrient values for the soil. Levels are expressed
  // on a scale of 1-255, where 32 is a "standard level" which neither enhances
  // nor impedes growth.
  uint8_t base_phosphorus;
  uint8_t base_nitrogen;
  uint8_t base_potassium;

  // These are secondary nutrient levels, which again use a scale of 0-255 with
  // 32 as the "standard level."
  uint8_t base_sulfur;
  uint8_t base_magnesium;
  uint8_t base_calcium;
};

struct clay_species_s {
  material material;
  // clay_texture_params appearance;
};

struct stone_species_s {
  // Reference: http://www.physicalgeography.net/fundamentals/10d.html
  material material;
  stone_filter_args appearance;

  // Fundamental composition:
  // Note: elemental minerals use B_NATIVE_METAL and metal_species
  mineral_primary primary_composition;
  mineral_secondary secondary_composition;
  mineral_secondary tertiary_composition;
  mineral_secondary trace_composition[MAX_STONE_TRACE_MINERALS];
};

struct metal_species_s {
  material material;
  // metal_texture_params appearance;
};
