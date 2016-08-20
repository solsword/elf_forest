<gen.geo>

~CONSTITUENT_AVERAGING_WEIGHTS = [1.5, 1.0, 0.7];
~SOURCE_DISTRIBUTION = #rngtable {
  values = [GEO_IGNEOUS, GEO_METAMORPHIC, GEO_SEDIMENTARY];
  weights = [0.35, 0.25, 0.4];
};
~IGNEOUS_COMPOSITION = #rngtable {
  values = [
    MNRL_COMP_STONE,
    MNRL_COMP_STONE_STONE,

    MNRL_COMP_STONE_LIFE,

    MNRL_COMP_STONE_METAL,
    MNRL_COMP_STONE_METAL_METAL,

    MNRL_COMP_STONE_RARE,
    MNRL_COMP_LIFE,
  ];
  weights = [
    0.40, 0.35, // 75% |  75%
    0.10,       // 10% |  85%
    0.05, 0.05, // 10% |  95%
    0.03, 0.02, //  5% | 100%
  ];
};
~METAMORPHIC_COMPOSITION = #rngtable {
  values = [
    MNRL_COMP_STONE_WATER,
    MNRL_COMP_STONE_STONE,
    MNRL_COMP_STONE_METAL,

    MNRL_COMP_STONE_STONE_STONE,
    MNRL_COMP_STONE_STONE_METAL,
    MNRL_COMP_STONE_METAL_METAL,

    MNRL_COMP_STONE_STONE_LIFE,
    MNRL_COMP_STONE_AIR,
    MNRL_COMP_STONE_STONE,
    MNRL_COMP_RARE_RARE,

    MNRL_COMP_STONE_LIFE,
    MNRL_COMP_STONE_METAL_RARE,
    MNRL_COMP_STONE_RARE,

    MNRL_COMP_STONE_STONE_RARE,
    MNRL_COMP_STONE_RARE_RARE,
  ];
  weights = [
    0.17, 0.12, 0.12,        // 41% |  41%
    0.09, 0.09, 0.07,        // 25% |  66%
    0.06, 0.06, 0.06, 0.05,  // 23% |  89%
    0.04, 0.03, 0.02,        //  9% |  98%
    0.01, 0.01,              //  2% | 100%
  ];
};
~SEDIMENTARY_COMPOSITION = #rngtable {
  values = [
    MNRL_COMP_STONE_LIFE,
    MNRL_COMP_STONE_WATER,
    MNRL_COMP_STONE_AIR,

    MNRL_COMP_LIFE,

    MNRL_COMP_STONE,
    MNRL_COMP_STONE_STONE,

    MNRL_COMP_RARE_RARE,
  ];
  weights = [
    0.26, 0.23, 0.21,    // 70% |  70%
    0.12,                // 12% |  82%
    0.08, 0.06,          // 14% |  96%
    0.04,                //  4% | 100%
  ];
};
~IGNEOUS_TRACES = #rngtable {
  values = [
    MNRL_TRACE_NONE,

    MNRL_TRACE_LIFE,
    MNRL_TRACE_METAL,
    MNRL_TRACE_METAL_METAL,

    MNRL_TRACE_RARE,
    MNRL_TRACE_METAL_RARE,

    MNRL_TRACE_RARE_RARE,
    MNRL_TRACE_STONE,
    MNRL_TRACE_STONE_METAL,
  ];
  weights = [
    0.5,               // 50% |  50%
    0.12, 0.11, 0.11,  // 34% |  84%
    0.06, 0.05,        // 11% |  95%
    0.02, 0.02, 0.01,  //  5% | 100%
  ];
};
~METAMORPHIC_TRACES = #rngtable {
  values = [
    MNRL_TRACE_NONE,

    MNRL_TRACE_RARE,
    MNRL_TRACE_METAL,

    MNRL_TRACE_WATER,
    MNRL_TRACE_AIR,

    MNRL_TRACE_METAL_RARE,
    MNRL_TRACE_RARE_RARE,
    MNRL_TRACE_METAL_METAL,

    MNRL_TRACE_LIFE,
    MNRL_TRACE_STONE_METAL,
    MNRL_TRACE_STONE
  ];
  weights = [
    0.20,              // 20% |  20%
    0.15, 0.13,        // 28% |  48%
    0.10, 0.09,        // 19% |  67%
    0.07, 0.07, 0.07,  // 21% |  88%
    0.05, 0.04, 0.03   // 12% | 100%
  ];
};
~SEDIMENTARY_TRACES = #rngtable {
  values = [
    MNRL_TRACE_NONE,

    MNRL_TRACE_STONE,

    MNRL_TRACE_LIFE,

    MNRL_TRACE_WATER,
    MNRL_TRACE_AIR,

    MNRL_TRACE_METAL,
    MNRL_TRACE_STONE_METAL,
    MNRL_TRACE_RARE
  ];
  weights = [
    0.5,               // 50% |  50%
    0.17,              // 17% |  67%
    0.12,              // 12% |  79%
    0.08, 0.06,        // 14% |  93%
    0.03, 0.02, 0.02   //  7% | 100%
  ];
};

