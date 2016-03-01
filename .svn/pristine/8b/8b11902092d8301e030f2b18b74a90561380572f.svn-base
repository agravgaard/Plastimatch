function delete_edges (img_reg, img_ref, img_out, img_edge, background)

% This function deletes the patient outside edges that the registration task generates.
% Author: Paolo Zaffino  (p.zaffino@yahoo.it)
%
% img_reg = image with edges
% img_ref = fixed image in the registration
% img_out = image without edges
% img_edge = only the edges
% background = background value (HU)

[A Ainfo]=readmha(img_reg);
[B Binfo]=readmha(img_ref);

diff=A-B;
edge=zeros(size(A,1),size(A,2), size(A,3));

for i=1:size(A,3)
    for j=1:size(A,2)
        for k=1:size(A,1)
            if (diff(k,j,i)==abs(background))
                A(k,j,i)=background;
                edge(k,j,i)=1;
            end
        end
    end
end

writemha(img_out, A, Ainfo.Offset', Ainfo.ElementSpacing', 'short');
writemha(img_edge, edge, Ainfo.Offset', Ainfo.ElementSpacing', 'uchar');

return