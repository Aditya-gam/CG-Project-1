#include "acceleration.h"
#include "object.h"
#include "hit.h"

// This external variable controls the resolution of the uniform grid.
// By default, it might be set to (40,40,40).
extern int acceleration_grid_size;

// Constructor for the Acceleration class.
Acceleration::Acceleration()
{
    // Start with an empty box so we can grow it as we discover finite objects
    domain.Make_Empty();

    // All three dimensions initially set to the same number of cells
    num_cells.fill(acceleration_grid_size);
}

void Acceleration::Add_Object(const Object* obj, int id)
{
    // For each "part" of the object. For simple objects, num_parts=1.
    // For meshes, num_parts might be the number of triangles, etc.
    for(int part_index = 0; part_index < obj->num_parts; ++part_index)
    {
        // Query the bounding box for this (object, part_index).
        // The returned pair is (Box bounding_box, bool is_infinite).
        auto [b, is_infinite] = obj->Bounding_Box(part_index);

        // Create a primitive record.
        Primitive prim;
        prim.obj = obj;
        prim.part = part_index;
        prim.id   = id;

        if(is_infinite)
        {
            // If the object/part is infinite, store it separately.
            infinite_objects.push_back(prim);
        }
        else
        {
            // Otherwise, store it in the finite list and
            // enlarge 'domain' to include this bounding box.
            finite_objects.push_back(prim);
            domain.Include_Point(b.lo);
            domain.Include_Point(b.hi);
        }
    }
}

ivec3 Acceleration::Cell_Index(const vec3& pt) const
{
    ivec3 idx;
    for(int axis = 0; axis < 3; ++axis)
    {
        // Relative position along this axis within the domain
        double rel = (pt[axis] - domain.lo[axis]) / dx[axis];

        // Floor to find the cell index
        int cell = static_cast<int>(std::floor(rel));

        // Clamp index to [0, num_cells[axis]-1]
        if(cell < 0) cell = 0;
        if(cell >= num_cells[axis]) cell = num_cells[axis] - 1;

        idx[axis] = cell;
    }
    return idx;
}

void Acceleration::Initialize()
{
    // If no finite objects, there's nothing to populate in the grid.
    // We'll rely on infinite_objects alone.
    if(finite_objects.empty())
    {
        // No finite objects => domain is irrelevant => skip building the grid
        cells.clear(); // ensure cells is empty
        return;
    }

    // Otherwise proceed as normal
    vec3 range = domain.hi - domain.lo;
    for(int i=0; i<3; i++)
    {
        if(num_cells[i] > 0) dx[i] = range[i]/(double)num_cells[i];
        else dx[i] = 0.0;
    }

    cells.resize(num_cells[0]*num_cells[1]*num_cells[2]);

    // Fill the cells with the finite objects
    for(const auto& prim : finite_objects)
    {
        auto [b, _] = prim.obj->Bounding_Box(prim.part);
        ivec3 min_index = Cell_Index(b.lo);
        ivec3 max_index = Cell_Index(b.hi);
        for(int i = min_index[0]; i <= max_index[0]; ++i)
        for(int j = min_index[1]; j <= max_index[1]; ++j)
        for(int k = min_index[2]; k <= max_index[2]; ++k)
        {
            cells[Flat_Index({i,j,k})].push_back(prim);
        }
    }
    finite_objects.clear();
}


