Mutual information
------------------

The intensity :math:`a` of a voxel located at coordinates
(:math:`\textbf{x}`) within the fixed image :math:`F` is given
by:

.. math::
	a = F(\textbf{x})


and similarly, the intensity :math:`$b$` of a voxel at coordinates
(:math:`$\textbf{y}$`) within the moving image :math:`$M$` is given
by:

.. math::
        b = M(\textbf{y})


We want to apply a coordinate transform
:math:`$\textbf{T}(\textbf{y})$` to the moving image such that it
registers best with the fixed image :math:`$F$`. However, since
:math:`$F$` and :math:`$M$` could have been obtained from different
imaging modalities, we cannot simply minimize the error between
intensities :math:`$a$` and :math:`$b$`. Instead, we must attempt
to maximize the amount of information shared between the two
images. The statistical **Mutual Information** can therefore be
used as a similarity metric:

.. math::
        I = \sum_{\textbf{a},\textbf{b}} p_j(a,\textbf{T}(b)) \ln \frac{p_j(a,\textbf{T}(b))}{p_F(a)p_M(\textbf{T}(b))}
        :label: eqnmi


which depends on the probability distributions of the voxel
intensities for the static and moving images. We now view
:math:`$a$` and :math:`$b$` as **random variables** with associated
probability distribution functions :math:`$p_F(a)$` and
:math:`$p_M(b)$` and joint probability :math:`$p_j(a,b)$`. Note
that applying the spatial transformation :math:`$\textbf{T}(y)$` to
:math:`$M$` will result in modification of :math:`$p_j(a,b)$`. This
effect is implied with the notation :math:`$p_j(a,\textbf{T}(b))$`.
Furthermore, if :math:`$\textbf{T}$` results in voxels being
displaced outside of the image, :math:`$p_M(b)$` will change.
Perhaps more importantly, if :math:`$\textbf{T}$` results in a
voxel being remapped to a location that falls between points on the
voxel grid, some form of interpolation must be employed to obtain
:math:`$b$`, which will modify :math:`$p_M(b)$`. These effects are
implied with the notation :math:`$p_M(\textbf{T}(b))$`.

The method of interpolation used to obtain :math:`$b$` given
:math:`$\textbf{T}(\textbf{y})$` is important both in terms of
execution speed and solution convergence. Here we will discuss the
**Partial Volume Interpolation** method proposed by *Maes et. al*.

.. image::
        ../figures/PV8.png
        :align: center


Figure 1(a) depicts the situation where :math:`$\boldmath{\Delta} =
\textbf{T}(\textbf{y})$` has displaced a voxel (red) to the center of a
neighborhood of 8 other voxels (black). The interpolation method divides the
volume defined by the 8-nearest neighbors (red) into 8 partial volumes (w1 -
w8) that all share the red voxel's location as a common vertex Figure 1(b-c).
We can more formally define the 8 partial volumes as a function of
:math:`$\boldmath{\Delta}$`:

.. math::
    w1 = (1 - x) (1 - y) (1 - z) \\
    w2 = ( x) (1 - y) (1 - z) \\
    w3 = (1 - x) ( y) (1 - z) \\
    w4 = ( x) ( y) (1 - z) \\
    w5 = (1 - x) (1 - y) ( z) \\
    w6 = ( x) (1 - y) ( z) \\
    w7 = (1 - x) ( y) ( z) \\
    w8 = ( x) ( y) ( z)
    :label: eqnvolumes


Noting that

.. math::
        {\sum_{i=1}^8}w_i = 1


Once the partial volumes have been computed, they are placed into
the histograms of their corresponding voxels in the 8-neighborhood.
Partial volume :math:`$w1$` is placed into the histogram bin
associated with :math:`$n1$`, :math:`$w2$` with :math:`$n2$`, etc.
This method is used in computing both :math:`$p_M(\textbf{T}(b))$`
and :math:`$p_j(a,\textbf{T}(b))$`.

We now have a complete expression for the mutual information
similarity metric, which we will use as a cost function that
describes the quality of the registration.

.. math::
        C = \sum_{\textbf{a},\textbf{b}} p_j(a,\textbf{T}(b)) \ln \frac{p_j(a,\textbf{T}(b))}{p_F(a)p_M(\textbf{T}(b))}
        :label: eqncost


We can attempt to achieve the optimal registration by maximizing this cost
function, which is achieved through modification of the coordinate
transformation :math:`$\textbf{T}(\textbf{y})$` applied to the moving image
:math:`$M$`. It is important to note that the :math:`$\textbf{T}$` could be any
type of conceivable coordinate transformation. For example,
:math:`$\textbf{T}$` could represent a set of affine transforms to be applied
to the moving image. Here, however, we will be focusing on a parametrically
defined nonrigid coordinate transformation. The transformation
:math:`$\textbf{T}(\textbf{y})$` is characterized by the displacement field
:math:`$\nu$`, which defines how individual voxels within the moving image
:math:`$M$` should be displaced such that :math:`$S$` and :math:`$M$` are
optimally registered. This displacement field is *computed* at every voxel
given the B-spline coefficients :math:`$P$` defined for the sparse array of
control-points:

