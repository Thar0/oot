#!/usr/bin/env python3
#
#   Convert vanilla bgcheck structures to new BVH Tree structures
#   Currently not used since new structures are not implemented (yet?)
#

import math, multiprocessing, os, sys, time
from typing import List, Set

def lex(lines, spec):
    spl = [x for x in "\n".join(lines).split() if x.strip() != ""]
    assert len(spl) == 23, len(spl)

    res = []
    for s,e in zip(spl, spec):
        if e is None:
            res.append(s)
        else:
            assert s == e, (s,e)

    return res

def stripstatic(s : str):
    return s.replace("static " if s.startswith("static ") else "", "")

def collect_decl_forname(contents, name, type):
    if name == "NULL":
        return None

    decl = f"{type} {name}"

    lines = []

    indecl = False
    for line in contents.split("\n"):
        if stripstatic(line).startswith(decl) and not line.endswith(";"):
            indecl = True

        if indecl:
            lines.append(stripstatic(line))

            if line.strip() == "};":
                break

    assert indecl, (name, type, contents)

    return lines

def collect_all_decls_of_type(contents, type):
    decls = {}

    name = None
    for line in contents.split("\n"):
        if stripstatic(line).startswith(f"{type} "):
            name = line.split(" ")[1]
            decls[name] = []

        if name is not None:
            decls[name].append(stripstatic(line))

            if line.strip() == "};":
                name = None

    return decls

class Vec3f:
    def __init__(self, x, y, z) -> None:
        self.x = x
        self.y = y
        self.z = z

    def __add__(self, other):
        return Vec3f(self.x + other.x, self.y + other.y, self.z + other.z)

    def __sub__(self, other):
        return Vec3f(self.x - other.x, self.y - other.y, self.z - other.z)

    def __getitem__(self, i):
        if i not in (0, 1, 2):
            raise ValueError(f"Bad index to Vec3f: {i}")
        return (self.x, self.y, self.z)[i]

    def inner(self, other):
        return self.x * other.x + self.y * other.y + self.z * other.z

    def magnitude_sq(self):
        return self.inner(self)

    def magnitude(self):
        return math.sqrt(self.magnitude_sq())

    def __str__(self) -> str:
        return f"{{ {self.x}, {self.y}, {self.z} }}"

def VEC3S_C(v : Vec3f) -> str:
    return f"{{ {int(v.x):6}, {int(v.y):6}, {int(v.z):6} }}"

def vmin(vs : List[Vec3f]) -> Vec3f:
    return Vec3f(min(v.x for v in vs), min(v.y for v in vs), min(v.z for v in vs))

def vmax(vs : List[Vec3f]) -> Vec3f:
    return Vec3f(max(v.x for v in vs), max(v.y for v in vs), max(v.z for v in vs))