@EXPAND_COMPOSITION = #func {~composition = MNRL_COMP_STONE} {
  !return @lookup(
    key = ~composition;
    dict = #dict {
      MNRL_COMP_STONE: [EL_CATEGORY_STONE];
      MNRL_COMP_LIFE: [EL_CATEGORY_LIFE];
      MNRL_COMP_STONE_AIR: [EL_CATEGORY_STONE, EL_CATEGORY_AIR];
      MNRL_COMP_STONE_WATER: [EL_CATEGORY_STONE, EL_CATEGORY_WATER];
      MNRL_COMP_STONE_LIFE: [EL_CATEGORY_STONE, EL_CATEGORY_LIFE];
      MNRL_COMP_STONE_STONE: [EL_CATEGORY_STONE, EL_CATEGORY_STONE];
      MNRL_COMP_STONE_STONE_LIFE: [
        EL_CATEGORY_STONE,
        EL_CATEGORY_STONE,
        EL_CATEGORY_LIFE
      ];
      MNRL_COMP_STONE_STONE_STONE: [
        EL_CATEGORY_STONE,
        EL_CATEGORY_STONE,
        EL_CATEGORY_STONE
      ];
      MNRL_COMP_STONE_METAL: [EL_CATEGORY_STONE, EL_CATEGORY_METAL];
      MNRL_COMP_STONE_STONE_METAL: [
        EL_CATEGORY_STONE,
        EL_CATEGORY_STONE,
        EL_CATEGORY_METAL
      ];
      MNRL_COMP_STONE_METAL_METAL: [
        EL_CATEGORY_STONE,
        EL_CATEGORY_METAL,
        EL_CATEGORY_METAL
      ];
      MNRL_COMP_STONE_METAL_RARE: [
        EL_CATEGORY_STONE,
        EL_CATEGORY_METAL,
        EL_CATEGORY_RARE
      ];
      MNRL_COMP_STONE_RARE: [EL_CATEGORY_STONE, EL_CATEGORY_RARE];
      MNRL_COMP_STONE_STONE_RARE: [
        EL_CATEGORY_STONE,
        EL_CATEGORY_STONE,
        EL_CATEGORY_RARE
      ];
      MNRL_COMP_STONE_RARE_RARE: [
        EL_CATEGORY_STONE,
        EL_CATEGORY_RARE,
        EL_CATEGORY_RARE
      ];
      MNRL_COMP_RARE_RARE: [EL_CATEGORY_RARE, EL_CATEGORY_RARE];
    };
  );
};

@EXPAND_TRACES = #func {~traces = MNRL_TRACE_NONE} {
  !return @lookup(
    key = ~traces;
    dict = #dict {
      MNRL_TRACE_NONE: [];
      MNRL_TRACE_AIR: [EL_CATEGORY_AIR];
      MNRL_TRACE_WATER: [EL_CATEGORY_WATER];
      MNRL_TRACE_LIFE: [EL_CATEGORY_LIFE];
      MNRL_TRACE_STONE: [EL_CATEGORY_STONE];
      MNRL_TRACE_METAL: [EL_CATEGORY_METAL];
      MNRL_TRACE_RARE: [EL_CATEGORY_RARE];
      MNRL_TRACE_STONE_METAL: [EL_CATEGORY_STONE, EL_CATEGORY_METAL];
      MNRL_TRACE_METAL_METAL: [EL_CATEGORY_METAL, EL_CATEGORY_METAL];
      MNRL_TRACE_METAL_RARE: [EL_CATEGORY_METAL, EL_CATEGORY_RARE];
      MNRL_TRACE_RARE_RARE: [EL_CATEGORY_RARE, EL_CATEGORY_RARE];
    };
  );
};

@COMPUTE_STONE_COLOR = #func {
  ~constituents = [];
  ~traces = [];
  ~source = MO_UNKNOWN;
  ~density = 0.0;
  ~plasticity = 0.0;
  ~hardness = 0.0;
  ~seed = 0;
} {
  ~hash = @brng(~seed, 7182731);
  ~el_brightness = @weight(
    weights = [0.3, 0.7],
    values = [
      100 * @avg(@randf(@brng(~hash, 1)), @randf(@brng(~hash, 2))),
      @weighted_el_prop(
        ~constituents,
        CONSTITUENT_AVERAGING_WEIGHTS,
        EL_PRP_F_STONE_TND_BRIGHTNESS
      )
    ]
  );
  ~chroma_per_constituent = @multimap(
    vars = {
      *constituent = ~constituents;
      *iter = &range(1,);
    };
    function = #func {*constituent = SP_INVALID, *iter = 0} {
      weight = @weight(
        weights = [0.5, 0.5],
        values = [
          @expdist(
            @randf(@brng(~hash, 12131 * *iter)),
            @if(~source == GEO_METAMORPHIC, then = 1.5, else = 4)
          ),
          @if(
            ~source == GEO_METAMORPHIC,
            then = @randf(@brng(~hash, 13151 * *iter)),
            else = @randf_pnorm(@brng(~hash, 13151 * iter), 0.4, 1)
          )
        ]
      )
      tnd = @get_el_attr(*constituent, EL_PRP_F_STONE_TND_CHROMA)
      !return {
        x = weight * @cos(tnd),
        y = weight * @sin(tnd),
      }
    };
  );
  ~chroma_x = @avg(~chroma_per_constituent.x);
  ~chroma_y = @avg(~chroma_per_constituent.y);
  // TODO: Better color-mixing algorithm?
  ~el_hue = @atan2(~chroma_x, ~chroma_y);
  ~el_intensity = (~chroma_x**2 + ~chroma_y**2)**0.5;
  ~el_saturation = @scale(
    value = @expdist(
      ~el_intensity,
      @if(~source == GEO_METAMORPHIC, then=2.0, else=3.5)
    ),
    from = 60,
    to = 90
  ) * @weight(
    weights = [0.3, 0.7],
    values = [
      ~el_intensity,
      @expdist(
        @randf(@brng(~hash, 3)),
        @if(~source == GEO_METAMORPHIC, then=2.7, else=5)
      )
    ]
  );
  !return @choose(
    index = @index_of(~source, [GEO_IGNEOUS, GEO_METAMORPHIC, GEO_SEDIMENTARY]);
    values = [
      igneous_color = #color {
        fmt = CMFT_LCH;
        L = (
          @weight(
            [0.6, 0.4],
            [
              ~el_brightness,
              100 * @weight([0.6, 0.4], [@scale(~density, 1, 0), ~plasticity])
            ]
          )
        )**0.8;
        c = (
          ~el_saturation
        * @if( // a bimodal distribution
            @randf(@brng(~hash, 4)) < 0.7,
            then=@randf_pnorm(@brng(~hash, 5), 0.05, 0.3),
            else=@randf_pnorm(@brng(~hash, 5), 0.6, 0.8),
          );
        );
        h = (
          ~el_hue
        + @randf_pnorm(@brng(~hash, 6), -M_PI/16.0, M_PI/16.0)
        );
      };
      metamorphic_color = #color {
        fmt = CMFT_LCH;
        L = ~el_brightness;
        ~boost = @randf_pnorm(@brng(~hash, 7), 0.0, 0.4);
        c = @weight(
          weights = [(1 - ~boost), ~boost],
          values = [
            ~el_saturation,
            @scale(@randf(@brng(~hash, 8)), 30, 100)
          ]
        );
        h = (
          ~el_hue
        + @randf_pnorm(@brng(~hash, 9), -M_PI/6.0, M_PI/6.0)
        );
      };
      sedimentary_color = #color {
        fmt = CMFT_LCH;
        L = (
          ~el_brightness
        * @randf_pnorm(@brng(~hash, 10), 1, 1.4)
        );
        c = (
          ~el_saturation
        * @expdist(@randf_pnorm(@brng(~hash, 11), 0.3, 1), 2)
        );
        h = (
          ~el_hue
        + @randf_pnorm(@brng(~hash, 12), -M_PI/12.0, M_PI/12.0)
        );
      };
    ];
  );
};

