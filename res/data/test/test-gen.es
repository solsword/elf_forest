<elfscript.internal.test>
<unit.gen>

deep_underground_hanging_flora_diversity = !f (!seed) {
  !return @rng(
    !seed,
    min = @sample(!seed, [ 0.5, 0.4, 0.1 ]),
    max = @sample(!seed, [ 0.15, 0.25, 0.35, 0.05, 0.05, 0.15 ]),
  );
};

SOURCE_DISTRIBUTION = #rngtable {
  [ $GEO_IGNEOUS, $GEO_METAMORPHIC, $GEO_SEDIMENTARY ],
  [ 0.35, 0.25, 0.4 ]
};


COMP_EXPANSION_TABLE = {
  $MNRL_COMP_STONE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_NONE, $EL_CATEGORY_NONE,
  ],
  $MNRL_COMP_LIFE = [
    $EL_CATEGORY_LIFE, $EL_CATEGORY_NONE, $EL_CATEGORY_NONE,
  ],
  $MNRL_COMP_STONE_AIR = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_AIR, $EL_CATEGORY_NONE,
  ],
  $MNRL_COMP_STONE_WATER = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_WATER, $EL_CATEGORY_NONE,
  ],
  $MNRL_COMP_STONE_LIFE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_LIFE, $EL_CATEGORY_NONE,
  ],
  $MNRL_COMP_STONE_STONE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_STONE, $EL_CATEGORY_NONE,
  ],
  $MNRL_COMP_STONE_STONE_LIFE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_STONE, $EL_CATEGORY_LIFE,
  ],
  $MNRL_COMP_STONE_STONE_STONE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_STONE, $EL_CATEGORY_STONE,
  ],
  $MNRL_COMP_STONE_METAL = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_METAL, $EL_CATEGORY_NONE,
  ],
  $MNRL_COMP_STONE_STONE_METAL = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_STONE, $EL_CATEGORY_METAL,
  ],
  $MNRL_COMP_STONE_METAL_METAL = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_METAL, $EL_CATEGORY_METAL,
  ],
  $MNRL_COMP_STONE_METAL_RARE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_METAL, $EL_CATEGORY_RARE,
  ],
  $MNRL_COMP_STONE_RARE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_RARE, $EL_CATEGORY_NONE,
  ],
  $MNRL_COMP_STONE_STONE_RARE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_STONE, $EL_CATEGORY_RARE,
  ],
  $MNRL_COMP_STONE_RARE_RARE = [
    $EL_CATEGORY_STONE, $EL_CATEGORY_RARE, $EL_CATEGORY_RARE,
  ],
  $MNRL_COMP_RARE_RARE = [
    $EL_CATEGORY_RARE, $EL_CATEGORY_RARE, $EL_CATEGORY_NONE,
  ],
};

expand_composition = !f (composition = $MNRL_COMP_STONE) {
  !return @get(~COMP_EXPANSION_TABLE, composition);
};


CNST_AVG_WEIGHTS = [ 1.5, 1.0, 0.7 ];

COMPUTE_STONE_COLOR = !f (
  constituents = NULL,
  traces = NULL,
  source = 0,
  density = 0,
  plasticity = 0,
  hardness = 0,
  !seed 
) {
  el_brightness = (
    0.3 * 100 * @avg(@rng(!seed), @rng(!seed))
  + 0.7 * @weighted_el_prop(
      constituents,
      ~CNST_AVG_WEIGHTS,
      $EL_PRP_F_STONE_TND_BRIGHTNESS
    )
  );
  per_constituent = @unroll(
    // TODO: Better color mixing here
    @map(
      {
        constituents,
        @range(1,)
      },
      !f (cnst, i) {
        weight = @weight(
          [ 0.5, 0.5 ],
          @expdist(
            @rng(!seed(i)), 
            @if(~source == $GEO_METAMORPHIC, 1.5, 4)
          ),
          @if(
            ~source == $GEO_METAMORPHIC,
            @rng(!seed(i)),
            @rng_pnorm(!seed(i), min = 0.4, max = 1)
          )
        );
        x = weight * @cos(
          @get_el_attr(cnst, $EL_PRP_F_STONE_TND_CHROMA)
        );
        y = weight * @sin(
          @get_el_attr(cnst, $EL_PRP_F_STONE_TND_CHROMA)
        );
        !return { ~x, ~y };
      }
    )
  );
  avg_chroma = @avg(per_constituent);
  el_hue = @atan2(avg_chroma.x, avg_chroma.y);
  el_intensity = (avg_chroma.x**2 + avg_chroma.y**2)**0.5;
  el_saturation = @scale(
    @expdist(
      el_intensity,
      @if(source == $GEO_METAMORPHIC, 2, 3.5)
    ),
    min = 60,
    max = 90
  ) * @weight(
    [ 0.7, 0.3 ],
    @expdist(
      @rng(!seed),
      @if(source == GEO_METAMORPHIC, 2.7, 5)
    ),
    el_intensity
  );
  color = @switch(
    source,
    {
      $GEO_IGNEOUS = !f () {
        !return {
          fmt = $CFMT_LCH;
          L = @weight(
              [ 0.6, 0.4 ],
              el_brightness,
              100 * @weight(
                [ 0.6, 0.4 ],
                @scale(density, 1, 0),
                plasticity
              )
          ) ** 0.8;
          c = el_saturation * @if(
            0.7 < @rng(!seed),
            @rng_pnorm(!seed, 0.05, 0.3),
            @rng_pnorm(!seed, 0.6, 0.8),
          );
          h = el_hue + @rng_pnorm(!seed, -$M_PI / 16, $M_PI / 16);
        };
      };
      $GEO_METAMORPHIC = !f () {
        !return {
          fmt = $CFMT_LCH;
          L = el_brightness;
          _boost = @rng_pnorm(!seed, 0, 0.4)
          c = @weight(
            [ _boost, 1 - _boost ],
            el_saturation,
            @scale(@rng(!seed), 30, 100)
          );
          h = el_hue + @rng_pnorm(!seed, -$M_PI / 6, $M_PI / 6);
        };
      };
      $GEO_SEDIMENTARY = !f () {
        !return {
          fmt = $CFMT_LCH;
          L = el_brightness * @rng_pnorm(!seed, 1, 1.4);
          c = el_saturation * @expdist(@rng_pnorm(!seed, 0.3, 1), 2);
          h = el_hue + @rng_pnorm(!seed, -M_PI/12, M_PI/12);
        };
      };
    }
  );
  }
};

</unit.gen>
</elfscript.internal.test>