class AABB:

    def __init__(self, min : Vec3f, max : Vec3f, ob = None):
        self.min : Vec3f = min
        self.max : Vec3f = max
        self.ob = ob

    def __str__(self) -> str:
        return f"AABB({self.min}, {self.max})"

    @property
    def extents(self) -> Vec3f:
        return self.max - self.min

    @property
    def surface_area(self) -> float:
        dim = self.extents
        return 2.0 * (dim.x * dim.y + dim.y * dim.z + dim.z * dim.x)

    @property
    def volume(self) -> float:
        ext = self.extents
        return ext.x * ext.y * ext.z

    @property
    def center(self) -> Vec3f:
        return (self.min + self.max) / 2.0

    def center_onaxis(self, axis : int) -> Vec3f:
        return (self.min[axis] + self.max[axis]) / 2.0

    @property
    def axis_of_greatest_extent(self) -> int:
        ext = self.extents()

        res = 0
        if ext[1] > ext[res]:
            res = 1
        if ext[2] > ext[res]:
            res = 2
        return res

    def infront(self, pos : Vec3f, axis : int) -> bool:
        return self.min[axis] >= pos and self.max[axis] >= pos

    def behind(self, pos : Vec3f, axis : int) -> bool:
        return self.min[axis] < pos and self.max[axis] < pos

    def straddles(self, pos : Vec3f, axis : int) -> bool:
        return (self.min[axis] >= pos and self.max[axis] < pos) or (self.min[axis] < pos and self.max[axis] >= pos)

    def union(self, other):
        if isinstance(other, AABB):
            # aabb U aabb
            return AABB(vmin((self.min, other.min)), vmax((self.max, other.max)))
        elif isinstance(other, Vec3f):
            # aabb U point
            return AABB(vmin((self.min, other)), vmax((self.max, other)))
        else:
            assert False , type(other)

    @staticmethod
    def Union(bblist):
        return AABB(Vec3f(
            min(bblist, key = lambda bb : bb.min.x).min.x,
            min(bblist, key = lambda bb : bb.min.y).min.y,
            min(bblist, key = lambda bb : bb.min.z).min.z
        ), Vec3f(
            max(bblist, key = lambda bb : bb.max.x).max.x,
            max(bblist, key = lambda bb : bb.max.y).max.y,
            max(bblist, key = lambda bb : bb.max.z).max.z
        ))

    def intersection(self, other):
        # TODO check
        if isinstance(other, AABB):
            new_min = [None, None, None]
            new_max = [None, None, None]

            for axis in range(3):
                points = (
                    (0, self.min[axis]),
                    (0, self.max[axis]),
                    (1, other.min[axis]),
                    (1, other.max[axis]),
                )
                points = list(sorted(points, key=lambda p : p[1]))

                prim0, _ = points[0]
                prim1, p1 = points[1]
                prim2, p2 = points[2]
                prim3, _ = points[3]

                if prim0 == prim1:
                    continue # no overlap

                new_min[axis] = p1
                new_max[axis] = p2

            if any(p is None for p in new_min) or any(p is None for p in new_max):
                return None

            return AABB(Vec3f(*new_min), Vec3f(*new_max))
        elif isinstance(other, Vec3f):
            # if other is contained in self, that is the intersection
            # otherwise the intersection is the empty set
            if other.x in range(self.min.x, self.max.x) and \
               other.y in range(self.min.y, self.max.y) and \
               other.z in range(self.min.z, self.max.z):
                return other
            return None
        else:
            assert False , type(other)

    def to_c(self, indent=0) -> str:
        out  = f"{' ' * indent}{{\n"
        out += f"{' ' * indent + 4}{VEC3S_C(self.min)},\n"
        out += f"{' ' * indent + 4}{VEC3S_C(self.max)},\n"
        out += f"{' ' * indent}}},\n"
        return out

    @staticmethod
    def from_tri_vertices(vertices):
        min_coords = Vec3f(float('inf'), float('inf'), float('inf'))
        max_coords = Vec3f(float('-inf'), float('-inf'), float('-inf'))

        for vtxp in vertices:
            min_coords.x = min(min_coords.x, vtxp.x)
            min_coords.y = min(min_coords.y, vtxp.y)
            min_coords.z = min(min_coords.z, vtxp.z)
            max_coords.x = max(max_coords.x, vtxp.x)
            max_coords.y = max(max_coords.y, vtxp.y)
            max_coords.z = max(max_coords.z, vtxp.z)

        return AABB(min_coords, max_coords)

class BVHLeaf:
    def __init__(self, aabbs : List[AABB]):
        self.aabbs : List[AABB] = aabbs

    def print(self, ofile=sys.stdout, indent=0):
        print(f"{' ' * indent}leaf: {len(self.aabbs)}", file=ofile)
        # print(f"{' ' * indent}  {AABB.Union(self.aabbs)}", file=ofile)

    def to_c(self) -> str:
        pass

class BVHNode:
    def __init__(self, aabb : AABB, left, right):
        self.aabb : AABB = aabb
        self.left = left
        self.right = right

    def print(self, ofile=sys.stdout, indent=0):
        # print(f"{' ' * indent}{self.aabb}")
        print("BRANCH", file=ofile)
        print(f"{' ' * indent} left:", file=ofile)
        self.left.print(ofile, indent + 4)
        print(f"{' ' * indent}right:", file=ofile)
        self.right.print(ofile, indent + 4)

    def to_c(self) -> str:
        pass