@gen_stone_species = @func {~world_map = NULL; ~seed = 0;} {
  ~hash = @brng(~seed, 467541);
  ~source = @sample_rngtable(~hash, ~SOURCE_DISTRIBUTION);
  ~hash = @prng(~hash);
  ~comp_table = @choose(
    index = @index_of(~source, [GEO_IGNEOUS, GEO_METAMORPHIC, GEO_SEDIMENTARY]),
    values = [~IGNEOUS_COMP, ~METAMORPHIC_COMP, ~SEDIMENTARY_COMP]
  );
  ~trace_table = @choose(
    index = @index_of(~source, [GEO_IGNEOUS, GEO_METAMORPHIC, GEO_SEDIMENTARY]),
    values = [~IGNEOUS_TRACES, ~METAMORPHIC_TRACES, ~SEDIMENTARY_TRACES]
  );
  !return #stone_species {
    source = ~source;
    composition = {
      ~comp = @sample_rngtable(~hash, ~comp_table);
      ~hash = @prng(~hash);
      ~traces = @sample_rngtable(~hash, ~trace_table);
      ~hash = @prng(~hash);
      ~comp_categories = @EXPAND_COMPOSITION(~comp);
      ~trace_categories = @EXPAND_TRACES(~traces);
      ~used = [];
      ~comp_elements = @map(
        ~comp_categories,
        #func {~cat = EL_CATEGORY_NONE} {
          ~result = @pick_element(~world_map, ~cat, ~used, ~hash);
          ~hash = @prng(~hash);
          @append(~used, ~result);
          !return ~result;
        };
      );
      ~trace_elements = @map(
        ~trace_categories,
        #func {~cat = EL_CATEGORY_NONE} {
          ~result = @pick_element(~world_map, ~cat, ~used, ~hash);
          ~hash = @prng(~hash);
          @append(~used, ~result);
          !return ~result;
        };
      );
      constituents = @filter(
        ~comp_elements,
        #func {~item = NULL} { !return @not(@=i(SP_INVALID, ~item)) }
      )
      traces = @filter(
        ~trace_elements,
        #func {~item = NULL} { !return @not(@=i(SP_INVALID, ~item)) }
      )
    };
    material = @choose(
      index = @index_of(~source, [GEO_IGNEOUS,GEO_METAMORPHIC,GEO_SEDIMENTARY]),
      values = [
        igneous_material = #material {
          // Base variables:
          base_density = @weight(
            [0.4, 0.6],
            [
              @randf(~hash),
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_DENSITY
              )
            ]
          );
          ~hash = @prng(~hash);
          base_sp_heat = @weight(
            [0.42, 0.18, 0.4],
            [
              @avg(
                @randf_pnorm(~hash, 0, 1),
                @randf_pnorm(@brng(~hash, 17), 0, 1)
              ),
              (1 - base_density),
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_SP_HEAT
              )
            ]
          );
          ~hash = @prng(~hash);
          base_tr_temp = @weight(
            [0.48, 0.12, 0.4],
            [
              @randf_pnorm(~hash, 0, 1),
              base_density,
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_TR_TEMP
              )
            ]
          );
          ~hash = @prng(~hash);
          base_plastic_temp = randf_pnorm(~hash, 0, 1)**1.2;
          ~hash = @prng(~hash);
          ~cnst_plasticity = @weighted_el_prop(
            composition.constituents,
            ~CONSTITUENT_AVERAGING_WEIGHTS,
            EL_PRP_F_STONE_TND_PLASTICITY
          );
          // More than 75% of values fall into [0, 0.33]:
          base_cold_plasticity = @weight(
            [0.3, 0.7],
            [ ~cnst_plasticity, @expdist(@randf_pnorm(~hash, 0, 1), 5) ]
          );
          ~hash = @prng(~hash);
          base_warm_plasticity = @weight(
            [0.4, 0.6],
            [ ~cnst_plasticity, @expdist(@randf_pnorm(~hash, 0, 1), 3) ]
          );
          ~hash = @prng(~hash);
          base_hardness = @weight(
            [0.36, 0.12, 0.12, 0.4],
            [
              @randf_pnorm(~hash, 0, 1),
              base_density,
              base_warm_plasticity,
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_HARDNESS
              );
            ]
          );
          ~hash = @prng(~hash);

          // Output variables:
          origin = MO_IGNEOUS_MINERAL;
          solid_density = @scale(base_density, 0.25, 5.0);
          liquid_density = @scale(base_density, 2.5, 3.2);
          gas_density = @scale(base_density, 1.8, 2.8);

          solid_specific_heat = @scale(base_sp_heat, 0.3, 1.4);
          liquid_specific_heat = @scale(base_sp_heat, 0.8, 2.2);
          gas_specific_heat = @scale(base_sp_heat, 0.65, 1.1);

          cold_damage_temp = 0;

          solidus = @scale(base_tr_temp, 550, 1250);
          liquidus = solidus + @randf_pnorm(~hash, 50, 250);
          ~hash = @prng(~hash);
          boiling_point = @scale(base_tr_temp, 1800, 2400);
          ignition_point = MAT_MAX_TEMP;
          flash_point = MAT_MAX_TEMP;

          cold_plastic_temp = solidus * @scale(base_plastic_temp, 0.2, 0.7);
          warm_plastic_temp = solidus * @scale(base_plastic_temp, 0.6, 1.0);

          // Roughly 75% of igneous stones will have 0 plasticity
          cold_plasticity = @max(0, @scale(base_cold_plasticity, -5, 10));
          warm_plasticity = @scale(base_warm_plasticity, 5, 80);

          viscosity = 10**(
            4
          + @randf_pnorm(~hash, 0, 2)
          + 3 * base_density
          + @scale(~cnst_plasticity, 3, 0)
          );

          hardness = @scale(base_hardness, 100, 220);
        };
        metamorphic_material = #material {
          // Base variables:
          base_density = @weight(
            [0.4, 0.6],
            [
              @randf(~hash),
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_DENSITY
              )
            ]
          );
          ~hash = @prng(~hash);
          base_sp_heat = @weight(
            [0.42, 0.18, 0.4],
            [
              @avg(
                @randf_pnorm(~hash, 0, 1),
                @randf_pnorm(@brng(~hash, 17), 0, 1)
              ),
              (1 - base_density),
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_SP_HEAT
              )
            ]
          );
          ~hash = @prng(~hash);
          base_tr_temp = @weight(
            [0.48, 0.12, 0.4],
            [
              @randf_pnorm(~hash, 0, 1),
              base_density,
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_TR_TEMP
              )
            ]
          );
          ~hash = @prng(~hash);
          base_plastic_temp = randf_pnorm(~hash, 0, 1)**1.2;
          ~hash = @prng(~hash);
          ~cnst_plasticity = @weighted_el_prop(
            composition.constituents,
            ~CONSTITUENT_AVERAGING_WEIGHTS,
            EL_PRP_F_STONE_TND_PLASTICITY
          );
          // More than 75% of values fall into [0, 0.33]:
          base_cold_plasticity = @weight(
            [0.5, 0.5],
            [ ~cnst_plasticity, @expdist(@randf_pnorm(~hash, 0, 1), 3) ]
          );
          ~hash = @prng(~hash);
          base_warm_plasticity = @weight(
            [0.5, 0.5],
            [ ~cnst_plasticity, @expdist(@randf_pnorm(~hash, 0, 1), 2) ]
          );
          ~hash = @prng(~hash);
          base_hardness = @weight(
            [0.36, 0.12, 0.12, 0.4],
            [
              @randf_pnorm(~hash, 0, 1)**0.5,
              base_density,
              base_warm_plasticity,
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_HARDNESS
              );
            ]
          );
          ~hash = @prng(~hash);

          // Output variables:
          origin = MO_METAMORPHIC_MINERAL;
          solid_density = @scale(base_density, 0.9, 5.7);
          liquid_density = @scale(base_density, 2.6, 3.2);
          gas_density = @scale(base_density, 1.9, 2.8);

          solid_specific_heat = @scale(base_sp_heat, 0.2, 1.5);
          liquid_specific_heat = @scale(base_sp_heat, 0.7, 2.4);
          gas_specific_heat = @scale(base_sp_heat, 0.6, 1.15);

          cold_damage_temp = 0;

          solidus = @scale(base_tr_temp, 520, 1280);
          liquidus = solidus + @randf_pnorm(~hash, 20, 320);
          ~hash = @prng(~hash);
          boiling_point = @scale(base_tr_temp, 1700, 2500);
          ignition_point = MAT_MAX_TEMP;
          flash_point = MAT_MAX_TEMP;

          cold_plastic_temp = solidus * @scale(base_plastic_temp, 0.3, 0.7);
          warm_plastic_temp = solidus * @scale(base_plastic_temp, 0.5, 1.0);

          cold_plasticity = @max(0, @scale(base_cold_plasticity, -3, 18));
          warm_plasticity = @scale(base_warm_plasticity, 10, 100);

          viscosity = 10**(
            6
          + 4 * @randf(~hash)
          + 3 * base_density
          + @scale(~cnst_plasticity, 3, 0)
          );

          hardness = @scale(base_hardness, 40, 230);
        };
        sedimentary_material = #material {
          // Base variables:
          base_density = @weight(
            [0.4, 0.6],
            [
              @randf(~hash),
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_DENSITY
              )
            ]
          );
          ~hash = @prng(~hash);
          base_sp_heat = @weight(
            [0.42, 0.18, 0.4],
            [
              @avg(
                @randf_pnorm(~hash, 0, 1),
                @randf_pnorm(@brng(~hash, 17), 0, 1)
              ),
              (1 - base_density),
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_SP_HEAT
              )
            ]
          );
          ~hash = @prng(~hash);
          base_tr_temp = @weight(
            [0.48, 0.12, 0.4],
            [
              @randf_pnorm(~hash, 0, 1),
              base_density,
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_TR_TEMP
              )
            ]
          );
          ~hash = @prng(~hash);
          base_plastic_temp = randf_pnorm(~hash, 0, 1)**1.3;
          ~hash = @prng(~hash);
          ~cnst_plasticity = @weighted_el_prop(
            composition.constituents,
            ~CONSTITUENT_AVERAGING_WEIGHTS,
            EL_PRP_F_STONE_TND_PLASTICITY
          );
          // More than 75% of values fall into [0, 0.33]:
          base_cold_plasticity = @weight(
            [0.2, 0.8],
            [ ~cnst_plasticity, @expdist(@randf_pnorm(~hash, 0, 1), 5.3) ]
          );
          ~hash = @prng(~hash);
          base_warm_plasticity = @weight(
            [0.4, 0.6],
            [ ~cnst_plasticity, @expdist(@randf_pnorm(~hash, 0, 1), 4) ]
          );
          ~hash = @prng(~hash);
          base_hardness = @weight(
            [0.36, 0.12, 0.12, 0.4],
            [
              @randf_pnorm(~hash, 0, 1)**0.8,
              base_density,
              base_warm_plasticity,
              @weighted_el_prop(
                composition.constituents,
                ~CONSTITUENT_AVERAGING_WEIGHTS,
                EL_PRP_F_STONE_TND_HARDNESS
              );
            ]
          );
          ~hash = @prng(~hash);

          // Output variables:
          origin = MO_SEDIMENTARY_MINERAL;
          solid_density = @scale(base_density, 1.2, 4.9);
          liquid_density = @scale(base_density, 2.6, 3.1);
          gas_density = @scale(base_density, 1.6, 2.9);

          solid_specific_heat = @scale(base_sp_heat, 0.4, 1.8);
          liquid_specific_heat = @scale(base_sp_heat, 0.9, 2.3);
          gas_specific_heat = @scale(base_sp_heat, 0.7, 1.2);

          cold_damage_temp = 0;

          solidus = @scale(base_tr_temp, 440, 1220);
          liquidus = solidus + @randf_pnorm(~hash, 10, 190);
          ~hash = @prng(~hash);
          boiling_point = @scale(base_tr_temp, 1550, 2150);
          // TODO: Fuel stones!
          ignition_point = MAT_MAX_TEMP;
          flash_point = MAT_MAX_TEMP;

          cold_plastic_temp = solidus * @scale(base_plastic_temp, 0.2, 0.7);
          warm_plastic_temp = solidus * @scale(base_plastic_temp, 0.5, 1.0);

          // Mostly 0 plasticity when cold
          cold_plasticity = @max(0, @scale(base_cold_plasticity, -7, 12));
          warm_plasticity = @scale(base_warm_plasticity, 0, 60);

          viscosity = 10**(
            5
          + @randf_pnorm(~hash, 0, 2)
          + 3 * base_density
          + @scale(~cnst_plasticity, 3, 0)
          );

          hardness = @scale(base_hardness, 30, 180);
        };
      ];
    );
    ~softness = @weight(
      weights = [ 0.15, 0.15, 0.35, 0.35 ]
      values = [
        @randf_pnorm(~hash, 0, 1),
        1 - material.base_density,
        material.base_cold_plasticity,
        1 - material.base_hardness
      ]
    );
    appearance = @choose(
      index = @index_of(~source, [GEO_IGNEOUS,GEO_METAMORPHIC,GEO_SEDIMENTARY]),
      values = [
        igneous_appearance = {
          ~hash = @brng(~seed, 1021820);
          ~soft = @scale(~softness, 0.15, 0.35);
          seed = ~hash;
          ~hash = @prng(~hash);
          scale = @scale(
            @weight(
              [0.8, 0.2],
              [@randf_pnorm(~hash, 0, 1), material.base_density]
            ),
            0.1,
            0.18
          );
          ~hash = @prng(~hash);
          gritty = @scale(
            @weight(
              [0.2, 0.2, 0.3, 0.3],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.14,
            0.37
          );
          ~hash = @prng(~hash);
          contoured = @scale(
            @weight(
              [0.2, 0.25, 0.35, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.35,
            0.7
          );
          ~hash = @prng(~hash);
          porous = @scale(
            @weight(
              [0.2, 0.4, 0.3, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.2,
            1.0
          );
          ~hash = @prng(~hash);
          bumpy = @scale(
            @weight(
              [0.4, 0.4, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.1,
            0.5
          );
          ~hash = @prng(~hash);
          layered = @scale(
            @weight(
              [0.2, 0.3, 0.3, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0,
            0.5
          );
          ~hash = @prng(~hash);
          layerscale = 1 / @scale(
            @weight(
              [0.3, 0.2, 0.2, 0.3],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            3,
            7
          );
          ~hash = @prng(~hash);
          layerwaves = @scale(
            @weight(
              [0.2, 0.2, 0.4, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.5,
            3.5
          );
          ~hash = @prng(~hash);
          wavescale = 1 / @scale(
            @weight(
              [0.3, 0.2, 0.2, 0.3],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            3,
            6
          );
          ~hash = @prng(~hash);
          inclusions = @weight(
            [0.6, 0.1, 0.2, 0.1],
            [
              @randf_pnorm(~hash, 0, 1),
              material.base_density,
              material.base_cold_plasticity,
              1 - material.hardness
            ]
          ) ** 2.5;
          ~hash = @prng(~hash);
          dscale = scale * @scale(
            @weight(
              [0.4, 0.2, 0.3, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.86,
            1.14
          );
          ~hash = @prng(~hash);
          distortion = 7 * @weight(
            [0.4, 0.2, 0.3, 0.1],
            [
              @randf_pnorm(~hash, 0, 1),
              1 - material.base_density,
              material.base_cold_plasticity,
              1 - material.hardness
            ]
          ) ** 1.5;
          ~hash = @prng(~hash);
          squash = (
            @scale(
              @weight(
                [0.6, 0.2, 0.2],
                [
                  @randf_pnorm(~hash, 0, 1),
                  material.base_density,
                  material.base_cold_plasticity,
                ]
              ),
              1 - soft,
              1 + soft
            );
          / @scale(
              @weight(
                [0.6, 0.2, 0.2],
                [
                  @randf_pnorm(@brng(~hash, 141), 0, 1),
                  1 - material.base_density,
                  1 - material.base_cold_plasticity,
                ]
              ),
              1 - soft,
              1 + soft
            );
          );
          ~hash = @prng(~hash);
          desaturate = @randf_pnorm(~hash, 0.4, 0.8);
          ~hash = @prng(~hash);
          sat_noise = @randf_pnorm(~hash, 0, 0.6);
          ~hash = @prng(~hash);
          brightness = @scale(
            @weight(
              [0.8, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
              ]
            ),
            -0.2,
            0.2
          );
          ~hash = @prng(~hash);
          base_color = @color__pixel(
            @COMPUTE_STONE_COLOR(
              ~constituents = composition.constituents,
              ~traces = composition.traces,
              ~source = ~source,
              ~density = material.base_density,
              ~plasticity = material.base_cold_plasticity,
              ~hardness = material.base_hardness,
              ~seed = ~hash,
            );
          );
          ~hash = @prng(~hash);
          alt_color = @color__pixel(
            #color {
              ~alt_base = @COMPUTE_STONE_COLOR(
                ~constituents = composition.constituents,
                ~traces = composition.traces,
                ~source = ~source,
                ~density = material.base_density,
                ~plasticity = material.base_cold_plasticity,
                ~hardness = material.base_hardness,
                ~seed = ~hash, // a different seed
              );
              fmt = ~alt_base.fmt;
              L = ~alt_base.L * @randf_pnorm(~hash, 0.4, 1.2);
              c = ~alt_base.c * @randf_pnorm(~hash, 0.6, 1.1);
              h = ~alt_base.h;
            };
          );
        };
        metamorphic_appearance = {
          ~hash = @brng(~seed, 1021821);
          ~soft = @scale(~softness, 0.25, 0.45);
          seed = ~hash;
          ~hash = @prng(~hash);
          scale = @randf_pnorm(~hash, 0.08, 0.2);
          ~hash = @prng(~hash);
          gritty = @scale(
            @weight(
              [0.2, 0.3, 0.3, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.08,
            0.33
          );
          ~hash = @prng(~hash);
          contoured = @scale(
            @weight(
              [0.4, 0.2, 0.3, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.15,
            0.9
          );
          ~hash = @prng(~hash);
          porous = @scale(
            @weight(
              [0.4, 0.3, 0.2, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.1,
            0.9
          );
          ~hash = @prng(~hash);
          bumpy = @scale(
            @weight(
              [0.4, 0.15, 0.25, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.1,
            1.0
          );
          ~hash = @prng(~hash);
          layered = @scale(
            @expdist(
              @weight(
                [0.4, 0.2, 0.3, 0.1],
                [
                  @randf_pnorm(~hash, 0, 1),
                  material.base_density,
                  material.base_cold_plasticity,
                  1 - material.hardness
                ]
              ),
              3
            ),
            0.1,
            0.8
          );
          ~hash = @prng(~hash);
          layerscale = 1 / @scale(
            @weight(
              [0.4, 0.1, 0.3, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                // Note higher divisors -> more compact layers
                material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            2.5,
            10
          );
          ~hash = @prng(~hash);
          layerwaves = @scale(
            @weight(
              [0.4, 0.2, 0.2, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.8,
            5
          );
          ~hash = @prng(~hash);
          wavescale = 1 / @scale(
            @weight(
              [0.4, 0.2, 0.2, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            3,
            7
          );
          ~hash = @prng(~hash);
          inclusions = @weight(
            [0.5, 0.2, 0.2, 0.1],
            [
              @randf_pnorm(~hash, 0, 1),
              material.base_density,
              material.base_cold_plasticity,
              1 - material.hardness
            ]
          ) ** 1.3;
          ~hash = @prng(~hash);
          dscale = scale * @scale(
            @weight(
              [0.6, 0.1, 0.2, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.9,
            1.1
          );
          ~hash = @prng(~hash);
          distortion = 6.5 * @weight(
            [0.4, 0.1, 0.3, 0.2],
            [
              @randf_pnorm(~hash, 0, 1),
              1 - material.base_density,
              material.base_cold_plasticity,
              1 - material.hardness
            ]
          );
          ~hash = @prng(~hash);
          squash = (
            @scale(
              @weight(
                [0.8, 0.1, 0.1],
                [
                  @randf_pnorm(~hash, 0, 1),
                  material.base_density,
                  material.base_cold_plasticity,
                ]
              ),
              1 - soft,
              1 + soft
            );
          / @scale(
              @weight(
                [0.8, 0.1, 0.1],
                [
                  @randf_pnorm(@brng(~hash, 141), 0, 1),
                  1 - material.base_density,
                  1 - material.base_cold_plasticity,
                ]
              ),
              1 - soft,
              1 + soft
            );
          );
          ~hash = @prng(~hash);
          desaturate = @randf_pnorm(~hash, 0.0, 0.6);
          ~hash = @prng(~hash);
          sat_noise = @randf_pnorm(~hash, 0.1, 0.9);
          ~hash = @prng(~hash);
          brightness = @scale(
            @weight(
              [0.9, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
              ]
            ),
            -0.25,
            0.35
          );
          ~hash = @prng(~hash);
          base_color = @color__pixel(
            @COMPUTE_STONE_COLOR(
              ~constituents = composition.constituents,
              ~traces = composition.traces,
              ~source = ~source,
              ~density = material.base_density,
              ~plasticity = material.base_cold_plasticity,
              ~hardness = material.base_hardness,
              ~seed = ~hash,
            );
          );
          ~hash = @prng(~hash);
          alt_color = @color__pixel(
            #color {
              ~alt_base = @COMPUTE_STONE_COLOR(
                ~constituents = composition.constituents,
                ~traces = composition.traces,
                ~source = ~source,
                ~density = material.base_density,
                ~plasticity = material.base_cold_plasticity,
                ~hardness = material.base_hardness,
                ~seed = ~hash, // a different seed
              );
              fmt = ~alt_base.fmt;
              L = ~alt_base.L;
              c = ~alt_base.c * @randf_pnorm(~hash, 1.0, 1.5);
              h = ~alt_base.h;
            };
          );
        };
        sedimentary_appearance = {
          ~hash = @brng(~seed, 1021822);
          ~soft = @scale(~softness, 0.2, 0.4);
          seed = ~hash;
          ~hash = @prng(~hash);
          scale = @randf_pnorm(~hash, 0.07, 0.14);
          ~hash = @prng(~hash);
          gritty = @scale(
            @weight(
              [0.3, 0.3, 0.2, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.31,
            0.56
          );
          ~hash = @prng(~hash);
          contoured = @scale(
            @weight(
              [0.4, 0.2, 0.3, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.15,
            1.1
          );
          ~hash = @prng(~hash);
          porous = @scale(
            @weight(
              [0.4, 0.3, 0.2, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.3,
            0.8
          );
          ~hash = @prng(~hash);
          bumpy = @scale(
            @weight(
              [0.4, 0.2, 0.2, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.3,
            0.9
          );
          ~hash = @prng(~hash);
          layered = @scale(
            @weight(
              [0.2, 0.3, 0.3, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            0.1,
            1.0
          );
          ~hash = @prng(~hash);
          layerscale = 1 / @scale(
            @weight(
              [0.4, 0.1, 0.3, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                material.base_cold_plasticity,
                1 - material.hardness
              ]
            ),
            2,
            10
          );
          ~hash = @prng(~hash);
          layerwaves = @scale(
            @expdist(
              @weight(
                [0.4, 0.2, 0.2, 0.2],
                [
                  @randf_pnorm(~hash, 0, 1),
                  1 - material.base_density,
                  material.base_cold_plasticity,
                  1 - material.hardness
                ]
              ),
              2
            ),
            0.5,
            4.5
          );
          ~hash = @prng(~hash);
          wavescale = 1 / @scale(
            @weight(
              [0.4, 0.2, 0.2, 0.2],
              [
                @randf_pnorm(~hash, 0, 1),
                // Note higher divisors -> more compact waves
                material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            2.5,
            5
          );
          ~hash = @prng(~hash);
          inclusions = @weight(
            [0.5, 0.2, 0.1, 0.2],
            [
              @randf_pnorm(~hash, 0, 1),
              1 - material.base_density,
              1 - material.base_cold_plasticity,
              1 - material.hardness
            ]
          ) ** 1.9;
          ~hash = @prng(~hash);
          dscale = scale * @scale(
            @weight(
              [0.6, 0.1, 0.2, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                material.base_density,
                1 - material.base_cold_plasticity,
                material.hardness
              ]
            ),
            0.8,
            1.2
          );
          ~hash = @prng(~hash);
          distortion = 4.5 * @weight(
            [0.4, 0.1, 0.3, 0.2],
            [
              @randf_pnorm(~hash, 0, 1),
              1 - material.base_density,
              material.base_cold_plasticity,
              1 - material.hardness
            ]
          ) ** 1.4;
          ~hash = @prng(~hash);
          squash = (
            @scale(
              @weight(
                [0.8, 0.1, 0.1],
                [
                  @randf_pnorm(~hash, 0, 1),
                  material.base_density,
                  material.base_cold_plasticity,
                ]
              ),
              0.9 - soft,
              0.9 + soft
            );
          / @scale(
              @weight(
                [0.8, 0.1, 0.1],
                [
                  @randf_pnorm(@brng(~hash, 141), 0, 1),
                  1 - material.base_density,
                  1 - material.base_cold_plasticity,
                ]
              ),
              1.1 - soft,
              1.1 + soft
            );
          );
          ~hash = @prng(~hash);
          desaturate = @randf_pnorm(~hash, 0.3, 0.8);
          ~hash = @prng(~hash);
          sat_noise = @randf_pnorm(~hash, 0.1, 0.7);
          ~hash = @prng(~hash);
          brightness = @scale(
            @weight(
              [0.9, 0.1],
              [
                @randf_pnorm(~hash, 0, 1),
                1 - material.base_density,
              ]
            ),
            -0.05,
            0.35
          );
          ~hash = @prng(~hash);
          base_color = @color__pixel(
            @COMPUTE_STONE_COLOR(
              ~constituents = composition.constituents,
              ~traces = composition.traces,
              ~source = ~source,
              ~density = material.base_density,
              ~plasticity = material.base_cold_plasticity,
              ~hardness = material.base_hardness,
              ~seed = ~hash,
            );
          );
          ~hash = @prng(~hash);
          alt_color = @color__pixel(
            #color {
              ~alt_base = @COMPUTE_STONE_COLOR(
                ~constituents = composition.constituents,
                ~traces = composition.traces,
                ~source = ~source,
                ~density = material.base_density,
                ~plasticity = material.base_cold_plasticity,
                ~hardness = material.base_hardness,
                ~seed = ~hash, // a different seed
              );
              fmt = ~alt_base.fmt;
              // Inclusions are generally darker and less saturated:
              L = ~alt_base.L * @randf_pnorm(~hash, 0.3, 1.3);
              c = ~alt_base.c * @randf_pnorm(~hash, 0.6, 1.2);
              h = ~alt_base.h;
            };
          );
        };
      ];
    );
  };
};

@gen_strata_params = #func {
  ~i = 0,
  ~species = SP_INVALID,
  ~wm_seed = 0,
  ~wm_width = 0,
  ~wm_height = 0,
} {
  ~hash = @brng(~wm_seed, ~i * 567);
  seed = ~hash;
  ~hash = @prng(~hash);
  species = ~species;
  profile = @choose(
    ~hash,
    [MFN_SPREAD_UP, MFN_TERRACE, MFN_HILL]
  );
  ~hash = @prng(~hash);
  thickness = BASE_STRATUM_THICKNESS * @exp(@randf_pnorm(~hash, -0.5, 3));
  ~hash = @prng(~hash);
  size = (
    STRATA_AVG_SIZE
  * WORLD_REGION_BLOCKS
  * @randf_pnorm(~hash, 0.6, 1.4)
  * (~wm_width * ~wm_height) ** 0.5
  );
  ~hash = @prng(~hash);
  cx = @randf_pnorm(~hash, 0, ~wm_width);
  cy = @randf_pnorm(~hash, 0, ~wm_height);
};

@gen_stratum = #func {
  ~params = {
    seed = 0,
    species = 0,
    source = 0,
    profile = 0,
    thickness = 0,
    size = 0,
    cx = 0,
    cy = 0,
  }
} {
  ~hash = @brng(~params.seed, 12098491);
  seed = ~params.seed;
  base_species = ~params.species;
  source = ~params.source;
  profile = ~params.profile;
  thickness = ~params.thickness;
  size = ~params.size;
  cx = ~params.cx;
  cy = ~params.cy;
  ~dependent_params = @choose(
    @index_of(source, [GEO_IGNEOUS, GEO_METAMORPHIC, GEO_SEDIMENTARY]),
    [
      igneous_stratum = {
        persistence = @randf_pnorm(~hash, 1.2, 1.6);
        ~hash = @prng(~hash);
        scale_bias = @randf_pnorm(~hash, 0.7, 1.1);
        ~hash = @prng(~hash);
        radial_frequency = M_PI / @randf_pnorm(~hash, 2.4, 4.0);
        ~hash = @prng(~hash);
        radial_variance = @randf_pnorm(~hash, 0.1, 0.4);
        ~hash = @prng(~hash);
        // TODO: more generalized scale?
        gross_distortion = @randf_pnorm(~hash, 900, 1400);
        ~hash = @prng(~hash);
        fine_distortion = @randf_pnorm(~hash, 110, 150);
        ~hash = @prng(~hash);
        large_var = thickness * @randf_pnorm(~hash, 0.6, 0.9);
        ~hash = @prng(~hash);
        med_var = thickness * @randf_pnorm(~hash, 0.4, 0.65);
        ~hash = @prng(~hash);
        small_var = thickness * @randf_pnorm(~hash, 0.17, 0.22);
        ~hash = @prng(~hash);
        tiny_var = thickness * @randf_pnorm(~hash, 0.04, 0.10);
        ~hash = @prng(~hash);
        detail_var = @randf_pnorm(~hash, 1, 3);
        ~hash = @prng(~hash);
        ridges = @randf_pnorm(~hash, 2, 5);
        ~hash = @prng(~hash);
        smoothing = @randf_pnorm(~hash, 0.15, 0.35);
        ~hash = @prng(~hash);
        // TODO: Vein scale/strength/species
        vein_scales = [];
        vein_strengths = [];
        vein_species = [];
        // TODO: inclusion frequency/species
        inclusion_frequencies = [];
        inclusion_species = [];
      };
      metamorphic_stratum = {
        // TODO: More influence on these values from the stone properties...
        persistence = @randf_pnorm(~hash, 0.8, 1.3);
        ~hash = @prng(~hash);
        scale_bias = @randf_pnorm(~hash, 0.8, 1.2);
        ~hash = @prng(~hash);
        radial_frequency = M_PI / @randf_pnorm(~hash, 2.8, 4.8);
        ~hash = @prng(~hash);
        radial_variance = @randf_pnorm(~hash, 0.4, 0.8);
        ~hash = @prng(~hash);
        // TODO: more generalized scale?
        gross_distortion = @randf_pnorm(~hash, 1200, 2100);
        ~hash = @prng(~hash);
        fine_distortion = @randf_pnorm(~hash, 180, 290);
        ~hash = @prng(~hash);
        large_var = thickness * @randf_pnorm(~hash, 0.5, 0.8);
        ~hash = @prng(~hash);
        med_var = thickness * @randf_pnorm(~hash, 0.3, 0.55);
        ~hash = @prng(~hash);
        small_var = thickness * @randf_pnorm(~hash, 0.16, 0.22);
        ~hash = @prng(~hash);
        tiny_var = thickness * @randf_pnorm(~hash, 0.02, 0.05);
        ~hash = @prng(~hash);
        detail_var = @randf_pnorm(~hash, 0.3, 2.1);
        ~hash = @prng(~hash);
        ridges = @randf_pnorm(~hash, 0.4, 3.8);
        ~hash = @prng(~hash);
        smoothing = @randf_pnorm(~hash, 0.15, 0.6);
        ~hash = @prng(~hash);
        // TODO: Vein scale/strength/species
        vein_scales = [];
        vein_strengths = [];
        vein_species = [];
        // TODO: inclusion frequency/species
        inclusion_frequencies = [];
        inclusion_species = [];
      };
      sedimentary_stratum = {
        // TODO: More influence on these values from the stone properties...
        persistence = @randf_pnorm(~hash, 1.3, 1.8);
        ~hash = @prng(~hash);
        scale_bias = @randf_pnorm(~hash, 1.1, 1.4);
        ~hash = @prng(~hash);
        radial_frequency = M_PI / @randf_pnorm(~hash, 2.1, 3.3);
        ~hash = @prng(~hash);
        radial_variance = @randf_pnorm(~hash, 0.05, 0.25);
        ~hash = @prng(~hash);
        // TODO: more generalized scale?
        gross_distortion = @randf_pnorm(~hash, 700, 1100);
        ~hash = @prng(~hash);
        fine_distortion = @randf_pnorm(~hash, 30, 60);
        ~hash = @prng(~hash);
        large_var = thickness * @randf_pnorm(~hash, 0.4, 0.65);
        ~hash = @prng(~hash);
        med_var = thickness * @randf_pnorm(~hash, 0.2, 0.35);
        ~hash = @prng(~hash);
        small_var = thickness * @randf_pnorm(~hash, 0.11, 0.16);
        ~hash = @prng(~hash);
        tiny_var = thickness * @randf_pnorm(~hash, 0.03, 0.10);
        ~hash = @prng(~hash);
        detail_var = @randf_pnorm(~hash, 0.7, 3.9);
        ~hash = @prng(~hash);
        ridges = @randf_pnorm(~hash, 0.8, 5.3);
        ~hash = @prng(~hash);
        smoothing = @randf_pnorm(~hash, 0.15, 0.6);
        ~hash = @prng(~hash);
        // TODO: Vein scale/strength/species
        vein_scales = [];
        vein_strengths = [];
        vein_species = [];
        // TODO: inclusion frequency/species
        inclusion_frequencies = [];
        inclusion_species = [];
      };
    ];
  );
  !unpack ~dependent_params;
};

</gen.geo>
