--  Author: James Shackleford
-- Created: Feb. 29th, 2012
-- Updated: Mar. 23rd, 2012

-------------------------------------
--            TUTORIAL             --
-- Working with the Register class --
-- ----------------------------------

-- if we want to register two files, we must first load them
-- here, we will be registering the "moving image" (img_01.mha)
-- to the "fixed image" (img_03.mha)
fimg = Image.load ("data/img_03.mha")
mimg = Image.load ("data/img_01.mha")

-- we can setup a registration by loading registration stage
-- parameters from a standard plastimatch command file
r = Register.load ("data/stages.txt")

-- we must attach the fixed and moving images to the registration
r.fixed = fimg
r.moving = mimg

-- perform the registration. the result is a transform
xf = r:go()

-- of course, we can use this transform to warp the moving image
warp = mimg + xf

-- which we can save
warp:save ("out/register/my_warp.mha")
-- now, let's get ready for something more complex
-- note: we are keeping the fixed image
mimg = nil
warp = nil
xf = nil
collectgarbage()


-- let's do a 4D registration

-- first, build an associative array of inputs and outputs
phases = {
    { moving = "data/img_01.mha", warp = "out/register/warp_01.mha" },
    { moving = "data/img_02.mha", warp = "out/register/warp_02.mha" },
    { moving = "data/img_04.mha", warp = "out/register/warp_04.mha" },
    { moving = "data/img_05.mha", warp = "out/register/warp_05.mha" }
}

-- let's loop through every input/output pair in our phases array
--    p is the current phase
for _,p in pairs(phases) do
    -- load moving image
    mimg = Image.load (p.moving);
    -- assign it to the registration
    r.moving = mimg;
    -- perform the registration and save the transform
    xf = r:go()
    -- warp the moving image with the transform
    warp = mimg + xf
    -- write warped image to disk
    warp:save (p.warp);
end

-- ADVANCED NOTE: we do not need to perform garbage collection within the
-- above loop for mimg.  Lua will automatically remove the old mimgs from
-- memory... eventually, via automatic garbage collection.  However,
-- explitictly setting mimg=nil and calling collectgarbage() at the end
-- of each loop iteration will give you tighter memory management.


print ("Tutorial: Register Class -- Completed")
print ("-------------------------------------")
