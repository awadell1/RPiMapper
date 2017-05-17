function plotOccupancyGrid3(robot, dataStore, filename)
% Plot Occupancy Grid at 3 timesteps

% Count number of maps stored in dataStore
nMaps = length(dataStore.MapOcc);

mI = [1, floor(nMaps/2), nMaps];
for i = 1:3
   figure
   hold on
   plot(dataStore.MapOcc{mI(i)})
   plot(robot.map)
   hold off
   
   saveas(gcf, fullfile('img', sprintf('%s_n%d', filename, i)), 'png');
end
end