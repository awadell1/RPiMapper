function plotBumpTrajectory(robot, dataStore, filename)
figure
hold on
pose = dataStore.truthPose(:,2:4);
plot(pose(:,1), pose(:,2), 'k-')

%Plot Bump Events
hasBump = any(dataStore.bump(:,2:4), 2);
plot(pose(hasBump,1), pose(hasBump,2), 'ro')

%Plot Map
plot(robot.map)

%Save Figure
saveas(gcf, fullfile('img', sprintf('%s_traj', filename)), 'png')