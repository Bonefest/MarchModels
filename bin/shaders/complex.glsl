#ifndef COMPLEX_GLSL_INCLUDED
#define COMPLEX_GLSL_INCLUDED

  #include defines.glsl

  complex complexOne()
  {
    return complex(1.0f, 0.0f);
  }

  complex complexI()
  {
    return complex(0.0f, 1.0f);
  }

  complex complexConj(complex c)
  {
    return complex(c.x, -c.y);
  }

  complex complexMult(complex a, complex b)
  {
    return complex(a.x * b.x - a.y * b.y, a.x * b.y + b.x * a.y);
  }

  float32 complexLen2(complex c)
  {
    return c.x * c.x + c.y * c.y;
  }

  float32 complexLen(complex c)
  {
    return sqrt(c.x * c.x + c.y * c.y);
  }

#endif
