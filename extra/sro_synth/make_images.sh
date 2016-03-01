# Create multi-sphere-1.mha 
plastimatch synth \
    --dim 100 \
    --spacing 1 \
    --origin -49.5 \
    --output multi-sphere-1.mha \
    --pattern sphere \
    --background -1000 \
    --foreground 0 \
    --sphere-center "0 0 0" \
    --sphere-radius 8

plastimatch synth \
    --input multi-sphere-1.mha \
    --output multi-sphere-1.mha \
    --background 0 \
    --foreground -100 \
    --pattern sphere \
    --sphere-center "20 0 0" \
    --sphere-radius 5

plastimatch synth \
    --input multi-sphere-1.mha \
    --output multi-sphere-1.mha \
    --background 0 \
    --foreground -200 \
    --pattern sphere \
    --sphere-center "0 30 0" \
    --sphere-radius 3

# Create multi-sphere-2.mha 
plastimatch synth \
    --dim 100 \
    --spacing 1 \
    --origin -49.5 \
    --output multi-sphere-2.mha \
    --pattern sphere \
    --background -1000 \
    --foreground 0 \
    --sphere-center "-5 -15 -10" \
    --sphere-radius 8

plastimatch synth \
    --input multi-sphere-2.mha \
    --output multi-sphere-2.mha \
    --background 0 \
    --foreground -100 \
    --pattern sphere \
    --sphere-center "12.32 -5 -10" \
    --sphere-radius 5

plastimatch synth \
    --input multi-sphere-2.mha \
    --output multi-sphere-2.mha \
    --background 0 \
    --foreground -200 \
    --pattern sphere \
    --sphere-center "-15.61 3.37 11.21" \
    --sphere-radius 3
