# Make homogenous image
plastimatch synth \
    --pattern rect \
    --background -1000 \
    --foreground 0 \
    --volume-size "200 200 200" \
    --dim "200 200 200" \
    --rect-size "-100 100 -100 100 -100 100" \
    --output homo_water.nrrd

# Make hetero image
plastimatch synth \
    --pattern rect \
    --background -1000 \
    --foreground 0 \
    --volume-size "200 200 200" \
    --dim "200 200 200" \
    --rect-size "-100 -50 -100 100 -100 100" \
    --output hetero_a.nrrd
plastimatch synth \
    --pattern rect \
    --foreground 1000 \
    --volume-size "200 200 200" \
    --dim "200 200 200" \
    --rect-size "-50 0 -100 100 -100 100" \
    --input hetero_a.nrrd \
    --output hetero_a.nrrd
plastimatch synth \
    --pattern rect \
    --foreground 3000 \
    --volume-size "200 200 200" \
    --dim "200 200 200" \
    --rect-size "0 50 -100 100 -100 100" \
    --input hetero_a.nrrd \
    --output hetero_a.nrrd
