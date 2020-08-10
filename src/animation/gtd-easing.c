/* gtd-easing.c
 *
 * Copyright 2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "gtd-easing.h"

#include <math.h>

gdouble
gtd_linear (gdouble t,
            gdouble d)
{
  return t / d;
}

gdouble
gtd_ease_in_quad (gdouble t,
                  gdouble d)
{
  gdouble p = t / d;

  return p * p;
}

gdouble
gtd_ease_out_quad (gdouble t,
                   gdouble d)
{
  gdouble p = t / d;

  return -1.0 * p * (p - 2);
}

gdouble
gtd_ease_in_out_quad (gdouble t,
                      gdouble d)
{
  gdouble p = t / (d / 2);

  if (p < 1)
    return 0.5 * p * p;

  p -= 1;

  return -0.5 * (p * (p - 2) - 1);
}

gdouble
gtd_ease_in_cubic (gdouble t,
                   gdouble d)
{
  gdouble p = t / d;

  return p * p * p;
}

gdouble
gtd_ease_out_cubic (gdouble t,
                    gdouble d)
{
  gdouble p = t / d - 1;

  return p * p * p + 1;
}

gdouble
gtd_ease_in_out_cubic (gdouble t,
                       gdouble d)
{
  gdouble p = t / (d / 2);

  if (p < 1)
    return 0.5 * p * p * p;

  p -= 2;

  return 0.5 * (p * p * p + 2);
}

gdouble
gtd_ease_in_quart (gdouble t,
                   gdouble d)
{
  gdouble p = t / d;

  return p * p * p * p;
}

gdouble
gtd_ease_out_quart (gdouble t,
                    gdouble d)
{
  gdouble p = t / d - 1;

  return -1.0 * (p * p * p * p - 1);
}

gdouble
gtd_ease_in_out_quart (gdouble t,
                       gdouble d)
{
  gdouble p = t / (d / 2);

  if (p < 1)
    return 0.5 * p * p * p * p;

  p -= 2;

  return -0.5 * (p * p * p * p - 2);
}

gdouble
gtd_ease_in_quint (gdouble t,
                   gdouble d)
 {
  gdouble p = t / d;

  return p * p * p * p * p;
}

gdouble
gtd_ease_out_quint (gdouble t,
                    gdouble d)
{
  gdouble p = t / d - 1;

  return p * p * p * p * p + 1;
}

gdouble
gtd_ease_in_out_quint (gdouble t,
                       gdouble d)
{
  gdouble p = t / (d / 2);

  if (p < 1)
    return 0.5 * p * p * p * p * p;

  p -= 2;

  return 0.5 * (p * p * p * p * p + 2);
}

gdouble
gtd_ease_in_sine (gdouble t,
                  gdouble d)
{
  return -1.0 * cos (t / d * G_PI_2) + 1.0;
}

gdouble
gtd_ease_out_sine (gdouble t,
                   gdouble d)
{
  return sin (t / d * G_PI_2);
}

gdouble
gtd_ease_in_out_sine (gdouble t,
                      gdouble d)
{
  return -0.5 * (cos (G_PI * t / d) - 1);
}

gdouble
gtd_ease_in_expo (gdouble t,
                  gdouble d)
{
  return (t == 0) ? 0.0 : pow (2, 10 * (t / d - 1));
}

gdouble
gtd_ease_out_expo (gdouble t,
                   gdouble d)
{
  return (t == d) ? 1.0 : -pow (2, -10 * t / d) + 1;
}

gdouble
gtd_ease_in_out_expo (gdouble t,
                      gdouble d)
{
  gdouble p;

  if (t == 0)
    return 0.0;

  if (t == d)
    return 1.0;

  p = t / (d / 2);

  if (p < 1)
    return 0.5 * pow (2, 10 * (p - 1));

  p -= 1;

  return 0.5 * (-pow (2, -10 * p) + 2);
}

gdouble
gtd_ease_in_circ (gdouble t,
                  gdouble d)
{
  gdouble p = t / d;

  return -1.0 * (sqrt (1 - p * p) - 1);
}

gdouble
gtd_ease_out_circ (gdouble t,
                   gdouble d)
{
  gdouble p = t / d - 1;

  return sqrt (1 - p * p);
}

gdouble
gtd_ease_in_out_circ (gdouble t,
                      gdouble d)
{
  gdouble p = t / (d / 2);

  if (p < 1)
    return -0.5 * (sqrt (1 - p * p) - 1);

  p -= 2;

  return 0.5 * (sqrt (1 - p * p) + 1);
}

gdouble
gtd_ease_in_elastic (gdouble t,
                     gdouble d)
{
  gdouble p = d * .3;
  gdouble s = p / 4;
  gdouble q = t / d;

  if (q == 1)
    return 1.0;

  q -= 1;

  return -(pow (2, 10 * q) * sin ((q * d - s) * (2 * G_PI) / p));
}

gdouble
gtd_ease_out_elastic (gdouble t,
                      gdouble d)
{
  gdouble p = d * .3;
  gdouble s = p / 4;
  gdouble q = t / d;

  if (q == 1)
    return 1.0;

  return pow (2, -10 * q) * sin ((q * d - s) * (2 * G_PI) / p) + 1.0;
}

gdouble
gtd_ease_in_out_elastic (gdouble t,
                         gdouble d)
{
  gdouble p = d * (.3 * 1.5);
  gdouble s = p / 4;
  gdouble q = t / (d / 2);

  if (q == 2)
    return 1.0;

  if (q < 1)
    {
      q -= 1;

      return -.5 * (pow (2, 10 * q) * sin ((q * d - s) * (2 * G_PI) / p));
    }
  else
    {
      q -= 1;

      return pow (2, -10 * q)
           * sin ((q * d - s) * (2 * G_PI) / p)
           * .5 + 1.0;
    }
}

gdouble
gtd_ease_in_back (gdouble t,
                  gdouble d)
{
  gdouble p = t / d;

  return p * p * ((1.70158 + 1) * p - 1.70158);
}

gdouble
gtd_ease_out_back (gdouble t,
                   gdouble d)
{
  gdouble p = t / d - 1;

  return p * p * ((1.70158 + 1) * p + 1.70158) + 1;
}

gdouble
gtd_ease_in_out_back (gdouble t,
                      gdouble d)
{
  gdouble p = t / (d / 2);
  gdouble s = 1.70158 * 1.525;

  if (p < 1)
    return 0.5 * (p * p * ((s + 1) * p - s));

  p -= 2;

  return 0.5 * (p * p * ((s + 1) * p + s) + 2);
}

static inline gdouble
ease_out_bounce_internal (gdouble t,
                          gdouble d)
{
  gdouble p = t / d;

  if (p < (1 / 2.75))
    {
      return 7.5625 * p * p;
    }
  else if (p < (2 / 2.75))
    {
      p -= (1.5 / 2.75);

      return 7.5625 * p * p + .75;
    }
  else if (p < (2.5 / 2.75))
    {
      p -= (2.25 / 2.75);

      return 7.5625 * p * p + .9375;
    }
  else
    {
      p -= (2.625 / 2.75);

      return 7.5625 * p * p + .984375;
    }
}

static inline gdouble
ease_in_bounce_internal (gdouble t,
                         gdouble d)
{
  return 1.0 - ease_out_bounce_internal (d - t, d);
}

gdouble
gtd_ease_in_bounce (gdouble t,
                    gdouble d)
{
  return ease_in_bounce_internal (t, d);
}

gdouble
gtd_ease_out_bounce (gdouble t,
                     gdouble d)
{
  return ease_out_bounce_internal (t, d);
}

gdouble
gtd_ease_in_out_bounce (gdouble t,
                        gdouble d)
{
  if (t < d / 2)
    return ease_in_bounce_internal (t * 2, d) * 0.5;
  else
    return ease_out_bounce_internal (t * 2 - d, d) * 0.5 + 1.0 * 0.5;
}

static inline gdouble
ease_steps_end (gdouble p,
                int    n_steps)
{
  return floor (p * (gdouble) n_steps) / (gdouble) n_steps;
}

gdouble
gtd_ease_steps_start (gdouble t,
                      gdouble d,
                      int     n_steps)
{
  return 1.0 - ease_steps_end (1.0 - (t / d), n_steps);
}

gdouble
gtd_ease_steps_end (gdouble t,
                    gdouble d,
                    int     n_steps)
{
  return ease_steps_end ((t / d), n_steps);
}

static inline gdouble
x_for_t (gdouble t,
         gdouble x_1,
         gdouble x_2)
{
  gdouble omt = 1.0 - t;

  return 3.0 * omt * omt * t * x_1
       + 3.0 * omt * t * t * x_2
       + t * t * t;
}

static inline gdouble
y_for_t (gdouble t,
         gdouble y_1,
         gdouble y_2)
{
  gdouble omt = 1.0 - t;

  return 3.0 * omt * omt * t * y_1
       + 3.0 * omt * t * t * y_2
       + t * t * t;
}

static inline gdouble
t_for_x (gdouble x,
         gdouble x_1,
         gdouble x_2)
{
  gdouble min_t = 0, max_t = 1;
  int i;

  for (i = 0; i < 30; ++i)
    {
      gdouble guess_t = (min_t + max_t) / 2.0;
      gdouble guess_x = x_for_t (guess_t, x_1, x_2);

      if (x < guess_x)
        max_t = guess_t;
      else
        min_t = guess_t;
    }

  return (min_t + max_t) / 2.0;
}

gdouble
gtd_ease_cubic_bezier (gdouble t,
                       gdouble d,
                       gdouble x_1,
                       gdouble y_1,
                       gdouble x_2,
                       gdouble y_2)
{
  gdouble p = t / d;

  if (p == 0.0)
    return 0.0;

  if (p == 1.0)
    return 1.0;

  return y_for_t (t_for_x (p, x_1, x_2), y_1, y_2);
}

/*< private >
 * _gtd_animation_modes:
 *
 * A mapping of animation modes and easing functions.
 */