# NOTE: these two costs are relative proportions
# A higher traversal cost tends to reduce memory footprint as it reduces max tree size by coalescing more
# tris into single leaves.
TRAVERSAL_COST = 0.25
INTERSECT_COST = 1.0

def SAH(aabbs : Set[AABB], bounding_aabb : AABB, axis : int, pos : Vec3f):
    # sort AABBs into left/right buckets
    left_aabbs = set(aabb for aabb in aabbs if aabb.center_onaxis(axis) < pos)
    right_aabbs = set(aabb for aabb in aabbs if aabb not in left_aabbs)

    # compute cost of left bucket
    left_num = len(left_aabbs)
    left_cost = 0
    if left_num != 0:
        left_cost = left_num * AABB.Union(left_aabbs).surface_area

    # compute cost of right bucket
    right_num = len(right_aabbs)
    right_cost = 0
    if right_num != 0:
        right_cost = right_num * AABB.Union(right_aabbs).surface_area

    # compute total cost associated with this split
    cost = TRAVERSAL_COST * bounding_aabb.surface_area + INTERSECT_COST * (left_cost + right_cost)
    # return
    return cost if cost > 0 else 1.0e30, left_aabbs, right_aabbs

def bvh_mp_runner(args):
    aabb, aabbs, bounding_aabb, axis = args
    return SAH(aabbs, bounding_aabb, axis, aabb.center_onaxis(axis))

def build_bvh(aabbs, bounding_aabb, pool : multiprocessing.Pool):
    # TODO SBVH?
    # Any aabb that straddles the split plane is duplicated into both left and right, then compare SAH cost for:
    # - aabb in left only
    # - aabb in right only
    # - aabb split into both
    # Lowest evaluated cost decides where it will end up.
    # If the ratio of bounding box overlap surface area and root node surface area is too great, skip splitting.
    # If we go with splitting we have to add another layer of indirection:
    # - The tree
    # - indexes an array of poly ids
    # - indexes the poly array
    # (https://www.nvidia.in/docs/IO/77714/sbvh.pdf)

    # print(f"BUILD BVH {len(aabbs)}")

    # select the split plane and split aabbs into two groups
    # assign the initial best to be the unsplit cost, so that we only split if it is better than not splitting at all
    best_cost = TRAVERSAL_COST * 0 + INTERSECT_COST * len(aabbs) * bounding_aabb.surface_area
    # TODO check termination works properly, one of these being empty should lead to a leaf node below
    best_left = aabbs
    best_right = set()
    leaf = True

    if len(aabbs) > 512:
        # compute costs (multiprocessed)
        costs_lr = pool.imap_unordered(bvh_mp_runner, ((aabb, aabbs, bounding_aabb, axis) for axis in range(3) for aabb in aabbs), 16)

        # select min cost from computed costs
        found_cost, found_left, found_right = min(costs_lr, key=lambda p : p[0])

        if found_cost < best_cost:
            best_cost, best_left, best_right = found_cost, found_left, found_right
            leaf = False
    else:
        # compute costs (single-threaded)
        # TODO binning instead of full-SAH for faster construction? or just multithread it?
        for aabb in aabbs:
            aabb : AABB

            for axis in range(3):
                # get cost for a split
                cost, left, right = SAH(aabbs, bounding_aabb, axis, aabb.center_onaxis(axis))

                # if this cost is best, record result
                if cost < best_cost:
                    # print("Win:", cost, best_cost)
                    best_cost, best_left, best_right = cost, left, right
                    leaf = False

    # print(f"leaf? {leaf}")

    # best split result
    left_aabbs, right_aabbs = best_left, best_right

    # if either left or right are empty, emit a node containing all aabbs
    if len(left_aabbs) == 0:
        return BVHLeaf(right_aabbs)
    if len(right_aabbs) == 0:
        return BVHLeaf(left_aabbs)

    left_full = AABB.Union(left_aabbs)
    right_full = AABB.Union(right_aabbs)

    # DEBUG: compute intersection of left and right aabbs
    # isect = left_full.intersection(right_full)
    # if isect is not None:
    #     print(f"Intersect: ext {isect.extents} vol {isect.volume}")
    #     isect.draw(isect_mat)
    # else:
    #     print("No intersect")

    # otherwise build child nodes for both sides, current node becomes a branch
    return BVHNode(bounding_aabb, build_bvh(left_aabbs, left_full, pool), build_bvh(right_aabbs, right_full, pool))

