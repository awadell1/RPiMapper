function plotOccupancyThreshold(robot, dataStore, filename)
   figure
   hold on
   mapOcc = dataStore.MapOcc{end};
   mapOcc.data = mapOcc.data > -6;
   plot(mapOcc)
   plot(robot.map)
   hold off
   colorbar off
  
   
   saveas(gcf, fullfile('img', sprintf('%s_Thes', filename)), 'png');