// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#ifdef ALI_TYPE_FEATURE_LAYERS

#undef ALI_TYPENAME
#define ALI_TYPENAME FeatureLayers

#ifdef ALI_TYPEFIELD
 ALI_TYPEFIELD(ImageData, SET, 1,  HeightMap)
 ALI_TYPEFIELD(ImageData, SET, 2,  VisibilityMap)
 ALI_TYPEFIELD(ImageData, SET, 3,  Creep)
 ALI_TYPEFIELD(ImageData, SET, 4,  Power)
 ALI_TYPEFIELD(ImageData, SET, 5,  PlayerId)
 ALI_TYPEFIELD(ImageData, SET, 6,  UnitType)
 ALI_TYPEFIELD(ImageData, SET, 7,  Selected)
 ALI_TYPEFIELD(ImageData, SET, 8,  UnitHitPoints)
 ALI_TYPEFIELD(ImageData, SET, 17, UnitHitPointsRatio)
 ALI_TYPEFIELD(ImageData, SET, 9,  UnitEnergy)
 ALI_TYPEFIELD(ImageData, SET, 18, UnitEnergyRatio)
 ALI_TYPEFIELD(ImageData, SET, 10, UnitShields)
 ALI_TYPEFIELD(ImageData, SET, 19, UnitShieldsRatio)
 ALI_TYPEFIELD(ImageData, SET, 11, PlayerRelative)
 ALI_TYPEFIELD(ImageData, SET, 14, UnitDensityAA)
 ALI_TYPEFIELD(ImageData, SET, 15, UnitDensity)
 ALI_TYPEFIELD(ImageData, SET, 20, Effects)
 ALI_TYPEFIELD(ImageData, SET, 21, Hallucinations)
 ALI_TYPEFIELD(ImageData, SET, 22, Cloaked)
 ALI_TYPEFIELD(ImageData, SET, 23, Blip)
 ALI_TYPEFIELD(ImageData, SET, 24, Buffs)
 ALI_TYPEFIELD(ImageData, SET, 26, BuffDuration)
 ALI_TYPEFIELD(ImageData, SET, 25, Active)
 ALI_TYPEFIELD(ImageData, SET, 27, BuildProgress)
 ALI_TYPEFIELD(ImageData, SET, 28, Buildable)
 ALI_TYPEFIELD(ImageData, SET, 29, Pathable)
 ALI_TYPEFIELD(ImageData, SET, 30, Placeholder)
#undef ALI_TYPEFIELD
#endif
#endif

#ifdef ALI_TYPE_FEATURE_LAYERS_MINIMAP
#undef ALI_TYPENAME
#define ALI_TYPENAME FeatureLayersMinimap
#ifdef ALI_TYPEFIELD
  ALI_TYPEFIELD(ImageData, SET, 1,   HeightMap)
  ALI_TYPEFIELD(ImageData, SET, 2,   VisibilityMap)
  ALI_TYPEFIELD(ImageData, SET, 3,   Creep)
  ALI_TYPEFIELD(ImageData, SET, 4,   Camera)
  ALI_TYPEFIELD(ImageData, SET, 5,   PlayerId)
  ALI_TYPEFIELD(ImageData, SET, 6,   PlayerRelative)
  ALI_TYPEFIELD(ImageData, SET, 7,   Selected)
  ALI_TYPEFIELD(ImageData, SET, 9,   Alerts)
  ALI_TYPEFIELD(ImageData, SET, 10,  Buildable)
  ALI_TYPEFIELD(ImageData, SET, 11,  Pathable)
  ALI_TYPEFIELD(ImageData, SET, 8,   UnitType)
#undef ALI_TYPEFIELD
#endif
#endif

#ifdef ALI_TYPE_OBSERVATION_RENDER
#undef ALI_TYPENAME
#define ALI_TYPENAME ObservationRender
#ifdef ALI_TYPEFIELD
  ALI_TYPEFIELD(ImageData, SET, 1, Map)
  ALI_TYPEFIELD(ImageData, SET, 2, Minimap)
#undef ALI_TYPEFIELD
#endif
#endif

