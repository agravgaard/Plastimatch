function B = makesphere(Ainfo,com,radius)
%% Usage: B = makesphere(Ainfo,com,radius)

xa = Ainfo.Offset(1):Ainfo.ElementSpacing(1):...
     Ainfo.Offset(1)+(Ainfo.Dimensions(1)-1)*Ainfo.ElementSpacing(1);
ya = Ainfo.Offset(2):Ainfo.ElementSpacing(2):...
     Ainfo.Offset(2)+(Ainfo.Dimensions(2)-1)*Ainfo.ElementSpacing(2);
za = Ainfo.Offset(3):Ainfo.ElementSpacing(3):...
     Ainfo.Offset(3)+(Ainfo.Dimensions(3)-1)*Ainfo.ElementSpacing(3);
xa = xa - com(1);
ya = ya - com(2);
za = za - com(3);
[x,y,z] = meshgrid(xa,ya,za);

D = x.*x + y.*y + z.*z;

B = D < radius * radius;
