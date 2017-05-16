function [belief] = sensorBelief(robot, pose, dataStore)
% sensorBelief: Computes the probability of the sensor readings given a pose
%
%	INPUTS
%		robot		A Robot Object
%		robotPose	n-by-3 pose vector in global coordinates (x,y,theta)
%		dataStore	The dataStore
%	OUTPUTS
%		belief		The probability of the sensor readings given the pose

%% Sonar Belief
sonar = Robot.getData(dataStore, 'sonar');
if ~isempty(sonar)
	sonarBelief = robot.sonarBelief(pose, sonar);
else
	sonarBelief = 1;
end

%% Bump Belief
bumpBelief = ~robot.map.collideCircle(pose(:,1), pose(:,2), robot.bump_radius)';

%% Beacon Belief
beacon = Robot.getData(dataStore, 'beacon');
if numel(beacon)
    beaconBelief = robot.beaconBelief(pose, beacon);
else
    beaconBelief = 1;
end

%% Combine Beliefs
belief = sonarBelief .* bumpBelief .* beaconBelief;