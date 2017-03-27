aviobj = avifile('foo.avi','FPS',1);
plot(rand(2))
aviobj = addframe(aviobj,gcf)
plot(rand(2))
aviobj = addframe(aviobj,gcf)
plot(rand(2))
aviobj = addframe(aviobj,gcf)
plot(rand(2))
aviobj = addframe(aviobj,gcf)
aviobj = close (aviobj);
