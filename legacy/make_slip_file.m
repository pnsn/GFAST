a = load('illapel_optimal_slipmodels.txt');
fid = fopen('illapel_150s.txt','wt');


a1 = find(a(:,2) == 180);
for i = 1:length(a1)
fprintf(fid,'%s %1.4f\n','>psxy -Z',sqrt(a(a1(i),7)^2+a(a1(i),8)^2));
fprintf(fid,'%1.4f %1.4f\n',a(a1(i),13),a(a1(i),12));
fprintf(fid,'%1.4f %1.4f\n',a(a1(i),15),a(a1(i),14));
fprintf(fid,'%1.4f %1.4f\n',a(a1(i),17),a(a1(i),16));
fprintf(fid,'%1.4f %1.4f\n',a(a1(i),19),a(a1(i),18));
fprintf(fid,'%1.4f %1.4f\n',a(a1(i),13),a(a1(i),12));
end
fclose(fid)