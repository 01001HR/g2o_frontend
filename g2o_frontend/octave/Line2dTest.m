# generate the lines (here i use the same generation procedure as for the points, since the
# lines in cartesian coords are mapped as points with a normal
# in this case i threat the normal as the direction


#  generate a transform

gtx = [-0.00129444 0.000101912 -0.139905]'; #4 e 5 no
# gtx = [2.46308e-07 3.77269e-06 -0.0912328]'; #3 e 4 si
printf("ground truth vector is:\n");
disp(gtx);
gtX = v2t_2d(gtx);
printf("the ground truth transform is:\n");
disp(gtX);

#np = 5;
#tscale = 100;
#printf("Generating a sample set of %d lines with normal, distributed in a radius of %f meters\n\n", np, tscale);
#Li = rand(3, np)- 0.5;
#Lj = zeros(3,np);
#Li(1:2, :) *= tscale;
#Lj = zeros(3,np);
#for i = [1:size(Li)(2)]
  #Li(1:2, i) /= norm(Li(1:2, i));
#Lj(:,i) = line2d_remapCartesian(gtX, Li(:,i));
#endfor;

# we need a coloum vector
Li = load('l1i_octave.dat');
Li = Li';
disp("Li:");
disp(Li);

Lj = load('l2i_octave.dat');
Lj = Lj';
disp("Lj:");
disp(Lj);

Ljpoint = load('l2i.dat');
disp("Ljpoint");
disp(Ljpoint);

X=eye(3);

Omega=eye(3);
Omega(1:2,1:2)*=1000;
Omega(3,3)*=100;

# choose line2d_solve if you want a non linear solving
# [Xsnl, es] = line2d_solve(Li, Lj, Omega, X, 10);
[Xs, es] = line2d_linearSolve(Li, Lj, Omega, X);
# Xs_final = zeros(3,3);
# Xs_final = Xsnl(:,:,10);
#disp("- final transform ");
# disp(Xs);
disp("Transform error vector:");
disp(t2v_2d(gtX*Xs));
# disp("Transform error:");
# disp(gtX*Xs);


#debug: controllo se la trasf trovata * Lj =  Li (è l'inversa)
TLj = zeros(3, size(Li, 2));
for i = [1:size(Lj)(2)]
  #Li(1:2, i) /= norm(Li(1:2, i));
 TLj(:,i) = line2d_remapCartesian(Xs, Lj(:,i));
endfor;
TLj

# just to visualize the line2 remmaped with a plot
TLjpoint = zeros(size(Ljpoint, 1), 2);
for i = [1:size(Ljpoint)(1)]
 TLjpoint(i,:)= point2d_remapCartesian(Xs, Ljpoint(i,:)');
endfor;
save("l2_transformed.dat", "TLjpoint");
TLjpoint
