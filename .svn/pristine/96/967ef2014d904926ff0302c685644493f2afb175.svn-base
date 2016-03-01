--  Author: James Shackleford
-- Created: Feb. 29th, 2012
-- Updated: Mar. 23rd, 2012

----------------------------------
--          TUTORIAL            --
-- Working with the Image class --
-- -------------------------------

-- load an image
my_image = Image.load ("data/img_01.mha")

-- "Save" (write it to disk)
my_image:save ()

-- "Save-As" (write it to disk as a new file)
my_image:save ("out/images/img_01_copy.mha")

-- get image header information
my_image:info ();

-- scale total volume intensity by a constant
w1 = 0.1*my_image
w1:save ("out/images/w1.mha");

-- the order doesn't matter
w2 = my_image*0.9
w2:save ("out/images/w2.mha");

-- multiplying 2 volumes is illegal and will return (nil) & print
-- a warning to stderr.
foo = w1*w2
print ("foo:", foo)

-- you can save memory by freeing volumes that are no longer needed.
-- just set them to (nil) and call collectgarbage()
w2 = nil
collectgarbage()

-- derived images cannot be saved without explicitly specifying a
-- filename.  this will print a warning to stderr.
w3 = 2.5*w1
w3:save ();

-- but once a filename is specifed, you can save without specifying
-- a name.  the last explicitly specified filename sticks to the image
w3:save ("out/images/w3.mha");
w3:save ();

-- you can also automatically generate masks based on a threshold.
-- everything *above* the threshold will be masked
my_mask = my_image:automask (-900);
my_mask:save ("out/images/mask.mha");

print ("Tutorial: Image Class -- Completed")
print ("----------------------------------")
