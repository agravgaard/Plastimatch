B-splines
=========
This document is incomplete.  Don't add it to the index until it is complete.

The B-spline is parametric representation of a function, based 
on piecewise smooth polynomial curves. 
The "B" in bspline is short for "basis-spline".
Plastimatch uses uniform cubic B-splines, so this document will 
only describe those.

In one dimension, a B-spline is given by

.. math::
   u(x) = \sum_i p_i \beta_i (x).

where (:math:`\beta_i (x)`) is a piecewise cubic polynomial.  There 
are four pieces in the polynomial.

References
----------
#. http://en.wikipedia.org/wiki/B-spline
#. http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/surface/bspline-construct.html
#. http://graphics.idav.ucdavis.edu/education/CAGDNotes/Quadratic-B-Spline-Surface-Refinement/Quadratic-B-Spline-Surface-Refinement.html
#. http://chapter.aapm.org/NE/meet021011/talks/sharp-neaapm-2011-02-10.pdf