def recalc_normal(vertices):
    (x1, y1, z1) = vertices[0]
    (x2, y2, z2) = vertices[1]
    (x3, y3, z3) = vertices[2]

    nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2)
    ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2)
    nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2)

    mag_sq = nx * nx + ny * ny + nz * nz
    if mag_sq < 0.0008:
        return None, True

    mag = math.sqrt(mag_sq)
    nx /= mag
    ny /= mag
    nz /= mag

    return Vec3f(nx, ny, nz), False

class Vec3s:
    def __init__(self, x, y, z) -> None:
        self.x = int(x)
        self.y = int(y)
        self.z = int(z)

    def __iter__(self):
        yield self.x
        yield self.y
        yield self.z

    def __str__(self) -> str:
        return f"{{ {self.x}, {self.y}, {self.z} }}"

class CollisionPoly:
    def __init__(self, type, v1, v2, v3, nx, ny, nz, dist) -> None:
        v1 = int(v1, 16)
        v2 = int(v2, 16)
        v3 = int(v3, 16)
        self.type = int(type, 16)
        self.flags_1 = (v1 & 0xE000) >> 13
        self.flags_2 = (v2 & 0xE000) >> 13
        assert (v3 & 0xE000) >> 13 == 0
        self.v1 = v1 & 0x1FFF
        self.v2 = v2 & 0x1FFF
        self.v3 = v3 & 0x1FFF
        self.normal = Vec3s(int(nx, 16) / 32767.0, int(ny, 16) / 32767.0, int(nz, 16) / 32767.0)
        self.plane_dist = int(dist, 16)

class SurfaceType:
    def __init__(self, w0, w1) -> None:
        self.w0 = int(w0, 16)
        self.w1 = int(w1, 16)

    def __iter__(self):
        yield self.w0
        yield self.w1

class BgCamInfo:
    def __init__(self, setting, count, func) -> None:
        self.setting = int(setting, 16)
        self.count = int(count, 16)
        self.func = func # string

    def __str__(self) -> str:
        return f"{{ {self.setting}, {self.count}, {self.func} }}"

class WaterBox:
    def __init__(self, xmin, ysurface, zmin, xlen, zlen, properties) -> None:
        self.xmin = int(xmin)
        self.ysurface = int(ysurface)
        self.zmin = int(zmin)
        self.xlen = int(xlen)
        self.zlen = int(zlen)
        self.properties = int(properties, 16)

    def __str__(self) -> str:
        return f"{{ {self.xmin}, {self.ysurface}, {self.zmin}, {self.xlen}, {self.zlen}, 0x{self.properties:08X} }}"

