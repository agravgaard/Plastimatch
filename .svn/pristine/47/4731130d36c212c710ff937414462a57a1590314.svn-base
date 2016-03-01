function sphere_match (id)

fn = sprintf('data/%04d_rp.mha', id);

[A,Ainfo] = readmha(fn);

xa = Ainfo.Offset(1):Ainfo.ElementSpacing(1):...
     Ainfo.Offset(1)+(Ainfo.Dimensions(1)-1)*Ainfo.ElementSpacing(1);
ya = Ainfo.Offset(2):Ainfo.ElementSpacing(2):...
     Ainfo.Offset(2)+(Ainfo.Dimensions(2)-1)*Ainfo.ElementSpacing(2);
za = Ainfo.Offset(3):Ainfo.ElementSpacing(3):...
     Ainfo.Offset(3)+(Ainfo.Dimensions(3)-1)*Ainfo.ElementSpacing(3);
[x,y,z] = meshgrid(xa,ya,za);

np = sum(A(:));
com = [sum(x(:).*A(:))/np,sum(y(:).*A(:))/np,sum(z(:).*A(:))/np];
radius = 18;

x0 = [com, radius];

%B = makesphere (Ainfo, com, 10);
%D = 2 * sum(B(:).*A(:)) / (sum(B(:)) + sum(A(:)));

x = fminsearch ('sphere_match_obj', x0, [], [], A, Ainfo);
printf ('Restarting...\n');
x0 = x;
x = fminsearch ('sphere_match_obj', x0, [], [], A, Ainfo);
