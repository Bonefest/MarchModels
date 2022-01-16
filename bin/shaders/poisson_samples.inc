
#ifndef POISSON_SAMPLES_INC
#define POISSON_SAMPLES_INC

// ----------------------------------------------------------------------------
// 12 samples, radius 1
// source: https://github.com/spite/Wagner/blob/master/fragment-shaders/poisson-disc-blur-fs.glsl
// ----------------------------------------------------------------------------
#ifdef POISSON_DISK_12S_1R

#define POISSON_SAMPLES_COUNT 12
#define POISSON_DISK_RADIUS   1

const vec2 PoissonSamples[POISSON_SAMPLES_COUNT] = vec2[POISSON_SAMPLES_COUNT]
(
  vec2(-0.326f, -0.406f),
  vec2(-0.840f, -0.074f),
  vec2(-0.696f,  0.457f),
  vec2(-0.203f,  0.621f),
  vec2( 0.962f, -0.195f),
  vec2( 0.473f, -0.480f),
  vec2( 0.519f,  0.767f),
  vec2( 0.185f, -0.893f),
  vec2( 0.507f,  0.064f),
  vec2( 0.896f,  0.412f),
  vec2(-0.322f, -0.933f),
  vec2(-0.792f, -0.598f)
);

// ----------------------------------------------------------------------------
// 52 samples, radius 15
// source: https://github.com/GPUOpen-Effects/ShadowFX/blob/master/amd_shadowfx/src/Shaders/AMD_SHADOWFX_FILTER_SIZE_15_POISSON.inc
// ----------------------------------------------------------------------------
#elif defined(POISSON_DISK_52S_15R)

#define POISSON_SAMPLES_COUNT 52
#define POISSON_DISK_RADIUS   15

const vec2 PoissonSamples[POISSON_SAMPLES_COUNT] = vec2[POISSON_SAMPLES_COUNT]
(
  vec2(-3.412354f,  -5.284019f),
  vec2(-3.254736f,  -3.330548f),
  vec2(-5.816196f,  -3.46201f),
  vec2(-1.492874f,  -6.775923f),
  vec2(-1.363082f,  -4.576695f),
  vec2( 0.5120577f, -7.036356f),
  vec2( 0.8487682f, -5.458392f),
  vec2(-2.482593f,  -1.455628f),
  vec2(-5.299086f,  -4.989777f),
  vec2(-4.506917f,  -2.084349f),
  vec2(-0.9384603f, -2.567248f),
  vec2( 2.992283f,  -6.154196f),
  vec2(-6.816841f,  -0.8215709f),
  vec2(-6.176595f,   0.8565118f),
  vec2(-4.742037f,   1.505409f),
  vec2(-4.878184f,  -0.06490026f),
  vec2(-2.135786f,   2.793251f),
  vec2(-3.964286f,   3.217015f),
  vec2(-3.306162f,   0.3788477f),
  vec2(-6.248262f,   3.178992f),
  vec2(-0.9384207f,  1.089011f),
  vec2( 2.814241f,  -4.365722f),
  vec2( 4.603385f,  -5.340619f),
  vec2(-2.667056f,   4.525906f),
  vec2(-3.413226f,   6.194156f),
  vec2(-5.432701f,   4.530499f),
  vec2( 0.5791532f, -1.817929f),
  vec2( 0.3678799f, -3.957979f),
  vec2( 0.2722762f,  4.098415f),
  vec2( 0.2881673f,  2.266828f),
  vec2(-1.376121f,   5.507777f),
  vec2( 4.755373f,  -3.748413f),
  vec2(-0.8779002f, -0.7270181f),
  vec2( 2.496858f,  -0.6773291f),
  vec2( 1.174333f,   0.8396815f),
  vec2( 3.040598f,  -2.650676f),
  vec2( 2.469612f,   2.670604f),
  vec2( 6.43206f,   -3.162919f),
  vec2( 4.905748f,  -1.159387f),
  vec2( 6.753581f,  -1.277563f),
  vec2( 4.555129f,   4.07637f),
  vec2( 3.289762f,   5.106628f),
  vec2( 4.039778f,   1.9229f),
  vec2( 1.417698f,   5.273744f),
  vec2( 6.49455f,    0.9413887f),
  vec2( 5.521292f,   2.777053f),
  vec2( 2.831632f,   0.8962862f),
  vec2( 2.649947f,   6.684711f),
  vec2( 0.9264914f,  6.971447f),
  vec2( 4.845179f,   5.70947f),
  vec2( 4.915648f,   0.6151388f)
);

#endif

#endif