std::pair<int,Hit> Acceleration::Closest_Intersection(const Ray& ray) const
{
    // Track the best (closest) intersection found so far.
    // best_id = -1 means "no hit yet".
    int best_id = -1;
    Hit best_hit;
    best_hit.dist = -1;

    for(const auto& prim : infinite_objects)
    {
        // Intersect the ray with this infinite object-part
        Hit h = prim.obj->Intersection(ray, prim.part);
        if(h.Valid())
        {
            // If this is the first valid hit or is closer than the previous best
            if(!best_hit.Valid() || h.dist < best_hit.dist)
            {
                best_hit = h;
                best_id  = prim.id;
            }
        }
    }

    // If grid is empty, no finite objects => skip the domain intersection
    if(cells.empty()) 
    {
        return {best_id, best_hit};
    }

    // We'll manually compute the intersection interval [tmin, tmax].
    double tmin = 0.0;
    double tmax = std::numeric_limits<double>::infinity();

    for(int axis = 0; axis < 3; ++axis)
    {
        if(std::abs(ray.direction[axis]) < 1e-16)
        {
            // If the ray is parallel to this axis, check if it's outside the domain
            if(ray.endpoint[axis] < domain.lo[axis] || ray.endpoint[axis] > domain.hi[axis])
            {
                // No intersection with the domain
                return {best_id, best_hit};
            }
        }
        else
        {
            // Compute intersection t-values with the slab [lo, hi] for this axis
            double inv_dir = 1.0 / ray.direction[axis];
            double t1 = (domain.lo[axis] - ray.endpoint[axis]) * inv_dir;
            double t2 = (domain.hi[axis] - ray.endpoint[axis]) * inv_dir;

            if(t1 > t2) std::swap(t1, t2);

            // Update overall tmin/tmax intersection interval
            if(t1 > tmin) tmin = t1;
            if(t2 < tmax) tmax = t2;

            // If interval is invalid, no intersection
            if(tmax < tmin) 
                return {best_id, best_hit};
        }
    }
    
    // Make sure we start at least at small_t to avoid floating precision issues
    if(tmin < small_t) tmin = small_t;
    if(tmax < tmin) // might happen if domain intersection starts beyond small_t
        return {best_id, best_hit};

    // The point of first contact with the domain
    double t_cur = tmin;
    vec3 p_cur = ray.endpoint + ray.direction * t_cur;
    ivec3 cell = Cell_Index(p_cur);

    // Precompute step direction and the tDelta for each axis (distance in t to cross one cell).
    // If the direction is negative, we step "backwards".
    ivec3 step;
    vec3 t_delta;
    vec3 next_boundary; // store the t-value at which we cross the next boundary in each axis

    for(int axis=0; axis<3; ++axis)
    {
        if(ray.direction[axis] > 0)
        {
            step[axis] = 1;
        }
        else
        {
            step[axis] = -1;
        }

        // Avoid division by zero. If dx[axis] is 0, there's effectively no domain in that axis.
        if(std::abs(ray.direction[axis]) < 1e-16)
        {
            t_delta[axis] = std::numeric_limits<double>::infinity();
            next_boundary[axis] = std::numeric_limits<double>::infinity();
        }
        else
        {
            t_delta[axis] = std::abs(dx[axis] / ray.direction[axis]);

            // Determine boundary coordinate (in index space) that we cross next.
            double boundaryCoord = (step[axis] > 0) 
                                   ? (domain.lo[axis] + (cell[axis] + 1) * dx[axis])
                                   : (domain.lo[axis] +  cell[axis]      * dx[axis]);

            // If stepping negative, the "boundary" is the lower edge of the cell
            // if cell[axis] is exactly on some boundary. We can unify it by an
            // offset if stepping < 0. But let's do a direct formula:
            if(step[axis] < 0)
            {
                boundaryCoord = domain.lo[axis] + cell[axis]*dx[axis];
            }

            double distToBoundary = boundaryCoord - ray.endpoint[axis];
            next_boundary[axis] = (std::abs(ray.direction[axis]) > 1e-16) 
                                  ? distToBoundary / ray.direction[axis]
                                  : std::numeric_limits<double>::infinity();
        }
    }

    // Because we started at tmin, we might need to move next_boundary[axis] forward
    // to match tmin if tmin is already beyond that boundary calculation.
    // Let's shift next_boundary so that it represents the next crossing strictly after t_cur.
    for(int axis=0; axis<3; ++axis)
    {
        while(next_boundary[axis] < t_cur && next_boundary[axis] < std::numeric_limits<double>::infinity())
            next_boundary[axis] += t_delta[axis];
    }

    // We'll keep stepping until t_cur > tmax or we've found a hit that is closer
    // than the next boundary traversal. This is the standard 3D DDA algorithm.
    while(t_cur <= tmax)
    {
        // Check all primitives in this cell
        const auto& primitives_in_cell = Cell_Data(cell);
        for(const auto& prim : primitives_in_cell)
        {
            // If we already have a best_hit, we only need to check if
            // the new intersection could be smaller than best_hit.dist.
            // But let's just do the intersection and compare.

            Hit h = prim.obj->Intersection(ray, prim.part);
            if(h.Valid())
            {
                // If it's new or closer, update best_hit.
                if(!best_hit.Valid() || h.dist < best_hit.dist)
                {
                    // Only record it if it's within [small_t, t_cur -> tmax].
                    // The typical approach is [small_t, +inf], but let's be sure
                    // we don't record an intersection behind our current traversal.
                    if(h.dist >= small_t && h.dist < (best_hit.Valid() ? best_hit.dist : std::numeric_limits<double>::infinity()))
                    {
                        best_hit = h;
                        best_id  = prim.id;
                    }
                }
            }
        }

        // If we found a valid hit that is closer than the next boundary crossing
        // or less than t_cur, we can exit early to speed up.
        // This is an optimization, not mandatory for correctness.
        if(best_hit.Valid() && best_hit.dist <= t_cur)
        {
            // We can't get a better intersection after we've passed it in the grid.
            break;
        }

        // Determine which axis' boundary is the next to be crossed in t.
        double t_next_x = next_boundary[0];
        double t_next_y = next_boundary[1];
        double t_next_z = next_boundary[2];

        double t_next = std::min(t_next_x, std::min(t_next_y, t_next_z));

        // If next crossing is beyond tmax, we are done.
        if(t_next > tmax) break;

        // Advance to that crossing
        t_cur = t_next;

        // Identify which axis was stepped (the one with the smallest t_next).
        // Step that axis in the cell grid and update next_boundary[axis].
        for(int axis=0; axis<3; ++axis)
        {
            if(t_next == next_boundary[axis])
            {
                // Move the boundary forward one cell in that axis
                next_boundary[axis] += t_delta[axis];
                // Step the cell index in that axis
                cell[axis] += step[axis];

                // If we are out of the domain, we can stop
                if(cell[axis] < 0 || cell[axis] >= num_cells[axis])
                {
                    goto FINISH; // break out of the while loop
                }
            }
        }
    }

FINISH: // label to jump out of the while loop

    return {best_id, best_hit};
}