class Collision:
    def __init__(self, names, min : Vec3s, max : Vec3s, vertices : List[Vec3s], collision_polys : List[CollisionPoly],
                 surface_types : List[SurfaceType], cam_data : List[BgCamInfo], waterboxes : List[WaterBox]) -> None:
        self.name, self.vertices_name, self.polygons_name, self.surfacetypes_name, self.camdata_name, self.waterboxes_name = names
        self.min = min
        self.max = max
        self.vertices = vertices
        self.collision_polys = collision_polys
        self.surface_types = surface_types
        self.cam_data = cam_data
        self.waterboxes = waterboxes

    def convert_collision(self, static, logfile):
        output = ""

        print("COLLISION EXPORT", file=logfile)
        print(f"num vertices: {len(self.vertices)}", file=logfile)
        print(f"num polys: {len(self.collision_polys)}", file=logfile)

        collision_header_name = self.name
        vertex_array_name = f"{self.name}_ColVertices"
        colpoly_array_name = f"{self.name}_ColPolys"
        surface_types_array_name = f"{self.name}_ColSurfaces"
        cam_data_array_name = f"{self.name}_ColCamData"
        waterboxes_array_name = f"{self.name}_ColWaterBoxes"
        bvh_tree_name = f"{self.name}_ColTree"

        # Vertex array
        num_vertices = len(self.vertices)

        static = "static " if static else ""

        vertex_array_c = f"{static}Vec3s {vertex_array_name}[{num_vertices}] = {{\n"
        for vertex in self.vertices:
            vertex_array_c += f"    {VEC3S_C(vertex)},\n"
        vertex_array_c += "};\n"

        output += vertex_array_c + "\n"

        # Make bounding boxes and CollisionPolys for every tri

        tri_bboxes = set()
        for poly in self.collision_polys:
            vtx_inds = (poly.v1, poly.v2, poly.v3)

            assert all(vi < 0x2000 for vi in vtx_inds)

            # sort min y
            sorted_indices = list(sorted(vtx_inds, key = lambda vi : self.vertices[vi].y))
            tri_vertices = [self.vertices[vi] for vi in sorted_indices]

            normal,degenerate = recalc_normal(tri_vertices)
            if degenerate:
                # skip 0-area tris
                # TODO delete vertices that are no longer referenced?
                print("DEGENERATE TRI SKIPPED", file=logfile)
                continue

            # Make CollisionPoly
            type = poly.type
            flagsA = poly.flags_1 << 13
            flagsB = poly.flags_2 << 13

            # indices for min/max x/z
            idxs = [0,1,2]
            minx = min(idxs, key = lambda i : self.vertices[sorted_indices[i]].x)
            minz = min(idxs, key = lambda i : self.vertices[sorted_indices[i]].z)
            maxx = max(idxs, key = lambda i : self.vertices[sorted_indices[i]].x)
            maxz = max(idxs, key = lambda i : self.vertices[sorted_indices[i]].z)
            bb_indices = (minx << 6) | (minz << 4) | (maxx << 2) | (maxz << 0)

            # (signed) origin dist, rounded to nearest integer
            # NB This is accurate for axis-aligned polys as only one normal component is non-zero (and is therefore equal
            #    to one) and the vertex positions have only integer resolution. Problems arise when a poly is not axis-aligned,
            #    in which case the distance will be inaccurate by some amount < 1. This is OK for regular gameplay, let the
            #    glitchers have their fun :-)
            dist = round(-(normal.x * tri_vertices[0].x + normal.y * tri_vertices[0].y + normal.z * tri_vertices[0].z))

            # quantize normal components from [-1.0, 1.0] (float) -> [-32767, 32767] (int)
            normal.x = round(normal.x * 32767)
            normal.y = round(normal.y * 32767)
            normal.z = round(normal.z * 32767)

            # Make bounding box for tree construction
            aabb = AABB.from_tri_vertices(tri_vertices)
            # create line for C output, assign to bounding box for access later
            aabb.ob = "    " + \
                f"{{ 0b{bb_indices:08b}, {type}, " + \
                f"{{ {flagsA} | {sorted_indices[0]:4}, {flagsB} | {sorted_indices[1]:4}, {sorted_indices[2]:4} }}, " + \
                f"{VEC3S_C(normal)}, {dist:6} }},"
            tri_bboxes.add(aabb)

        # Make total bounding box

        full_bbox = AABB.Union(tri_bboxes)

        # Build tree out of the bounding boxes

        t = time.time()
        with multiprocessing.Pool(processes=os.cpu_count()) as pool:
            tree = build_bvh(tri_bboxes, full_bbox, pool)
        print(f"Tree construction took {time.time() - t}s")

        # Make BVHNode C structures

        nodes_ordered = []

        node_queue = [(tree, None)]
        while len(node_queue) != 0:
            node,parent_idx = node_queue.pop(0)

            nodes_ordered.append([node, None])

            if parent_idx is not None:
                nodes_ordered[parent_idx][1] = len(nodes_ordered) - 1

            if isinstance(node, BVHNode):
                # guarantees left and right nodes are adjacent
                node_queue.append((node.left, len(nodes_ordered) - 1))
                node_queue.append((node.right, None))

        node_array = []
        tris_array = []

        num_leaves = 0

        for i,(node,left) in enumerate(nodes_ordered):
            if isinstance(node, BVHLeaf):
                assert left is None
                bounds = AABB.Union(node.aabbs)

                node_array.append(f"    /* LEAF   */ {{ {{ {VEC3S_C(bounds.min)}, {VEC3S_C(bounds.max)} }}, {len(tris_array)}, {len(node.aabbs)} }},")

                tris_array.append(f"    // Leaf {i}")
                num_leaves += 1

                # We order these from most likely to least likely to receive an intersection, that is from greatest to
                # least surface area
                sorted_aabbs = list(sorted(node.aabbs, key = lambda aabb : aabb.surface_area, reverse=True))
                for aabb in sorted_aabbs:
                    assert aabb.ob is not None
                    tris_array.append(aabb.ob)
            else:
                assert left is not None
                node : BVHNode
                node_array.append(f"    /* BRANCH */ {{ {{ {VEC3S_C(node.aabb.min)}, {VEC3S_C(node.aabb.max)} }}, {left}, 0 }},")

        num_polys = len(tri_bboxes)
        assert len(tris_array) == num_polys + num_leaves

        node_array_c = f"{static}BVHNode {bvh_tree_name}[{len(node_array)}] = {{\n"
        for node_str in node_array:
            node_array_c += node_str + "\n"
        node_array_c += "};\n"

        # Make poly array now that tris are in the correct order

        colpoly_array_c = f"{static}CollisionPoly {colpoly_array_name}[{num_polys}] = {{\n"
        for line in tris_array:
            colpoly_array_c += line + "\n"
        colpoly_array_c += "};\n"

        output += colpoly_array_c + "\n"
        output += node_array_c + "\n"

        # LOGFILE = open(LOGPATH, "w")
        # tree.print(ofile=LOGFILE)
        # LOGFILE.close()

        # Make Surface Types

        surface_types_c = f"{static}SurfaceType {surface_types_array_name}[{len(self.surface_types)}] = {{\n"
        for w0,w1 in self.surface_types:
            surface_types_c += f"    {{ 0x{w0:08X}, 0x{w1:08X} }},\n"
        surface_types_c += "};\n"

        output += surface_types_c + "\n"

        # Make Cam Data

        output += static + self.write_cam_data(cam_data_array_name) + "\n"

        # Make Waterboxes

        if self.waterboxes is not None:
            output += static + self.write_water_boxes(waterboxes_array_name) + "\n"

        # Make CollisionHeader

        collision_header_c = f"{static}CollisionHeader {collision_header_name} = {{\n"
        collision_header_c += f"    {VEC3S_C(full_bbox.min)},\n"
        collision_header_c += f"    {VEC3S_C(full_bbox.max)},\n"
        collision_header_c += f"    {num_vertices},\n"
        collision_header_c += f"    {vertex_array_name},\n"
        collision_header_c += f"    {num_polys},\n"
        collision_header_c += f"    {colpoly_array_name},\n"
        collision_header_c += f"    {bvh_tree_name},\n"
        collision_header_c += f"    {surface_types_array_name},\n"
        collision_header_c += f"    {cam_data_array_name},\n"
        if self.waterboxes is not None:
            collision_header_c += f"    {len(self.waterboxes)},\n"
            collision_header_c += f"    {waterboxes_array_name},\n"
        else:
            collision_header_c += f"    {0},\n"
            collision_header_c += f"    NULL,\n"
        collision_header_c += "};\n"

        output += collision_header_c + "\n"

        memsize = 0x10 * len(node_array) + 0x10 * num_polys + 6 * num_vertices + 8 * len(self.surface_types)
        print(f"memory size = 0x{memsize:X}", file=logfile)

        return output

    def write_cam_data(self, name):
        nl = "\n"
        return f"BgCamInfo {name}[] = {{{nl}{nl.join('    ' + str(x) + ',' for x in self.cam_data)}\n}};\n"

    def write_water_boxes(self, name):
        nl = "\n"
        return f"WaterBox {name}[] = {{{nl}{nl.join('    ' + str(x) + ',' for x in self.waterboxes)}\n}};\n"

