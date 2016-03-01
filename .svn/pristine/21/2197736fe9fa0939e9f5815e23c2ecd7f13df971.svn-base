--  Author: James Shackleford
-- Created: Feb. 29th, 2012
-- Updated: Mar. 23rd, 2012

----------------------------------
--          TUTORIAL            --
-- Working with the XForm class --
-- -------------------------------

-- frist, let's load an image to play with
img = Image.load ("data/img_01.mha")

-- let's load some transforms that register
-- img_01.mha to img_03.mha

-- you can load a vector field deformation transform like this
xf1 = XForm.load ("data/vf.mha")

-- or an ITK or plastimatch native transform like this...
xf2 = XForm.load ("data/xf.txt")

-- and you can warp an image by adding a compatible transform
warp1 = img + xf1
warp2 = img + xf2

-- and, of course, you can save the warp
warp1:save ("out/xform/warp1.mha")
warp2:save ("out/xform/warp2.mha")

-- you cannot add transforms to make composites...yet
-- currently, this will print a warning to stderr and return (nil)
new_xform = xf1 + xf2

-- you can free XForms you don't need from memory
xf1 = nil
xf2 = nil
collectgarbage()

-- NOTE
-- when the script ends, all items will be removed from memory
-- automatically.  img, warp1, and warp2 will automatically
-- be deleted.  it is not necessary to always manually free
-- items, but it is recommened if you don't need them since
-- images and transforms can take up large amounts of memory.

print ("Tutorial: Xform Class -- Completed")
print ("----------------------------------")