static const struct {
  GtdEaseMode mode;
  GtdEaseFunc func;
  const char *name;
} _gtd_animation_modes[] = {
  { GTD_CUSTOM_MODE,         NULL, "custom" },

  { GTD_EASE_LINEAR,         gtd_linear, "linear" },
  { GTD_EASE_IN_QUAD,        gtd_ease_in_quad, "easeInQuad" },
  { GTD_EASE_OUT_QUAD,       gtd_ease_out_quad, "easeOutQuad" },
  { GTD_EASE_IN_OUT_QUAD,    gtd_ease_in_out_quad, "easeInOutQuad" },
  { GTD_EASE_IN_CUBIC,       gtd_ease_in_cubic, "easeInCubic" },
  { GTD_EASE_OUT_CUBIC,      gtd_ease_out_cubic, "easeOutCubic" },
  { GTD_EASE_IN_OUT_CUBIC,   gtd_ease_in_out_cubic, "easeInOutCubic" },
  { GTD_EASE_IN_QUART,       gtd_ease_in_quart, "easeInQuart" },
  { GTD_EASE_OUT_QUART,      gtd_ease_out_quart, "easeOutQuart" },
  { GTD_EASE_IN_OUT_QUART,   gtd_ease_in_out_quart, "easeInOutQuart" },
  { GTD_EASE_IN_QUINT,       gtd_ease_in_quint, "easeInQuint" },
  { GTD_EASE_OUT_QUINT,      gtd_ease_out_quint, "easeOutQuint" },
  { GTD_EASE_IN_OUT_QUINT,   gtd_ease_in_out_quint, "easeInOutQuint" },
  { GTD_EASE_IN_SINE,        gtd_ease_in_sine, "easeInSine" },
  { GTD_EASE_OUT_SINE,       gtd_ease_out_sine, "easeOutSine" },
  { GTD_EASE_IN_OUT_SINE,    gtd_ease_in_out_sine, "easeInOutSine" },
  { GTD_EASE_IN_EXPO,        gtd_ease_in_expo, "easeInExpo" },
  { GTD_EASE_OUT_EXPO,       gtd_ease_out_expo, "easeOutExpo" },
  { GTD_EASE_IN_OUT_EXPO,    gtd_ease_in_out_expo, "easeInOutExpo" },
  { GTD_EASE_IN_CIRC,        gtd_ease_in_circ, "easeInCirc" },
  { GTD_EASE_OUT_CIRC,       gtd_ease_out_circ, "easeOutCirc" },
  { GTD_EASE_IN_OUT_CIRC,    gtd_ease_in_out_circ, "easeInOutCirc" },
  { GTD_EASE_IN_ELASTIC,     gtd_ease_in_elastic, "easeInElastic" },
  { GTD_EASE_OUT_ELASTIC,    gtd_ease_out_elastic, "easeOutElastic" },
  { GTD_EASE_IN_OUT_ELASTIC, gtd_ease_in_out_elastic, "easeInOutElastic" },
  { GTD_EASE_IN_BACK,        gtd_ease_in_back, "easeInBack" },
  { GTD_EASE_OUT_BACK,       gtd_ease_out_back, "easeOutBack" },
  { GTD_EASE_IN_OUT_BACK,    gtd_ease_in_out_back, "easeInOutBack" },
  { GTD_EASE_IN_BOUNCE,      gtd_ease_in_bounce, "easeInBounce" },
  { GTD_EASE_OUT_BOUNCE,     gtd_ease_out_bounce, "easeOutBounce" },
  { GTD_EASE_IN_OUT_BOUNCE,  gtd_ease_in_out_bounce, "easeInOutBounce" },

  /* the parametrized functions need a cast */
  { GTD_STEPS,               (GtdEaseFunc) gtd_ease_steps_end, "steps" },
  { GTD_STEP_START,          (GtdEaseFunc) gtd_ease_steps_start, "stepStart" },
  { GTD_STEP_END,            (GtdEaseFunc) gtd_ease_steps_end, "stepEnd" },

  { GTD_EASE_CUBIC_BEZIER,   (GtdEaseFunc) gtd_ease_cubic_bezier, "cubicBezier" },
  { GTD_EASE,                (GtdEaseFunc) gtd_ease_cubic_bezier, "ease" },
  { GTD_EASE_IN,             (GtdEaseFunc) gtd_ease_cubic_bezier, "easeIn" },
  { GTD_EASE_OUT,            (GtdEaseFunc) gtd_ease_cubic_bezier, "easeOut" },
  { GTD_EASE_IN_OUT,         (GtdEaseFunc) gtd_ease_cubic_bezier, "easeInOut" },

  { GTD_EASE_LAST,           NULL, "sentinel" },
};

GtdEaseFunc
gtd_get_easing_func_for_mode (GtdEaseMode mode)
{
  g_assert (_gtd_animation_modes[mode].mode == mode);
  g_assert (_gtd_animation_modes[mode].func != NULL);

  return _gtd_animation_modes[mode].func;
}

const char *
gtd_get_easing_name_for_mode (GtdEaseMode mode)
{
  g_assert (_gtd_animation_modes[mode].mode == mode);
  g_assert (_gtd_animation_modes[mode].func != NULL);

  return _gtd_animation_modes[mode].name;
}

gdouble
gtd_easing_for_mode (GtdEaseMode mode,
                     gdouble     t,
                     gdouble     d)
{
  g_assert (_gtd_animation_modes[mode].mode == mode);
  g_assert (_gtd_animation_modes[mode].func != NULL);

  return _gtd_animation_modes[mode].func (t, d);
}