def crawl_files(base_path):
    for root,dirs,files in os.walk(base_path):
        for f in files:
            if not f.endswith(".c"):
                continue
            path = os.path.join(root, f)

            with open(path, "r") as infile:
                contents = infile.read()

            if "CollisionHeader" not in contents:
                continue

            print("")
            print(path)

            decls = collect_all_decls_of_type(contents, "CollisionHeader")

            all_names = []
            collision : List[Collision] = []

            for header in decls:
                name, minx, miny, minz, maxx, maxy, maxz, _, vertices, _, polygons, surfacetypes, camdata, _, waterboxes = \
                    lex(decls[header], (
                    "CollisionHeader", None, "=", "{",
                        "{", None, None, None, "},",
                        "{", None, None, None, "},",
                        None, None,
                        None, None,
                        None,
                        None,
                        None, None,
                    "};"
                ))

                def toint(x):
                    return int(x.replace(",",""))

                min_coord = Vec3s(toint(minx), toint(miny), toint(minz))
                max_coord = Vec3s(toint(maxx), toint(maxy), toint(maxz))

                vertices = vertices.replace(",","")
                polygons = polygons.replace(",","")
                surfacetypes = surfacetypes.replace(",","")
                camdata = camdata.replace(",","")
                waterboxes = waterboxes.replace(",","")

                vertices_data = collect_decl_forname(contents, vertices, "Vec3s")
                polygons_data = collect_decl_forname(contents, polygons, "CollisionPoly")
                surfacetypes_data = collect_decl_forname(contents, surfacetypes, "SurfaceType")
                camdata_data = collect_decl_forname(contents, camdata, "BgCamInfo")
                waterboxes_data = collect_decl_forname(contents, waterboxes, "WaterBox")

                vertices_data = [Vec3s(*[toint(v) for v in v.split()[1:-1]]) for v in vertices_data[1:-1]]
                polygons_data = [CollisionPoly(*(p.strip()[1:-2].split(", "))) for p in polygons_data[1:-1]]
                surfacetypes_data = "\n".join(s.replace(",   ", "\n    ").replace(",","") for s in surfacetypes_data[1:-1])
                surfacetypes_data = [SurfaceType(*(s.strip()[1:-1].split(" "))) for s in surfacetypes_data.split("\n")]
                camdata_data = [BgCamInfo(*(cd.strip()[1:-2].strip().split(", "))) for cd in camdata_data[1:-1]]
                if waterboxes_data is not None:
                    waterboxes_data = [WaterBox(*(wb.strip()[1:-2].strip().split(", "))) for wb in waterboxes_data[1:-1]]

                all_names.extend((name, vertices, polygons, surfacetypes, camdata, waterboxes))
                col = Collision((name, vertices, polygons, surfacetypes, camdata, waterboxes), min_coord, max_coord,
                                vertices_data, polygons_data, surfacetypes_data, camdata_data, waterboxes_data)
                collision.append(col)

            contents_without_collision = ""

            delayed_inc = False
            inc = True
            for line in contents.split("\n"):
                for name in all_names:
                    if name in line and "SCENE_CMD_COL_HEADER" not in line:
                        inc = False

                if inc:
                    contents_without_collision += line + "\n"

                if delayed_inc:
                    inc = True
                    delayed_inc = False
                if line.startswith("};"):
                    delayed_inc = True

            contents_new_collision = contents_without_collision.strip() + "\n"

            # TODO add new collision structures
            for col in collision:
                contents_new_collision += "\n" + col.convert_collision("/overlays/" in path, sys.stdout)

            contents_new_collision = contents_new_collision.strip() + "\n"

            with open(path, "w") as outfile:
                outfile.write(contents_new_collision)

if __name__ == "__main__":
    crawl_files("extracted/gc-eu-mq-dbg/assets/")
