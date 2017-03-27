# Make fixed image (two spheres)
plastimatch synth \
    --pattern rect \
    --background -1000 \
    --foreground 0 \
    --volume-size "200 200 200" \
    --dim "200 200 200" \
    --rect-size "-80 80 -80 80 -80 80" \
    --output fixed.nrrd
plastimatch synth \
    --input fixed.nrrd \
    --foreground 20 \
    --pattern sphere \
    --sphere-center "-40 -40 0" \
    --sphere-radius 15 \
    --output fixed.nrrd
plastimatch synth \
    --input fixed.nrrd \
    --foreground 15 \
    --pattern sphere \
    --sphere-center "45 45 0" \
    --sphere-radius 10 \
    --output fixed.nrrd

# Make structure image for fixed target sphere
plastimatch synth \
    --background 0 \
    --foreground 1 \
    --volume-size "200 200 200" \
    --dim "200 200 200" \
    --pattern sphere \
    --sphere-center "45 45 0" \
    --sphere-radius 10 \
    --output fixed-struct.nrrd

# Make moving image (one sphere)
plastimatch synth \
    --pattern rect \
    --background -1000 \
    --foreground 0 \
    --volume-size "200 200 200" \
    --dim "200 200 200" \
    --rect-size "-80 80 -80 80 -80 80" \
    --output moving.nrrd
plastimatch synth \
    --input moving.nrrd \
    --foreground 20 \
    --pattern sphere \
    --sphere-center "0 0 0" \
    --sphere-radius 20 \
    --output moving.nrrd

# Make structure image for moving target sphere
plastimatch synth \
    --background 0 \
    --foreground 1 \
    --volume-size "200 200 200" \
    --dim "200 200 200" \
    --pattern sphere \
    --sphere-center "0 0 0" \
    --sphere-radius 20 \
    --output moving-struct.nrrd

# Create distance maps
plastimatch dmap \
    --algorithm danielsson \
    --maximum-distance 75 \
    --input fixed-struct.nrrd \
    --output fixed-struct-dmap.nrrd
plastimatch dmap \
    --algorithm danielsson \
    --maximum-distance 75 \
    --input moving-struct.nrrd \
    --output moving-struct-dmap.nrrd
