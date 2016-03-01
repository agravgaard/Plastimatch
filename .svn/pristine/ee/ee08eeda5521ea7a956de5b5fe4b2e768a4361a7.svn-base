function generate_vfs(volumeFile,outputFile,transf_type,rigidParam)

if isempty(rigidParam)
    rigidParam=[0 0 0];
end

% if isempty(volumeFile)
%     volume=ones([20 20 50]);
%     volInfo.Offset=[0 0 0];
%     volInfo.ElementSpacing=[1 1 1];
%     writemha ('cubicTrial.mha',volume,volInfo.Offset,volInfo.ElementSpacing,'float');
%     volumeFile='cubicTrial.mha';
% end

% if ~isempty(volumeFile)
    [volume,volInfo]=readmha(volumeFile);
% end

dim=size(volume);
clear volume;

% w(1:dim(1),1:dim(2),1:dim(3),1:3)=0;
switch transf_type
    case 'translation'
        if(rigidParam==[0 0 0])
            warndlg('Attention!You specified a null translation along all the axis!');
        else
            grid=ones(dim);
            x=rigidParam(1);
            y=rigidParam(2);
            z=rigidParam(3);
            w(:,:,:,1)=x*grid;
            w(:,:,:,2)=y*grid;
            w(:,:,:,3)=z*grid;
            writemha (outputFile,w,volInfo.Offset,volInfo.ElementSpacing,'float');
        end
    case 'rotation'
        if(rigidParam==[0 0 0])
            warndlg('Attention!You specified a null rotation along all the axis!');
        else
            angleX=(rigidParam(1)*pi)/180;
            angleY=(rigidParam(2)*pi)/180;
            angleZ=(rigidParam(3)*pi)/180;
            rotX=[1 0 0;0 cos(angleX) -sin(angleX);0 sin(angleX) cos(angleX)];
            rotY=[cos(angleY) 0 -sin(angleY);0 1 0;sin(angleY) 0 cos(angleY)];
            rotZ=[cos(angleZ) -sin(angleZ) 0;sin(angleZ) cos(angleZ) 0;0 0 1];
            rotation=rotZ*rotY*rotX;
            aX=((dim(1)-1)/2)*volInfo.ElementSpacing(1);
            aY=((dim(2)-1)/2)*volInfo.ElementSpacing(2);
            aZ=((dim(3)-1)/2)*volInfo.ElementSpacing(3);
            x=linspace(-aX,aX,dim(1));
            y=linspace(-aY,aY,dim(2));
            z=linspace(-aZ,aZ,dim(3));
            [X,Y,Z]=meshgrid(x,y,z);
%             coord=[X(:) Y(:) Z(:)];
            for k=1:dim(3)
                coord(:,:,1)=X(:,:,k)';
                coord(:,:,2)=Y(:,:,k)';
                coord(:,:,3)=Z(:,:,k)';
                for c=1:dim(2)
                    for r=1:dim(1)
                        point(1,1)=coord(r,c,1);
                        point(2,1)=coord(r,c,2);
                        point(3,1)=coord(r,c,3);
                        w(r,c,k,:)=rotation*point-point;
                    end 
                end
            end
            clear rotation
            writemha (outputFile,w,volInfo.Offset,volInfo.ElementSpacing,'float');
        end
    case 'rototranslation'
        if(rigidParam==[0 0 0 0 0 0])
            warndlg('Attention!You specified a null rotation and translation along all the axis!');
        elseif(length(rigidParam)<6)
            warndlg('Sorry, I am not able to perform a rototranslation with less than 6 parameters');
        else
            angleX=(rigidParam(4)*pi)/180;
            angleY=(rigidParam(5)*pi)/180;
            angleZ=(rigidParam(6)*pi)/180;
            rotX=[1 0 0;0 cos(angleX) -sin(angleX);0 sin(angleX) cos(angleX)];
            rotY=[cos(angleY) 0 -sin(angleY);0 1 0;sin(angleY) 0 cos(angleY)];
            rotZ=[cos(angleZ) -sin(angleZ) 0;sin(angleZ) cos(angleZ) 0;0 0 1];
            rotation=rotZ*rotY*rotX;
            aX=((dim(1)-1)/2)*volInfo.ElementSpacing(1);
            aY=((dim(2)-1)/2)*volInfo.ElementSpacing(2);
            aZ=((dim(3)-1)/2)*volInfo.ElementSpacing(3);
            x=linspace(-aX,aX,dim(1));
            y=linspace(-aY,aY,dim(2));
            z=linspace(-aZ,aZ,dim(3));
            [X,Y,Z]=meshgrid(x,y,z);
%             coord=[X(:) Y(:) Z(:)];
            for k=1:dim(3)
                coord(:,:,1)=X(:,:,k)';
                coord(:,:,2)=Y(:,:,k)';
                coord(:,:,3)=Z(:,:,k)';
                for c=1:dim(2)
                    for r=1:dim(1)
                        point(1,1)=coord(r,c,1);
                        point(2,1)=coord(r,c,2);
                        point(3,1)=coord(r,c,3);
                        w(r,c,k,:)=rotation*point-point;
                    end 
                end
            end
            clear rotation
            clear x
            clear y
            clear z
            clear aX
            clear aY
            clear aZ
%             clear rigidParam
            clear coord
            clear point
            clear X
            clear Y
            clear Z
            grid=ones(dim);
            x=rigidParam(1);
            y=rigidParam(2);
            z=rigidParam(3);
            w(:,:,:,1)=w(:,:,:,1)+x*grid;
            w(:,:,:,2)=w(:,:,:,2)+y*grid;
            w(:,:,:,3)=w(:,:,:,3)+z*grid;
            writemha (outputFile,w,volInfo.Offset,volInfo.ElementSpacing,'float');
        end
    case 'radial'
        [y,x] = meshgrid([1:dim(1)]+1/2-dim(1)/2,[1:dim(2)]+1/2-dim(2)/2);
        d = sin(20*atan2(x,y));
        xd = x / dim(1);
        yd = y / dim(2);
        wx = (10 * d .* xd)';
        for i=1:size(w,3)
          w(:,:,i,1) = wx;
        end
        wy = (10 * d .* yd)';
        for i=1:dim(3)
          w(:,:,i,2) = wy;
        end
        w(:,:,:,3) = zeros(dim);
        writemha (outputFile,w,ainfo.Offset,ainfo.ElementSpacing,'float');

end

% writemha ('deformation.mha',w,volInfo.Offset,volInfo.ElementSpacing,'float');