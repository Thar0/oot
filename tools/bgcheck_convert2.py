#!/usr/bin/env python3
#
#   Convert vanilla bgcheck structures to add a new "bbIndices" field to the CollisionPolys
#   Currently not used since bbIndices are not implemented (yet?)
#

import os

def s16(x):
    return (x & 0x7FFF) - (x & 0x8000)

def crawl_files(base_path):
    for root,dirs,files in os.walk(base_path):
        for f in files:
            if not f.endswith(".c"):
                continue
            path = os.path.join(root, f)

            with open(path, "r") as infile:
                contents = infile.read()

            if "CollisionPoly " not in contents:
                continue

            print("")
            print(path)

            lines = ""

            in_data = False
            vertices = None
            for line in contents.split("\n"):
                if line.strip() == "};":
                    in_data = False

                if in_data:
                    assert line.startswith("    {") and line.endswith("},"), line
                    assert vertices is not None

                    poly_type, vtx0, vtx1, vtx2, nx, ny, nz, dist = \
                        [int(d, 16) for d in line[len("    {"):-len("},")].split(", ")]
                    dist = s16(dist)

                    # indices from original data
                    vtx_inds = [vtx0 & 0x1FFF, vtx1 & 0x1FFF, vtx2 & 0x1FFF]

                    # get vertex data
                    vtx_data = { vi : [int(i) for i in vertices[vi][len("    {"):-len("},")].strip().replace(" ", "").split(",")] for vi in vtx_inds}

                    # sort vertex indices by min y
                    sorted_indices = list(sorted(vtx_inds, key = lambda i : vtx_data[i][1]))

                    # get indices of min x/z
                    idxs = [0,1,2]
                    minx = min(idxs, key = lambda i : vtx_data[sorted_indices[i]][0])
                    minz = min(idxs, key = lambda i : vtx_data[sorted_indices[i]][2])
                    maxx = max(idxs, key = lambda i : vtx_data[sorted_indices[i]][0])
                    maxz = max(idxs, key = lambda i : vtx_data[sorted_indices[i]][2])
                    bb_indices = (minx << 6) | (minz << 4) | (maxx << 2) | (maxz << 0)

                    assert poly_type < 256

                    new_poly_line  = f"    {{ "
                    new_poly_line += f"0b{bb_indices:08b}, "
                    new_poly_line += f"{poly_type:3}, "
                    new_poly_line += f"{{ "
                    new_poly_line += f"0x{vtx0 & ~0x1FFF:04X} | {sorted_indices[0]:4}, "
                    new_poly_line += f"0x{vtx1 & ~0x1FFF:04X} | {sorted_indices[1]:4}, "
                    new_poly_line += f"{sorted_indices[2]:4} }}, "
                    new_poly_line += f"{{ 0x{nx:04X}, 0x{ny:04X}, 0x{nz:04X} }}, {dist} }},"

                    lines += new_poly_line + "\n"
                    continue

                if line.startswith("CollisionPoly "):
                    name = line[len("CollisionPoly "):line.find("[")]
                    assert name.endswith("Polygons")
                    vertices_name = name.replace("Polygons", "Vertices")
                    assert vertices_name in contents

                    in_data = True
                    vertices = []

                    in_data2 = False
                    for line2 in contents.split("\n"):
                        if line2.strip() == "};":
                            in_data2 = False

                        if in_data2:
                            vertices.append(line2)

                        if line2.startswith(f"Vec3s {vertices_name}"):
                            in_data2 = True

                lines += line + "\n"

            lines = lines.strip() + "\n"

            with open(path, "w") as outfile:
                outfile.write(lines)

if __name__ == "__main__":
    crawl_files("extracted/gc-eu-mq-dbg/assets/")
