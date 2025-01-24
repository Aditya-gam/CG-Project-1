#include <limits>
#include "box.h"

/**
 * Intersection
 * ------------
 * Returns whether a given ray intersects this axis-aligned bounding box, and if so,
 * provides the parametric distance t along the ray where the first intersection occurs.
 *
 * \param ray The ray to test against this box.
 * \return A pair (bool, double). The bool indicates if the intersection exists.
 *         If bool is true, the double is the parametric distance t for the ray at
 *         the closest intersection in front of the ray origin.
 */
std::pair<bool,double> Box::Intersection(const Ray& ray) const
{
    // We use a parametric approach. We keep track of an interval [tmin, tmax]
    // along the ray such that if this interval remains valid (tmax >= tmin),
    // the ray intersects the box.

    double tmin = -std::numeric_limits<double>::infinity();
    double tmax =  std::numeric_limits<double>::infinity();

    // For each coordinate axis:
    for(int i = 0; i < 3; i++)
    {
        // If the ray is (almost) parallel to this axis...
        if(std::abs(ray.direction[i]) < 1e-16)
        {
            // ...check if the ray's origin is inside the slab for this axis.
            // If the origin is outside, there's no intersection.
            if(ray.endpoint[i] < lo[i] || ray.endpoint[i] > hi[i])
            {
                return {false, -1};
            }
        }
        else
        {
            // Compute the t-values where the ray intersects the two planes
            // bounding the box on this axis.
            double inv_dir = 1.0 / ray.direction[i];
            double t1      = (lo[i] - ray.endpoint[i]) * inv_dir;
            double t2      = (hi[i] - ray.endpoint[i]) * inv_dir;

            // Enforce t1 <= t2 by swapping if needed
            if(t1 > t2) std::swap(t1, t2);

            // Narrow the intersection interval
            if(t1 > tmin) tmin = t1;
            if(t2 < tmax) tmax = t2;

            // If the interval is invalid, exit early
            if(tmax < tmin)
            {
                return {false, -1};
            }
        }
    }

    // If tmax < 0, the intersection is entirely behind the ray start
    if(tmax < 0)
    {
        return {false, -1};
    }

    // If tmin < 0, then the ray starts inside the box, so the first intersection
    // "in front" is at tmax. If tmax >= 0, that's valid.
    double t_hit = (tmin < 0) ? tmax : tmin;

    return {true, t_hit};
}

/**
 * Union
 * -----
 * Computes the smallest box that contains both 'this' box and another box 'bb'.
 *
 * \param bb Another box to unite with the current box.
 * \return A new box that encloses both.
 */
Box Box::Union(const Box& bb) const
{
    Box box;
    box.lo = componentwise_min(lo, bb.lo);
    box.hi = componentwise_max(hi, bb.hi);
    return box;
}

/**
 * Intersection
 * ------------
 * Computes the smallest box that is contained by both 'this' box and another box 'bb'.
 * If the two boxes do not overlap, this function returns an empty box.
 *
 * \param bb Another box to intersect with the current box.
 * \return A new box that represents the intersection. If there is no overlap,
 *         the returned box will be "empty" (lo > hi in at least one dimension).
 */
Box Box::Intersection(const Box& bb) const
{
    Box box;
    // The intersection in each axis is the overlap, which is from the max of the lower bounds
    // to the min of the upper bounds.
    box.lo = componentwise_max(lo, bb.lo);
    box.hi = componentwise_min(hi, bb.hi);

    // If there's no overlap in any dimension, we make the box empty.
    for(int i = 0; i < 3; i++)
    {
        if(box.lo[i] > box.hi[i])
        {
            box.Make_Empty();
            break;
        }
    }

    return box;
}

/**
 * Include_Point
 * -------------
 * Enlarges this box if necessary so that the given point 'pt' lies inside it.
 *
 * \param pt The point to include in the bounding box.
 */
void Box::Include_Point(const vec3& pt)
{
    for(int i = 0; i < 3; i++)
    {
        if(pt[i] < lo[i]) lo[i] = pt[i];
        if(pt[i] > hi[i]) hi[i] = pt[i];
    }
}

/**
 * Is_Full
 * -------
 * Checks if this box represents the "full" infinite box, i.e., lo = (-∞, -∞, -∞)
 * and hi = (+∞, +∞, +∞).
 *
 * \return true if the box is infinite in all directions; false otherwise.
 */
bool Box::Is_Full() const
{
    for(int i = 0; i < 3; i++)
    {
        if(lo[i] != -std::numeric_limits<double>::infinity() ||
           hi[i] !=  std::numeric_limits<double>::infinity())
        {
            return false;
        }
    }
    return true;
}

// Create a box to which points can be correctly added using Include_Point.
void Box::Make_Empty()
{
    lo.fill(std::numeric_limits<double>::infinity());
    hi=-lo;
}

// Create a box that contains everything.
void Box::Make_Full()
{
    hi.fill(std::numeric_limits<double>::infinity());
    lo=-hi;
}

bool Box::Test_Inside(const vec3& pt) const
{
    for(int i=0;i<3;i++)
        if(pt[i]<lo[i] || pt[i]>hi[i])
            return false;
    return true;
}

// Useful for debugging
std::ostream& operator<<(std::ostream& o, const Box& b)
{
    return o << "(lo: " << b.lo << "; hi: " << b.hi << ")";
}