.. math::
        \nu_x(\textbf{y}) = \sum_{l=0}^{3} \sum_{m=0}^{3} \sum_{n=0}^{3} \beta_l(v_x) \beta_m(v_y) \beta_n(v_z) P_x(j_x+l, j_y+m, j_z+n)
        :label: eqnvector


where (:math:`$l, m, n$`) form the indices of the 64 control points
in the immediate vicinity of a voxel located at
(:math:`$\textbf{y}$`). Note, (:math:`$\textbf{j}$`) forms the
voxel's offsets within the region bound by the 8 control-points in
the immediate vicinity of the voxel. For simplicity, we refer to
such a region as a *tile*.

Having defined the coordinate transformation
:math:`$\textbf{T}(\textbf{y})$` in terms of the B-spline
coefficients :math:`$P$`, it is becomes apparent that the mutual
information may be maximized through optimization of :math:`$P$`.
If this is to be accomplished via the method of gradient descent,
an analytic expression for the gradient
:math:`$\partial C / \partial P$` must be derived. The chain rule
is employed to separate the expression into two partial derivatives
with the first being dependent on the similarity metric employed
and the second on the type of transform employed:

.. math::
        \frac{\partial C}{\partial P} = \frac{\partial C}{\partial \nu} \times \frac{\partial \nu}{\partial P}
        :label: eqnchainrule


where the second term is easily obtained by taking the derivative
of :eq:`eqnvector` with respect to :math:`$P$`:

.. math::
        \frac{\partial \nu}{\partial P} = \sum_{l=0}^{3} \sum_{m=0}^{3} \sum_{n=0}^{3} \beta_l(u) \beta_m(v) \beta_n(w).


Referring again to the first term of :eq:`eqnchainrule`, it is
important to realize that :math:`$C$` and :math:`$v$` are coupled
through the probability distribution :math:`$p_j$` and are
therefore directly affected by the 8 neighborhood partial volume
interpolation. This becomes further apparent when
:math:`$\partial C / \partial v$` is further decomposed:

.. math::
    \frac{\partial C}{\partial \nu} =
    \frac{\partial C}{\partial p_j(a,M(n))} \frac{\partial p_j(a,M(n))}{\partial \nu} \\

.. math::
    \frac{\partial C}{\partial \nu} =
    \sum_{n=1}^{8} \frac{\partial C}{\partial p_j(a,M(n))} \frac{\partial w_n}{\partial \nu}
    :label: eqnhistcycle


where the first term may be found by taking the derivative of
:eq:`eqncost` with respect to the joint distribution :math:`$p_j$`:

.. math::
        \frac{\partial C}{\partial p_j(a,M(n))} = \ln \frac{p_j(a,M(n))}{p_F(a)p_M(M(n))} - C
        :label: eqninfochange

see {A}.

The second term, :math:`$\partial p_j(a,M(n)) / \partial \nu$`,
describes how the joint distribution changes with the vector field.
Explicitly stated, a displacement vector locally transforms the
coordinates of a voxel in the moving image :math:`$M$` such that:

.. math::
        \boldmath{\Delta} = \boldmath{x} + \boldmath{\nu}


From Figure fig:pv8 it becomes immediately apparent that as the vector field is
modified the partial volumes :math:`$w1$` - :math:`$w8$` to be inserted into
the moving and joint histograms :math:`$p_M$` and :math:`$p_j$` will change in
size. Consequently, :math:`$\partial p_{j_n} / \partial \boldmath{\nu}$` can be
found for :math:`$n \in{1,8}$` by taking the derivative of :eq:`eqnvolumes`
with respect to each of the Cartesian directions.

Therefore, as prescribed by :eq:`eqnhistcycle`, computing :math:`$\partial C /
\partial v$` at a given voxel in :math:`$M$` involves first cycling through the
8-bins corresponding to the 8 nearest neighbors used during the partial volume
interpolation and computing :math:`$\partial C / \partial p_j$` for each, which
is scalar and therefore the same for all Cartesian directions. The 8
:math:`$\partial C / \partial p_j$` values are each weighted by one
:math:`$\partial p_j / \partial \nu$`. Finally, once this process has been
completed for all 8 bins, the resulting :math:`$\partial C / \partial v$`
values is assimilated into the sparse coefficient array :math:`$P$` by way of
:eq:`eqnchainrule`.  Once this is completed for all voxels, the next optimizer
iteration begins and the process is repeated with a modified coefficient array
:math:`$P$`.

**NOTE**

It is important to note that if all 8 histograms associated with
the neighborhood are equal, the value of
:math:`$\partial C / \partial \nu = 0$`. This is a direct result
of

.. math::
        {\sum_{n=1}^8} \frac{\partial w_n}{\partial v_x} = 0


and similarly for the :math:`$y$`- and :math:`$z$`-directions.  However, when
the 8 histograms within the neighborhood are not equal, this will result in the
gradient :math:`$\partial C / \partial \nu$` being "pulled" in the Cartesian
direction of voxels associated with large :math:`$\partial C / \partial p_j$`,
which is desirable when attempting to maximize MI.
